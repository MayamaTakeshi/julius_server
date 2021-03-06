#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <signal.h>
#include <stdlib.h>

#include <errno.h>
#include<sys/wait.h>

#include <julius/juliuslib.h>

int max_concurrent_sessions = 10;

int ongoing_sessions = 0;

int client = -1;

void check_local_port() {
  struct sockaddr_in sin;
  int addrlen = sizeof(sin);
  if(getsockname(3, (struct sockaddr *)&sin, &addrlen) == 0 &&
    sin.sin_family == AF_INET &&
    addrlen == sizeof(sin))
  {
    int local_port = ntohs(sin.sin_port);
    char buf[24];
    sprintf(buf, "+AUDIO_PORT:%i\n", local_port);    
    printf("local_port=%i\n", local_port);
    write(client, buf, strlen(buf));
  } else {
    perror("getsockname failed:");
    if(errno == EBADF) {
      // OK: this means we are using fullduplex plugin instead of adinnet
    } else {
      // Unrecoverable error
      exit(1); 
    }
  }
}

static void
process_online(Recog *recog, void *dummy)
{
  write(client, "+READY\n", 7); 
}

static void
speech_start(Recog *recog, void *dummy)
{
  printf("speech_start\n");
  write(client, "+SPEECH_START\n", 14); 
}

static void
speech_ready(Recog *recog, void *dummy)
{
  printf("speech_ready\n");
  write(client, "+SPEECH_READY\n", 14); 
}

static void
speech_stop(Recog *recog, void *dummy)
{
  printf("speech_stop\n");
  write(client, "+SPEECH_STOP\n", 13); 
}

static void
recognition_begin(Recog *recog, void *dummy)
{
  printf("recognition_begin\n");
}

static void
recognition_end(Recog *recog, void *dummy)
{
  printf("recognition_end\n");
}

int poll_count = 0;

static void
process_poll(Recog *recog, void *dummy)
{
    poll_count++;
    if(poll_count > 500) {
        write(client, "+POLL\n", 6); 
        poll_count = 0;
    }
}

/* Sub function to output phoneme sequence. */
static void
put_hypo_phoneme(WORD_ID *seq, int n, WORD_INFO *winfo)
{
  int i,j;
  WORD_ID w;
  static char buf[MAX_HMMNAME_LEN];

  if (seq != NULL) {
    for (i=0;i<n;i++) {
      if (i > 0) printf(" |");
      w = seq[i];
      for (j=0;j<winfo->wlen[w];j++) {
    center_name(winfo->wseq[w][j]->name, buf);
    printf(" %s", buf);
      }
    }
  }
  printf("\n"); 
}

/* Callback to output final recognition result.*/
static void
output_result(Recog *recog, void *dummy)
{
  int i, j;
  int len;
  WORD_INFO *winfo;
  WORD_ID *seq;
  int seqnum;
  int n;
  Sentence *s;
  RecogProcess *r;
  HMM_Logical *p;
  SentenceAlign *align;

  /* all recognition results are stored at each recognition process
     instance */
  for(r=recog->process_list;r;r=r->next) {

    /* skip the process if the process is not alive */
    if (! r->live) continue;

    /* result are in r->result.  See recog.h for details */

    /* check result status */
    if (r->result.status < 0) {      /* no results obtained */
      /* outout message according to the status code */
      switch(r->result.status) {
      case J_RESULT_STATUS_REJECT_POWER:
    printf("<input rejected by power>\n");
    break;
      case J_RESULT_STATUS_TERMINATE:
    printf("<input teminated by request>\n");
    break;
      case J_RESULT_STATUS_ONLY_SILENCE:
    printf("<input rejected by decoder (silence input result)>\n");
    break;
      case J_RESULT_STATUS_REJECT_GMM:
    printf("<input rejected by GMM>\n");
    break;
      case J_RESULT_STATUS_REJECT_SHORT:
    printf("<input rejected by short input>\n");
    write(client, "+END\n", 4);
    break;
      case J_RESULT_STATUS_REJECT_LONG:
    printf("<input rejected by long input>\n");
    write(client, "+END\n", 4);
    break;
      case J_RESULT_STATUS_FAIL:
    printf("<search failed>\n");
    write(client, "+END\n", 4);
    break;
      }
      /* continue to next process instance */
      continue;
    }

    /* output results for all the obtained sentences */
    winfo = r->lm->winfo;

    for(n = 0; n < r->result.sentnum; n++) { /* for all sentences */
      s = &(r->result.sent[n]);
      seq = s->word;
      seqnum = s->word_num;

      /* output word sequence like Julius */
      printf("sentence%d:", n+1);

      write(client, "+RESULT:", 8);

      for(i=0;i<seqnum;i++) {
         char *p = winfo->woutput[seq[i]];
         printf(" %s", p);
         write(client, " ", 1);
         write(client, p, strlen(p));
      }
      write(client, "\n", 1);

      printf("\n");
      /* LM entry sequence */
      printf("wseq%d:", n+1);
      for(i=0;i<seqnum;i++) printf(" %s", winfo->wname[seq[i]]);
      printf("\n");
      /* phoneme sequence */
      printf("phseq%d:", n+1);
      put_hypo_phoneme(seq, seqnum, winfo);
      printf("\n");
      /* confidence scores */
      printf("cmscore%d:", n+1);
      for (i=0;i<seqnum; i++) printf(" %5.3f", s->confidence[i]);
      printf("\n");
      /* AM and LM scores */
      printf("score%d: %f", n+1, s->score);
      if (r->lmtype == LM_PROB) { /* if this process uses N-gram */
    printf(" (AM: %f  LM: %f)", s->score_am, s->score_lm);
      }
      printf("\n");
      if (r->lmtype == LM_DFA) { /* if this process uses DFA grammar */
    /* output which grammar the hypothesis belongs to
       when using multiple grammars */
    if (multigram_get_all_num(r->lm) > 1) {
      printf("grammar%d: %d\n", n+1, s->gram_id);
    }
      }
      
      /* output alignment result if exist */
      for (align = s->align; align; align = align->next) {
    printf("=== begin forced alignment ===\n");
    switch(align->unittype) {
    case PER_WORD:
      printf("-- word alignment --\n"); break;
    case PER_PHONEME:
      printf("-- phoneme alignment --\n"); break;
    case PER_STATE:
      printf("-- state alignment --\n"); break;
    }
    printf(" id: from  to    n_score    unit\n");
    printf(" ----------------------------------------\n");
    for(i=0;i<align->num;i++) {
      printf("[%4d %4d]  %f  ", align->begin_frame[i], align->end_frame[i], align->avgscore[i]);
      switch(align->unittype) {
      case PER_WORD:
        printf("%s  [%s]\n", winfo->wname[align->w[i]], winfo->woutput[align->w[i]]);
        break;
      case PER_PHONEME:
        p = align->ph[i];
        if (p->is_pseudo) {
          printf("{%s}\n", p->name);
        } else if (strmatch(p->name, p->body.defined->name)) {
          printf("%s\n", p->name);
        } else {
          printf("%s[%s]\n", p->name, p->body.defined->name);
        }
        break;
      case PER_STATE:
        p = align->ph[i];
        if (p->is_pseudo) {
          printf("{%s}", p->name);
        } else if (strmatch(p->name, p->body.defined->name)) {
          printf("%s", p->name);
        } else {
          printf("%s[%s]", p->name, p->body.defined->name);
        }
        if (r->am->hmminfo->multipath) {
          if (align->is_iwsp[i]) {
        printf(" #%d (sp)\n", align->loc[i]);
          } else {
        printf(" #%d\n", align->loc[i]);
          }
        } else {
          printf(" #%d\n", align->loc[i]);
        }
        break;
      }
    }
    
    printf("re-computed AM score: %f\n", align->allscore);

    printf("=== end forced alignment ===\n");
      }
    }
  }

  /* flush output buffer */
  fflush(stdout);
}

void proc_exit() {
    wait(NULL);
    ongoing_sessions--;
    printf("ongoing_sessions=%i\n", ongoing_sessions);
}

/**
 * Main function
 * 
 */
int
main(int argc, char *argv[])
{
  const char *s = getenv("MAX_CONCURRENT_SESSIONS");
  if(s != NULL) {
    max_concurrent_sessions = atoi(s); 
  }

  printf("max_concurrent_sessions=%i\n", max_concurrent_sessions);

  /**
   * configuration parameter holder
   * 
   */
  Jconf *jconf;

  /**
   * Recognition instance
   * 
   */
  Recog *recog;

  /**
   * speech file name for MFCC file input
   * 
   */
  static char speechfilename[MAXPATHLEN];

  int ret;

  /* by default, all messages will be output to standard out */
  /* to disable output, uncomment below */
  jlog_set_output(NULL);

  /* output log to a file */
  //FILE *fp; fp = fopen("log.txt", "w"); jlog_set_output(fp);

  /* if no argument, output usage and exit */
  if (argc == 1) {
    fprintf(stderr, "Julius rev.%s - based on ", JULIUS_VERSION);
    j_put_version(stderr);
    fprintf(stderr, "Try '-setting' for built-in engine configuration.\n");
    return -1;
  }

  /************/
  /* Start up */
  /************/
  /* 1. load configurations from command arguments */
  jconf = j_config_load_args_new(argc, argv);
  /* else, you can load configurations from a jconf file */
  //jconf = j_config_load_file_new(jconf_filename);
  if (jconf == NULL) {      /* error */
    fprintf(stderr, "Try '-setting' for built-in engine configuration.\n");
    return -1;
  }
 

  /* 2. create recognition instance according to the jconf */
  /* it loads models, setup final parameters, build lexicon
     and set up work area for recognition */
  recog = j_create_instance_from_jconf(jconf);
  if (recog == NULL) {
    fprintf(stderr, "Error in startup\n");
    return -1;
  }
 
  /* kernel should automatically reap the child */
  signal(SIGCHLD, proc_exit);

  int sck, addrlen;
  struct sockaddr_in this_addr, peer_addr;
  pid_t child_pid;
  unsigned short port = 10500;

  addrlen = sizeof( struct sockaddr_in );
  memset( &this_addr, 0, addrlen );
  memset( &peer_addr, 0, addrlen );

  this_addr.sin_family      = AF_INET;
  this_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  this_addr.sin_port        = htons(port);

  printf("listening on port=%i\n", port);

  sck = socket( AF_INET, SOCK_STREAM, IPPROTO_IP);
  if(sck == -1) {
    perror("socket creation failed:");
    exit(1);
  }

  int optval;
  socklen_t optlen = sizeof(optval);

  optval = 1;
  optlen = sizeof(optval);

  if(setsockopt(sck, SOL_SOCKET, SO_REUSEADDR, &optval, optlen) < 0) {
    perror("setsockopt(SO_REUSEADDR)");
    exit(EXIT_FAILURE);
  }

  if(bind( sck, (struct sockaddr *)&this_addr, addrlen ) != 0) {
    perror("socket bind failed:");
    exit(1);
  }

  if(listen(sck, 5) != 0) {
    perror("socket listen failed:");
    exit(1);
  }

  printf("waiting connections\n");
  while( -1 != (client = accept( sck, (struct sockaddr *)&peer_addr, &addrlen ) ) ) {
    printf("accepted connection\n");
    if(ongoing_sessions >= max_concurrent_sessions) {
      printf("ongoing_sessions=%i\n", ongoing_sessions);
      printf("max_concurrent_sessions (%i) reached. Refusing request.\n", max_concurrent_sessions);
      close(client);
      continue; 
    }

    child_pid = fork();
    if( child_pid < 0 ) {
      perror("Error forking");
      exit(1);   /* error */
    }

    if( child_pid == 0 ) {
      // in the child
      close(sck);
      break;
    } else {
      // in the parent
       close(client); 
       ongoing_sessions++;
       printf("ongoing_sessions=%i\n", ongoing_sessions);
    }
  }

  close(0);

  if( dup(client) != 0 ) {
    perror("error duplicating socket for stdin");
    exit(1);
  }

  /*
  int optval;
  socklen_t optlen = sizeof(optval);
  */

  optval = 1;
  optlen = sizeof(optval);

  if(setsockopt(client, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
    perror("setsockopt()");
    close(client);
    exit(EXIT_FAILURE);
  }

  /*********************/
  /* Register callback */
  /*********************/
  /* register result callback functions */
  callback_add(recog, CALLBACK_EVENT_PROCESS_ONLINE, process_online, NULL);
  callback_add(recog, CALLBACK_EVENT_SPEECH_READY, speech_ready, NULL);
  callback_add(recog, CALLBACK_EVENT_SPEECH_START, speech_start, NULL);
  callback_add(recog, CALLBACK_EVENT_SPEECH_STOP, speech_stop, NULL);
  callback_add(recog, CALLBACK_EVENT_RECOGNITION_BEGIN, recognition_begin, NULL);
  callback_add(recog, CALLBACK_EVENT_RECOGNITION_END, recognition_end, NULL);
  callback_add(recog, CALLBACK_RESULT, output_result, NULL);
  callback_add(recog, CALLBACK_POLL, process_poll, NULL);

  /**************************/
  /* Initialize audio input */
  /**************************/
  /* initialize audio input device */
  /* ad-in thread starts at this time for microphone */
  if (j_adin_init(recog) == FALSE) {    /* error */
    return -1;
  }

  check_local_port();

  /* output system information to log */
  j_recog_info(recog);

  /***********************************/
  /* Open input stream and recognize */
  /***********************************/

  if (jconf->input.speech_input == SP_MFCFILE || jconf->input.speech_input == SP_OUTPROBFILE) {
    /* MFCC file input */

    while (get_line_from_stdin(speechfilename, MAXPATHLEN, "enter MFCC filename->") != NULL) {
      if (verbose_flag) printf("\ninput MFCC file: %s\n", speechfilename);
      /* open the input file */
      ret = j_open_stream(recog, speechfilename);
      switch(ret) {
      case 0:           /* succeeded */
    break;
      case -1:              /* error */
    /* go on to the next input */
    continue;
      case -2:          /* end of recognition */
    return 0;
      }
      /* recognition loop */
      ret = j_recognize_stream(recog);
      if (ret == -1) return -1; /* error */
      /* reach here when an input ends */
    }

  } else {
    /* raw speech input (microphone etc.) */

    switch(j_open_stream(recog, NULL)) {
    case 0:         /* succeeded */
      break;
    case -1:            /* error */
      fprintf(stderr, "error in input stream\n");
      return 0;
    case -2:            /* end of recognition process */
      fprintf(stderr, "failed to begin input stream\n");
      return 0;
    }
    
    /**********************/
    /* Recognization Loop */
    /**********************/
    /* enter main loop to recognize the input stream */
    /* finish after whole input has been processed and input reaches end */
    ret = j_recognize_stream(recog);
    if (ret == -1) return -1;   /* error */
    
    /*******/
    /* End */
    /*******/
  }

  /* calling j_close_stream(recog) at any time will terminate
     recognition and exit j_recognize_stream() */
  j_close_stream(recog);

  j_recog_free(recog);

  /* exit program */
  return(0);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "plugin_defs.h"
#include <unistd.h>
#include <errno.h>

#include <sys/soundcard.h>
#include <sys/ioctl.h>

int
get_plugin_info (int opcode, char * buf, int buflen)
{
  printf("get_plugin_info for duplex-socket\n");
  switch (opcode) {
  case 0:
    strncpy (buf, "duplex-socket", buflen);
    break;
  }
  return 0;
}

void
adin_get_optname (char * buf, int buflen)
{
  printf("adin_get_optname for duplex-socket\n");
  strncpy (buf, "duplex-socket", buflen);
}

int
adin_get_configuration (int opcode)
{
  printf("adin_get_configuration for duplex-socket\n");
  switch (opcode) {
  case 0: /* enable real-time processing of 1st pass by default? */
    return 1; /* yes */
  case 1: /* enable frontend voice detection by level / zc by default? */
    return 1; /* yes */
  case 2: /* input module threading is needed or not, if supported? */
    return 1; /* yes, needed */
  }
}

boolean
adin_standby (int sfreq, void * dummy)
{
  printf("adin_standby for duplex-socket\n");
  return TRUE;
}

boolean
adin_open (char * pathname)
{
  printf("adin_open for duplex-socket\n");
  //fcntl(0, F_SETFL, O_NONBLOCK);
  return TRUE;
}

int
adin_read (SP16 * buf, int sampnum)
{
  //printf("adin_read for duplex-socket\n");
  int cnt = read(0, buf, sampnum);
  if (cnt <0) {
    printf("Error: failed to read samples errno=%i\n", errno);
    perror("");
    return(ADIN_ERROR);
  }

  return cnt;

  if(cnt > 0) return cnt;

  for(int i=0; i<4096; i++) {
    buf[i] = 0x00;
  }
  //printf("cnt=%i\n", cnt);
  return 4096;
}

boolean
adin_close ()
{
  printf("adin_close for duplex-socket\n");
  close(0);
  return TRUE;
}

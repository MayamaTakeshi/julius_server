LIBSENT=/usr/local/src/git/julius/libsent
JULIUS_ROOT=/usr/local/src/git/julius

LIBJULIUS=$(JULIUS_ROOT)/libjulius

CC=gcc
CFLAGS=-g -O2 

####
#### When using system-installed libraries
####
# CPPFLAGS=`libjulius-config --cflags` `libsent-config --cflags`
# LDFLAGS=`libjulius-config --libs` `libsent-config --libs`

####
#### When using within-package libraries
####
CPPFLAGS=-I$(LIBJULIUS)/include -I$(LIBSENT)/include  `$(LIBSENT)/libsent-config --cflags` `$(LIBJULIUS)/libjulius-config --cflags`
LDFLAGS= -L$(LIBJULIUS) `$(LIBJULIUS)/libjulius-config --libs` -L$(LIBSENT) `$(LIBSENT)/libsent-config --libs`

############################################################

all: julius_server fullduplex_plugin.jpi

julius_server: main.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o julius_server main.c $(LDFLAGS)

fullduplex_plugin.jpi: fullduplex_plugin.c
	gcc -shared -o fullduplex_plugin.jpi fullduplex_plugin.c -I $(JULIUS_ROOT)/plugin/

clean:
	$(RM) *.o *.bak *~ core TAGS

distclean:
	$(RM) *.o *.bak *~ core TAGS
	$(RM) julius_server




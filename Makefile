LIBSENT=/usr/local/src/git/julius/libsent
LIBJULIUS=/usr/local/src/git/julius/libjulius

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

all: julius_server

julius_server: main.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o julius_server main.c $(LDFLAGS)

myplugin.jpi: myplugin.c
	gcc -shared -o myplugin.jpi myplugin.c -I ../../julius/plugin/

clean:
	$(RM) *.o *.bak *~ core TAGS

distclean:
	$(RM) *.o *.bak *~ core TAGS
	$(RM) julius_server



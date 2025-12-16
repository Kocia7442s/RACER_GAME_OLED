ARCH = arm 
CROSS_COMPILE = arm-none-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
INSTALL = install
CFLAGS =  -g -Wall
LDFLAGS = -g -Wall  

PROGS     = game

ifeq ($(prefix),)
    prefix	 = /home/geii/DE1-Soc/rootfs
endif

INSTDIR   = $(prefix)/home/root/
INSTMODE  = 0775
INSTOWNER = geii
INSTGROUP = geii

# SRCR : list of C source code for compiler
SRCS	= main.c goldelox_function.c game_function.c
# OBJS : list of objet code for linker
OBJS	= main.o goldelox_function.o game_function.o

all:	$(PROGS)

$(PROGS): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(PROGS) *.o *~ core

install: $(PROGS)
	$(INSTALL) $(PROGS) -m $(INSTMODE) -t $(INSTDIR) 
#	$(INSTALL) -m $(INSTMODE) #-o $(INSTOWNER) -g $(INSTGROUP) $(PROGS) $(INSTDIR)

uninstall:
	rm -f $(INSTDIR)$(PROGS)
 
depend:
	makedepend -Y -- $(CFLAGS) -- $(SRCS) 2>/dev/null

# The following is used to automatically generate dependencies.
# DO NOT DELETE
game_function.o: game_function.h
goldelox_function.o: goldelox_function.h
main.o: main.h

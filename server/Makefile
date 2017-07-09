.PHONY: all clean install uninstall run

srcdir = .

CFLAGS = -std=c++14 -Wall -Wpointer-arith -O2 -fno-builtin -g

VPATH = $(srcdir)

SHELL = /bin/sh
RM = /bin/rm -vf
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -m a=rx
INSTALL_DATA = $(INSTALL) -m a=r
MKDIR = mkdir
RMDIR = rmdir
CHMOD = chmod

cgidir = /usr/lib/cgi-bin

objects = user-speed.o
cgi_binaries = distsn-top1000.cgi distsn-top1000-html.cgi
cron_binaries = distsn-user-speed 
binaries = $(cgi_binaries) $(cron_binaries)

.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) -c $(CFLAGS) $<

all: $(binaries)

distsn-user-speed: user-speed.o
	$(CXX) user-speed.o -lcurl $(CFLAGS) -o $@

distsn-top1000.cgi: top1000.o
	$(CXX) top1000.o -lcurl $(CFLAGS) -o $@

distsn-top1000-html.cgi: top1000-html.o
	$(CXX) top1000-html.o -lcurl $(CFLAGS) -o $@

install: $(cgi_binaries)
	$(INSTALL_PROGRAM) $(cgi_binaries) --target-directory=$(cgidir)

uninstall:
	-$(RM) /var/lib/distsn/user-speed/*
	-$(RM) $(addprefix $(cgidir)/,$(cgi_binaries))

intialize:
	$(MKDIR) -p /var/lib/distsn/user-speed
	$(CHMOD) -R a+rwx /var/lib/distsn

refresh:
	-$(RM) /var/lib/distsn/user-speed/*

clean:
	-$(RM) $(objects) $(binaries)

activate-cgi:
	a2enmod cgi
	service apache2 restart



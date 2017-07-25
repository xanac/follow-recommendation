.PHONY: all clean install uninstall initialize refresh activate-cgi

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

objects = \
	user-speed-api.o \
	top1000-html.o \
	user-recommendation-api.o \
	instance-speed-history-api.o \
	user-speed.o \
	instance-speed-cron.o \
	instance-first-toot-cron.o
cgi_binaries = \
	distsn-user-speed-api.cgi \
	distsn-top1000-html.cgi \
	distsn-user-recommendation-api.cgi \
	distsn-instance-speed-history-api.cgi
cron_binaries = \
	distsn-user-speed \
	distsn-instance-speed-cron \
	distsn-instance-first-toot-cron
binaries = $(cgi_binaries) $(cron_binaries)
cgi_scripts = \
	distsn-instance-speed-api.cgi \
	distsn-instance-speed-history-files-api.cgi \
	distsn-instance-first-toot-api.cgi
	
cgis = $(cgi_binaries) $(cgi_scripts)

.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) -c $(CFLAGS) $<

all: $(binaries)

distsn-user-speed: user-speed.o
	$(CXX) user-speed.o -lcurl $(CFLAGS) -o $@

distsn-user-speed-api.cgi: user-speed-api.o
	$(CXX) user-speed-api.o -lcurl $(CFLAGS) -o $@

distsn-top1000-html.cgi: top1000-html.o
	$(CXX) top1000-html.o -lcurl $(CFLAGS) -o $@

distsn-user-recommendation-api.cgi: user-recommendation-api.o
	$(CXX) user-recommendation-api.o -lcurl $(CFLAGS) -o $@

distsn-instance-speed-cron: instance-speed-cron.o
	$(CXX) instance-speed-cron.o -lcurl $(CFLAGS) -o $@

distsn-instance-speed-history-api.cgi: instance-speed-history-api.o
	$(CXX) instance-speed-history-api.o $(CFLAGS) -o $@

distsn-instance-first-toot-cron: instance-first-toot-cron.o
	$(CXX) instance-first-toot-cron.o -lcurl $(CFLAGS) -o $@

install: $(cgis)
	$(INSTALL_PROGRAM) $(cgis) --target-directory=$(cgidir)

uninstall:
	-$(RM) $(addprefix $(cgidir)/,$(cgis))

initialize:
	$(MKDIR) -p /var/lib/distsn/user-speed
	$(MKDIR) -p /var/lib/distsn/instance-speed
	$(MKDIR) -p /var/lib/distsn/instance-speed-cron
	$(MKDIR) -p /var/lib/distsn/instance-first-toot
	$(CHMOD) -R a+rwx /var/lib/distsn

refresh:
	-$(RM) /var/lib/distsn/user-speed/*
	-$(RM) /var/lib/distsn/instance-speed/*
	-$(RM) /var/lib/distsn/instance-speed-cron/*

clean:
	-$(RM) $(objects) $(binaries)

activate-cgi:
	a2enmod cgi
	service apache2 restart



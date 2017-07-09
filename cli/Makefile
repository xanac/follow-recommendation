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

objects = user-speed.o
binaries = distsn-user-speed

.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) -c $(CFLAGS) $<

all: $(binaries)

distsn-user-speed: user-speed.o
	$(CXX) user-speed.o -ltinyxml2 -lcurl $(CFLAGS) -o $@

install: $(scripts)
	$(MKDIR) -p /var/lib/distsn/user-speed
	$(CHMOD) -R a+rwx /var/lib/distsn
	$(INSTALL_PROGRAM) $(binaries) --target-directory=$(cgidir)

uninstall:
	-$(RM) /var/lib/distsn/user-speed/*
	-$(RM) $(addprefix $(cgidir)/,$(binaries))

refresh:
	-$(RM) /var/lib/distsn/user-speed/*

clean:
	-$(RM) $(objects) $(binaries)

run: $(binaries)
	./distsn-user-speed hosts.txt



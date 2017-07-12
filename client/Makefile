.PHONY: install uninstall

srcdir = .

htmldir = /var/www/html

VPATH = $(srcdir)

SHELL = /bin/sh
RM = /bin/rm -vf
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -m a=rx
INSTALL_DATA = $(INSTALL) -m a=r
MKDIR = mkdir -p
RMDIR = rmdir

www_files = index.html follow-recommendation.js

install: $(www_files)
	$(INSTALL_DATA) $(www_files) --target-directory=$(htmldir)

uninstall:
	-$(RM) $(addprefix $(htmldir)/,$(www_files))



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

www_files = \
	index.html follow-recommendation.js \
	user-speed.html user-speed.js \
	instance-speed.html instance-speed.js \
	instance-speed-history.html instance-speed-history.js \
	instance-preview.html instance-preview.js \
	instance-new.html instance-new.js \
	fake-mastodon.css \
	kasaikuminkan.jpg google214b9ca3f27d115d.html

install: $(www_files)
	$(INSTALL_DATA) $(www_files) --target-directory=$(htmldir)

uninstall:
	-$(RM) $(addprefix $(htmldir)/,$(www_files))



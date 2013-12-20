# $Id: Makefile,v 1.51 2005/08/16 20:57:45 baum Exp $

export VERSION := 0.9.91
export SUFFIX := 

# It is tried to guess the TCL version for stubs support. If the guess does
# not work, please use for example for Tcl 8.3: 'make TCL_VERSION=8.3'
#
# for installation of the resulting libs in a special directory 
# use 'make DESTDIR=/dir/to/use'

# comment the next lines if you don't want canvas, gconf and/or gnome support 
# or just do a "make gnocl" and then 
# "make gconf", "make gnome", make "vfs" or "make canvas" just as required
USE_CANVAS := 1
USE_GCONF := 1
USE_VFS := 1
USE_GNOME := 0

# for releases use -Os, this makes the lib 15% smaller 
# amd64 need -fPIC, x86 works with and without
CFLAGS += -Wall -pedantic -fPIC
GTK_24 := $(shell pkg-config --atleast-version=2.4 'gtk+-2.0' && echo yes)
GTK_26 := $(shell pkg-config --atleast-version=2.6 'gtk+-2.0' && echo yes)

TCL_VERSION := $(shell echo 'puts $$tcl_version' | tclsh)

# this breaks build on GTK 2.4 
# -DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED 
export ADDCFLAGS := -DVERSION=\"$(VERSION)\" -DUSE_TCL_STUBS $(shell pkg-config --cflags gtk+-2.0) -I/usr/include/tcl$(TCL_VERSION)/
export LIBS := $(shell pkg-config --libs gtk+-2.0) -ltclstub$(TCL_VERSION)

############################################################
# There should be no need to edit something below this line 
############################################################

TGZDIR:=gnocl-$(VERSION)$(SUFFIX)
TGZFILE:=gnocl-$(VERSION)$(SUFFIX).tar.bz2

GTK_OBJ := \
   box.o \
   button.o \
   callback.o \
   checkButton.o \
   colorSelection.o \
   commands.o \
   debug.o \
   dialog.o \
   fontSelection.o \
   entry.o \
   eventBox.o \
   gnocl.o \
   parseOptions.o \
   helperFuncs.o \
   image.o \
   label.o \
   menu.o \
   menuBar.o \
   menuCheckItem.o \
   menuRadioItem.o \
   menuSeparator.o \
   menuItem.o \
   notebook.o \
   paned.o \
   plug.o \
   progressBar.o \
   radioButton.o \
   scale.o \
   scrolledWindow.o \
   separator.o \
   socket.o \
   spinButton.o \
   statusBar.o \
   table.o \
   text.o \
   treeList.o \
   toolBar.o \
   window.o \
   deprecated/combo.o \
   deprecated/fileSelection.o \
   deprecated/optionMenu.o 

ifeq ($(GTK_24),yes)
   GTK_OBJ += action.o \
            colorButton.o \
            comboBox.o \
            expander.o \
            fileChooser.o \
            fontButton.o
endif

ifeq ($(GTK_26),yes)
   GTK_OBJ += aboutDialog.o 
endif

SUBDIRS := 
ifdef USE_CANVAS
   SUBDIRS += canvas
endif
ifdef USE_GCONF
   SUBDIRS += gconf
endif
ifdef USE_VFS
   SUBDIRS += vfs
endif
ifdef USE_GNOME
   SUBDIRS += gnome
endif


DOCFILES := manpages.txt gnocl.xml concepts.xml nonwidgetcmds.xml \
           appletfactory.xml file.xml mime.xml sed_cmds txt2xml.tcl 
EXCLFILES := -name "CVS" -o -name ".??*" -o -name "qqq*" 

.PHONY: all gnocl canvas gconf vfs gnome clean dist install installAndTest uninstall

%.o : %.c; $(CC) -c $(CPPFLAGS) $(CFLAGS) $(ADDCFLAGS) -o $*.o $<

all: pkgIndex.tcl gnocl.so $(SUBDIRS) ; 

gnocl: gnocl.so ;

gnocl.so: $(GTK_OBJ)
	$(CC) -shared -o $@ $(GTK_OBJ) $(LIBS)

gnome canvas gconf vfs: 
	$(MAKE) -C $@

gnoclCanvas.so: 
	$(MAKE) -C canvas

gnoclGconf.so: 
	$(MAKE) -C gconf

gnoclVFS.so: 
	$(MAKE) -C vfs

gnoclGnome.so: 
	$(MAKE) -C gnome

clean:
	rm -f $(GTK_OBJ) gnocl.so 
	$(MAKE) -C canvas clean
	$(MAKE) -C gconf clean
	$(MAKE) -C gnome clean
	$(MAKE) -C vfs clean

install:
	./install.tcl $(VERSION) install

installAndTest:
	./install.tcl $(VERSION) install
	./install.tcl $(VERSION) test

uninstall:
	./install.tcl $(VERSION) uninstall

pkgIndex.tcl: Makefile
	echo -e ',s/ [0-9]*\.[0-9]*\.[0-9]* / $(VERSION) /g\nwq' | ed $@

smalldist: EXCLFILES += -o -path "*/doc/pics/*.png" -o -path "*/doc/*.gif"

smalldist: dist
	

dist:
	(cd ..; \
	mkdir $(TGZDIR); mkdir $(TGZDIR)/doc ; mkdir $(TGZDIR)/doc/xml ; \
        for el in src src/canvas src/gconf src/gnome src/vfs src/deprecated; do \
           mkdir $(TGZDIR)/$$el; \
           cp -r $$el/{Makefile,*.[ch]} $(TGZDIR)/$$el; \
        done; \
        cp src/pkgIndex.tcl src/install.tcl $(TGZDIR)/src; \
	cp -r demos NEWS README license.terms gpg_key.txt $(TGZDIR); \
        (cd doc/xml; cp $(DOCFILES) ../../$(TGZDIR)/doc/xml); \
        cp -r doc/pics $(TGZDIR)/doc ; \
        cp -r doc/gnocl $(TGZDIR)/doc ; \
        find $(TGZDIR) \( $(EXCLFILES) \) -exec rm -r {} \; ; \
	tar -cjf $(TGZFILE) $(TGZDIR);  \
	rm -rf $(TGZDIR) )

doc:
	(cd ../doc/xml ; \
        rm -r ../gnocl ; \
        cat manpages.txt | sed -f sed_cmds | ./txt2xml.tcl; \
        docbook2html -o gnocl gnocl.xml; \
        mv gnocl ..)

test-bz:
	tar -xvjf $(TGZFILE)
	cd $(TGZDIR) && make && .demos/test.tcl
	rm -rf $(TGZDIR)


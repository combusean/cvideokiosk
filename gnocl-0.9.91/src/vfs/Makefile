# $Id: Makefile,v 1.2 2004/08/04 18:57:05 baum Exp $
#
# this Makefile must be called from the Master Gnocl Makefile

ADDCFLAGS += $(shell pkg-config --cflags gnome-vfs-2.0)
LIBS += $(shell pkg-config --libs gnome-vfs-2.0) 

############################################################
# There should be no need to edit something below this line 
############################################################

OBJS = \
   file.o \
   mime.o \
   gnoclVFS.o 


.PHONY: gconf clean 

%.o : %.c; $(CC) -c $(CPPFLAGS) $(CFLAGS) $(ADDCFLAGS) -o $*.o $<

vfs: ../gnoclVFS.so ;

../gnoclVFS.so: $(OBJS)
	$(CC) -shared -o ../gnoclVFS.so $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) ../gnoclVFS.so 


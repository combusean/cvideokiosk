# $Id: Makefile,v 1.4 2004/03/07 09:27:18 baum Exp $
#
# this Makefile must be called from the Master Gnocl Makefile

ADDCFLAGS += $(shell pkg-config --cflags libgnomeui-2.0 libpanelapplet-2.0)
LIBS += $(shell pkg-config --libs libgnomeui-2.0 libpanelapplet-2.0) 

############################################################
# There should be no need to edit something below this line 
############################################################

OBJS = \
   gnoclGnome.o \
   applet.o \
   session.o


.PHONY: gnome clean 

%.o : %.c; $(CC) -c $(CPPFLAGS) $(CFLAGS) $(ADDCFLAGS) -o $*.o $<

gnome: ../gnoclGnome.so ;

../gnoclGnome.so: $(OBJS)
	$(CC) -shared -o ../gnoclGnome.so $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) ../gnoclGnome.so 


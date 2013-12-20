# $Id: Makefile,v 1.3 2004/03/07 09:27:18 baum Exp $
#
# this Makefile must be called from the Master Gnocl Makefile

ADDCFLAGS += $(shell pkg-config --cflags gconf-2.0)
LIBS += $(shell pkg-config --libs gconf-2.0) 

############################################################
# There should be no need to edit something below this line 
############################################################

OBJS = \
   gnoclGconf.o 


.PHONY: gconf clean 

%.o : %.c; $(CC) -c $(CPPFLAGS) $(CFLAGS) $(ADDCFLAGS) -o $*.o $<

gconf: ../gnoclGconf.so ;

../gnoclGconf.so: $(OBJS)
	$(CC) -shared -o ../gnoclGconf.so $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) ../gnoclGconf.so 


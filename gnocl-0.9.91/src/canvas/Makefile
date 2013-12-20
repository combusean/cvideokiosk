# $Id: Makefile,v 1.7 2004/08/15 15:44:57 baum Exp $
#
# this Makefile must be called from the Master Gnocl Makefile

ADDCFLAGS += $(shell pkg-config --cflags libgnomecanvas-2.0)
LIBS += $(shell pkg-config --libs libgnomecanvas-2.0)

############################################################
# There should be no need to edit something below this line 
############################################################

OBJS = \
   canvasBPath.o \
   canvasClipGroup.o \
   canvasImage.o \
   canvasLine.o \
   canvas.o \
   canvasPolygon.o \
   canvasRE.o \
   canvasRichText.o \
   canvasTags.o \
   canvasText.o \
   canvasWidget.o \
   gnoclCanvas.o 


.PHONY: canvas clean 

%.o : %.c; $(CC) -c $(CPPFLAGS) $(CFLAGS) $(ADDCFLAGS) -o $*.o $<

canvas: ../gnoclCanvas.so ;

../gnoclCanvas.so: $(OBJS)
	$(CC) -shared -o ../gnoclCanvas.so $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) ../gnoclCanvas.so 


CFLAGS = -g -I.
COMPILE = g++ $(CFLAGS) -c
LINK = g++ $(CFLAGS)
LIBS = -lGL -lGLU -lglut -lqd -lmpfr

all:	ps3-nishida-1 ps3-nishida-2

ps3-nishida-1	: ps3-nishida-1.o hull3.o acp.o permute.o 
	$(LINK) ps3-nishida-1.o hull3.o acp.o permute.o $(LIBS) -o ps3-nishida-1

ps3-nishida-2: ps3-nishida-2.o hull3.o acp.o permute.o
	$(LINK) ps3-nishida-2.o hull3.o acp.o permute.o $(LIBS) -o ps3-nishida-2
acp.o:	acp.cc acp.h
	$(COMPILE) acp.cc

point.o: point.C point.h object.h pv.h acp.h
	$(COMPILE) point.C

plot.o: plot.C plot.h point.h pv.h acp.h
	$(COMPILE) plot.C

hull.o: hull.C hull.h object.h pv.h acp.h
	$(COMPILE) hull.C

permute.o: permute.C permute.h
	$(COMPILE) permute.C

hull3.o	: hull3.C hull3.h object.h pv.h acp.h permute.h 
	$(COMPILE) hull3.C

ps3-nishida-1.o: ps3-nishida-1.C
	$(COMPILE) ps3-nishida-1.C

ps3-nishida-2.o: ps3-nishida-2.C
	$(COMPILE) ps3-nishida-2.C

clean : 
	rm -f *.o *~ ps3-nishida

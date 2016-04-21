# Makefile for the line by line stats program

BIN 	=  /home/copernicus2/wray/bin
CC	= cc
ID3	=  ../../Include/SYM.h
PB	=  $(BIN)
OBJS	=  main.o
PROGS	=  main.c 
FILES	=  Makefile $(PROGS) 
LIBES	=  -L../../Eglib -L../../Statlib -L../../Lib \
		-leg -lstat -lutil -lm
LOBJS	= ../../Eglib/libeg.a ../../Statlib/libstat.a ../../Lib/libutil.a
CFLAGOPTS = -O2
CFLAGS	= -I../../Include $(CFLAGOPTS)
INCLUDES = BITARRAY.h DEFN.h SYM.h TREE.h mod.h BITSET.h Lib.h TABLE.h \
           class.h sets.h
#-I../../Include -O -s

install:	$(PB)/lstat

$(PB)/lstat:	lstat
	cp lstat $(PB)/lstat
	strip  $(PB)/lstat

lstat:		$(OBJS) $(LOBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBES) -o lstat

$(OBJS):	$(ID3)
	$(CC) $(CFLAGS) $(PROGS) -c

lint:
	lint -I../../Include main.c
		> errs

clean:
	rm -f *.O *.o lstat errs   $(INCLUDES)

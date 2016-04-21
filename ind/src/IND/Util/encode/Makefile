# Makefile for the line by line stats program

#BIN 	=  /home/copernicus2/wray/bin
BIN 	=  ../../../../bin
CC	= cc
ID3	=  ../../Include/SYM.h
PB	=  $(BIN)
OBJS	=  enc.o encsmpl.o
PROGS	=  enc encsmpl
FILES	=  enc.c encsmpl.c
LIBES	=  -L../../Eglib -L../../Statlib -L../../Lib \
		-leg -lstat -lutil -lm
LOBJS	= ../../Eglib/libeg.a ../../Statlib/libstat.a ../../Lib/libutil.a 
CFLAGOPTS = -O2
CFLAGS	= -I../../Include $(CFLAGOPTS)
INCLUDES = BITARRAY.h DEFN.h SYM.h TREE.h mod.h BITSET.h Lib.h TABLE.h \
           class.h sets.h
#-I../../Include -O -s

install:	$(PB)/enc $(PB)/encsmpl

$(PB)/enc:	enc
	cp enc $(PB)/enc
	strip  $(PB)/enc

$(PB)/encsmpl:	encsmpl
	cp encsmpl $(PB)/encsmpl
	strip  $(PB)/encsmpl

$(PROGS):		$$@.o $(OBJS) $(LOBJS)
	$(CC) $(CFLAGS) $@.o $(LIBES) -o $@

$(OBJS):	$(ID3)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.O *.o enc encsmpl errs   $(INCLUDES)

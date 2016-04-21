# Makefile for the Stats utilities lib

SYM	=  ../Include/SYM.h
SET	=  ../Include/sets.h

PROGS = utility.c calc_mse.c most_common.c random.c \
	mmprint.c best_dec.c restrat.c  shuffle.c stats2.c \
	Ztoprob.c std_error.c logbeta.c ex.c \
	beta.c sample.c  \
	cumm_dvec.c normrandom.c logsum.c same_class.c digamma.c
FILES	=  Makefile $(PROGS) 
OBJS 	= utility.o calc_mse.o most_common.o random.o  \
	mmprint.o best_dec.o restrat.o shuffle.o stats2.o \
	Ztoprob.o std_error.o logbeta.o ex.o \
	beta.o sample.o  \
	cumm_dvec.o normrandom.o logsum.o same_class.o digamma.o

INCLUDES = BITARRAY.h DEFN.h SYM.h TREE.h mod.h BITSET.h Lib.h TABLE.h \
           class.h sets.h
CFLAGOPTS =  -O2
CFLAGS	= -I../Include $(CFLAGOPTS) 
CC	= cc

install:	libstat.a

libstat.a:	$(OBJS)
	rm -f libstat.a
	ar qv libstat.a $(OBJS)
	ranlib libstat.a

test:	bt.o
	$(CC) $(CFLAGS) -o bt bt.o -L../Eglib -L../Statlib -lstat -leg -lm

check:	check_rand.o
	$(CC) $(CFLAGS) -o check check_rand.o -L../Eglib -L../Statlib -lstat -leg -lm

sample:	sample.o
	$(CC) $(CFLAGS) -o sample sample.o -L../Lib -L../Statlib -lstat -lutil -lm

.c.o:
	$(CC) -c $(CFLAGS) $< 

lint:
	lint -I../Include \
		 $(PROGS) ../Trees/choose.c ../Eglib/table.c ../Trees/tree.c

clean:
	rm -f $(OBJS) *.O  a.out  $(INCLUDES)

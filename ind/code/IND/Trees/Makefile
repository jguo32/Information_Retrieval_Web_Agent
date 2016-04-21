# Makefile for the ID3 tree system

BIN	= /pkg/IND/bin
CFLAGOPTS =
CC	= gcc

TREE	= ../Include/TREE.h
SETS	= ../Include/sets.h
SYM	= ../Include/SYM.h
PB	= $(BIN)
PROGS   = tgen tclass tprint tprune thead tchar tweight tgamma tquest
FILES	= tgen.c tclass.c tprune.c tprint.c thead.c \
		tchar.c tweight.c tgamma.c tquest.c
OBJS	= tgen.o tclass.o tprune.o tprint.o thead.o \
		tchar.o tweight.o tgamma.o tquest.o
LIBES	= -L../Eglib -L../Treelib -L../Statlib -L../Lib  -L../Graph \
		-ljgph -ltree -ljgph -leg -lstat -leg -lutil -lm 
LOBJS	= ../Eglib/libeg.a ../Treelib/libtree.a \
		../Statlib/libstat.a ../Lib/libutil.a ../Graph/libjgph.a
CFLAGS	= -I../Include $(CFLAGOPTS)
INCLUDES = BITARRAY.h DEFN.h SYM.h TREE.h mod.h BITSET.h Lib.h TABLE.h \
           class.h sets.h

install:	$(PB)/tgen $(PB)/tclass $(PB)/tprint $(PB)/tprune \
		$(PB)/tchar $(PB)/thead  $(PB)/tquest

$(PB)/tgen: tgen
	cp tgen $(PB)/tgen
	strip  $(PB)/tgen

$(PB)/tclass: tclass
	cp tclass $(PB)/tclass
	strip  $(PB)/tclass

$(PB)/tprint: tprint
	cp tprint $(PB)/tprint
	strip  $(PB)/tprint

$(PB)/tquest: tquest
	cp tquest $(PB)/tquest
	strip  $(PB)/tquest

$(PB)/tprune: tprune
	cp tprune $(PB)/tprune
	strip  $(PB)/tprune

$(PB)/tchar: tchar
	cp tchar $(PB)/tchar
	strip  $(PB)/tchar

$(PB)/thead: thead
	cp thead $(PB)/thead
	strip  $(PB)/thead

$(PROGS):	$$@.o  $(LOBJS)
	$(CC) $(CFLAGS) $@.o $(LIBES) -o $@ 

$(OBJS):	$(TREE) $(SETS) $(SYM) 
	$(CC) $(CFLAGS) -c $<

lib:
	cd ../Eglib ; make 
	cd ../Treelib ; make
	cd ../Statlib ; make
	cd ../Lib ; make

lint:
	lint -I../Include $(FILES) 

clean:
	rm -f *.O *.o testout *.out errs errs2 core $(PROGS)  $(INCLUDES)

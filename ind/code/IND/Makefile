# Makefile for the IND source, for installation refer to manual or Release.tex
#
# TO INSTALL:
# On initial installation, order should be:
#   1.  Check for machine compatibility by looking at
#       ``IND/Include/BITSET.h'', see initial comments and BITSETMAX.  The
#       system has only been used extensively on SUNs (though has compiled on
#       HP, AIX, RS600 and others), so expect problems on other UNIX machines.
#       Some things like ``ftime()'' and ``struct timeb'' (in random.c), and a
#       few others are used, which tend to be UNIX version dependent.  Also
#       the ``rand48()'' routines are used throughout for random numbers.
#   2.   Modify your MANPATH environment variable to include
#     	 the "IND/Man" entries, or copy these to the local
#	 man directory.
#   3.   Modify  BIN, CFLAGS and CC below
#   4.   Run "make cflags cc Bin"
#   5.   Run "make install" and check output to see compiles OK
#   6.   Run "make test >& test.out" and diff it with "make.test.out"
#   7.   Check about availability of "gnuplot" version 2.0 or later.
#	 IND can use this to display cut-point profiles.

BIN	= /pkg/IND/bin
CFLAGS	= #  -O2
CC	= gcc
Version = 2.1

#	basic compile
install	:
	cd Treelib; make install
	cd Eglib; make install
	cd Statlib; make install
	cd Lib; make install
	cd Graph; make install
	cd Util/lstat; make install
	cd Util/encode; make install
	cd Trees; make install
	cd Scripts; cp tstat mkclean mktree ttest $(BIN)
	#
	#  NOTICE don't forget to install "man" entries and print manual
	#

compress :
	compress Treelib/* Eglib/* Statlib/* Lib/* Util/lstat/* \
	   Util/encode/* Trees/* Scripts/* Man/man1/* Graph/*

uncompress :
	uncompress Treelib/* Eglib/* Statlib/* Lib/* Util/lstat/* \
	   Util/encode/* Trees/* Scripts/* Man/man1/* Graph/*

#	run a few things to try stuff out
test	:
	cd Data/thyroid ; enc hypo 
	cd Data/thyroid ; encsmpl -vg S,20,13579 hypo ; echo 
	cd Data/thyroid ; encsmpl S,400,13579 hypo ; echo 
	cd Data/thyroid ; \
	mktree -o "-tB2,2 -A1 -Pmml -d6" -p -b -c -slvgt hypo ; echo 
	cd Data/thyroid ; tprint -ipq hypo ; echo
	cd Data/thyroid ; \
	tchar hypo.attr hypo.tree hypo.chtr ; cat hypo.chtr ; echo 
	cd Data/thyroid ; thead -ps hypo.tree ; echo 
	cd Data/thyroid ; \
	  mktree -o "-Pnull -C10 -it -Sfull -d6" -p -n -c -slvGt hypo ; echo 
	cd Data/thyroid ; tprint -p hypo ; echo
	cd Data/thyroid ; mkclean hypo
	cd Data/thyroid ; rm -f hypo.enc* hypo.chtr

#	set CFLAGOPTS globally in Makefiles
cflags:
	for file in  */Makefile */*/Makefile ;\
	do sed -e 's?^\(CFLAGOPTS[ 	]*=[ 	]*\).*$$?\1$(CFLAGS)?' $${file} >temp ;\
	mv temp $${file} ;\
 	done

#	install stripping
strip:
	for file in  */Makefile */*/Makefile ;\
	do sed -e 's?##*strip?strip?' $${file} >temp ;\
	mv temp $${file} ;\
 	done

#	deinstall stripping
nostrip:
	for file in  */Makefile */*/Makefile ;\
	do sed -e 's?strip?#strip?' $${file} >temp ;\
	mv temp $${file} ;\
 	done

#	set CC globally in Makefiles
cc:
	for file in   */Makefile */*/Makefile ;\
	do sed -e 's?^\(CC[ 	]*=[ 	]*\).*$$?\1$(CC)?' \
		 $${file} >temp ;\
	mv temp $${file} ;\
 	done

#	set BIN globally in Makefiles
Bin:
	for file in   */Makefile */*/Makefile ;\
	do sed -e 's?^\(BIN[ 	]*=[ 	]*\).*$$?\1$(BIN)?' \
		 $${file} >temp ;\
	mv temp $${file} ;\
 	done

#	make directories squeeky clean, e.g. before "tar"
squeeky	:
	rm -f */*.dvi */*.log */*.alw */*/*.dvi */*/*.log */*/*.alw gggg
	rm -f */*.ps */*/*.ps gggg
	rm -f *~ */*~ */*/*~ gggg
	rm -f *% */*% */*/*% gggg
	rm -f Man/cat1/* 
	rm -f */*.a */*.o gggg

#	run all cleans recursively
clean	:
	cd Treelib; make clean
	cd Statlib; make clean
	cd Eglib; make clean
	cd Lib; make clean
	cd Graph; make clean
	cd Util/lstat; make clean
	cd Util/encode; make clean
	cd Trees; make clean

#	assemble "tar" for version backup
tar	:
	cd ..;  tar -cf IND.v${Version}.tar  IND 
	compress ../IND.v${Version}.tar

fixmake:
	for file in  */Makefile */*/Makefile ;\
	do sed -e 's? *-lm??' \
	   -e 's?^\(LIBES.*\) *$$?\1 -lm?' $${file} >temp;\
	mv temp $${file} ;\
 	done

edtry:
	for file in  `grep -l SYM.h */*.c */*/*.c` ; \
	do sed \
	   -e '/^#include *.Lib\.h/d' \
	   $${file} >temp;\
	mv temp $${file} ;\
	done

edall:
	for file in  `grep -l SYM.h */*.c */*/*.c` ; \
	do sed \
	   -e 's?MAXFLOAT?FLOATMAX?' \
	   $${file} >temp;\
	mv temp $${file} ;\
 	done

lintall:
	mkdir /tmp/INDdir
	cp Graph/*.c /tmp/INDdir
	cp Graph/*.h /tmp/INDdir
	cp Trees/*.c /tmp/INDdir
	cp Lib/*.c /tmp/INDdir
	cp Statlib/*.c /tmp/INDdir
	cp Treelib/*.c /tmp/INDdir
	cp Eglib/*.c  Eglib/sym.h Eglib/TABLE.h /tmp/INDdir
	cp Include/*.h /tmp/INDdir
	cd /tmp/INDdir ; lint -I/tmp/INDdir *.c -lm > lint.out
	rm -f lint.out
	cp  /tmp/INDdir/lint.out lint.out
	rm -r /tmp/INDdir

# TO RELEASE:
# First check directories for rubbish,
# check Doc to see the .ps.Z files are OK
# check Makefiles for #strip
# set CFLAGFS to -O and make cflags, clean install.
# switch off the various DEBUG flags in *.h, etc. by 'grep DEBUG */*c'
# set ASSERT() in Lib.h to null
# Run "make test" and compare with last run in "make.test.out".
# If all OK then do "make test >& make.test.out" then
# check Version below and do "make release".
#
release :
	#  have you got the copyright and release number in the code
	catman -w -M ../IND/Man
	cd ..;  shar -L 220 -o INDother/IND${Version}.shar -C IND


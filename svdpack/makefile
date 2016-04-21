#---------------------------------------------
# svdpack fortran makefile  (december 11, 1991)
#---------------------------------------------
#
flags=		-o -c
fortran=	f77
#---------------------------------------------
doit:	copyall exec
#---------------------------------------------
copyall:
	cp blas blas.f
	cp las1 las1.f
	cp las2 las2.f
	cp bls1 bls1.f
	cp bls2 bls2.f
	cp sis1 sis1.f
	cp sis2 sis2.f
	cp tms1 tms1.f
	cp tms2 tms2.f
#------------------------------------------------------------
exec: tms2.x tms1.x sis2.x sis1.x bls2.x bls1.x las2.x las1.x
#------------------------------------------------------------
las1.x:	las1.o 
	${fortran}  -o las1.x las1.o 
las1.o:	las1.f 
	${fortran} ${flags} las1.f
#---------------------------------------------
las2.x:	las2.o
	${fortran}  -o las2.x las2.o 
las2.o:	las2.f
	${fortran} ${flags} las2.f 
#---------------------------------------------
bls2.x:	bls2.o  blas.o
	${fortran}  -o bls2.x bls2.o  blas.o
bls2.o:	bls2.f 
	${fortran} ${flags} bls2.f
#---------------------------------------------
bls1.x:	bls1.o  blas.o
	${fortran}  -o bls1.x bls1.o  blas.o
bls1.o:	bls1.f 
	${fortran} ${flags} bls1.f
#---------------------------------------------
sis1.x:	sis1.o  blas.o
	${fortran}  -o sis1.x sis1.o  blas.o
sis1.o:	sis1.f 
	${fortran} ${flags} sis1.f
#---------------------------------------------
sis2.x:	sis2.o  blas.o
	${fortran}  -o sis2.x sis2.o  blas.o
sis2.o:	sis2.f 
	${fortran} ${flags} sis2.f
#---------------------------------------------
tms1.x:	tms1.o  blas.o
	${fortran}  -o tms1.x tms1.o  blas.o
tms1.o:	tms1.f 
	${fortran} ${flags} tms1.f
#---------------------------------------------
tms2.x:	tms2.o  blas.o
	${fortran}  -o tms2.x tms2.o  blas.o
tms2.o:	tms2.f 
	${fortran} ${flags} tms2.f
#---------------------------------------------
blas.o:	blas.f 
	${fortran} ${flags} blas.f
#---------------------------------------------

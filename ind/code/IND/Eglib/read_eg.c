/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"

double   fabs();

#define  MAXLINE_R  2000	/*  max. chars per line read in  */
#define  MAX_ERRS  20

/* #define DEBUG */

bool   enc_multiline = FALSE;   /* set to TRUE if you want multi-line egs */
int    enc_blocksize = 50;      /* egs. read into blocks of this size  */

static int	nerrs=0;	/* number of non-encoded examples */

/*
 *    this macro reads in the next token from a string,
 *      next_vname  =  start of string to scan, reset each time
 *      vname       =  ptr. to token returned, 0 terminated
 *    when the string is exhausted, vname will point to a null string
 *    tokens are white space separated;  skips leading white space
 */
#define  get_vname()   \
  	for (cp=next_vname; *cp && isspace(*cp); cp++) ;   \
	vname = cp;   \
	if ( *cp ) {  \
              for ( cp++ ; *cp && !isspace(*cp); cp++) ;   \
	      if ( *cp ) { next_vname = cp+1;  *cp = 0; } \
	         else next_vname = cp; }

/* encode(egfile, eg)
 *	Given the example line of text, encode it and return in "eg".
 *	If the text could not be encoded, return 0.
 *	skips over lines with errors and reports trouble;
 *	delimiters are as given above
 */
static egtype
encode(egfile, buf, eg)
FILE	*egfile;
char	*buf;  		/*  temp. storage for each line read */
egtype  eg;
{
	char	 *vname;	 /* Current value name		*/
	char	 *next_vname;	 /* next value name		*/
	char     *cp;
	register int	j;
	int      ret;
	float	 f;
	double	atof();
	char	*strtok();

	if (!eg.unordp)  
	    error("encoding into null pointer!","");
restart:
	j = 0;
newline:
	if ( !fgets(buf,MAXLINE_R,egfile) ) {
	   if ( feof(egfile) ) {
		eg.unordp = 0;
		return eg;
	   } else
		uerror("couldn't read example file","");
	}
	next_vname = buf;
	get_vname();
newj:
	if ( !vname || ! *vname  ) {
		/*	at end of line  */
		if ( vname && vname-buf>=MAXLINE_R-1 )
			uerror("example file line over %d chars", MAXLINE_R-1);
		if ( j>nattrs )
			return eg;
		if ( enc_multiline )
			goto newline;
		fprintf(stdrep, "Too few attributes per example\n");
		fprintf(stdrep, "Ignoring this example (%d)\n",negs+nerrs+1);
		nerrs++;
		goto restart;
	}
	/*
	 *	now process the field read
	 */
	if (num_type(j)) {
		/*
		 *   ORDERED
		 */
		if (*vname == DKNsymb && *(vname+1) == 0) {
			ord_val(eg,j) = FDKNOW;
			unkns(j) = TRUE;
	        } else {
		    ret = sscanf(vname,"%f%s",&f,vname);
		    if ( ret!=1 ) {
			fprintf(stdrep, "Value not a float error.\n");
			fprintf(stdrep,"Attribute %s Value %s\n",name(j),vname);
			fprintf(stdrep, "Ignoring this example (%d)\n",
					negs+nerrs+1);
			nerrs++;
			goto restart;
		    }		      
		    if ( sym_fill(j) ) {
		      if ( f < getmin(j) + fabs((double)getmin(j))*0.01 )
			getmin(j) = f - fabs((double)f)*0.01;
		      else if ( f > getmax(j) - fabs((double)getmax(j))*0.01 )
			getmax(j) = f + fabs((double)f)*0.01;
		    } else if (f < getmin(j) - fabs((double)getmin(j))*0.01
			    || f > getmax(j) + fabs((double)getmax(j))*0.01 ) {
			fprintf(stdrep, "Value out of range error.\n");
			fprintf(stdrep,"Attribute %s Value %f\n",name(j),f);
			fprintf(stdrep, "Ignoring this example (%d)\n",
					negs+nerrs+1);
			nerrs++;
			goto restart;
		    }
		    ord_val(eg,j) = f;
		}
	} else if ( set_type(j) ) {
		/*
		 *   SETTYPE
		 */
		if (*vname == DKNsymb &&  *(vname+1) == 0) {
		    /* DONTKNOW condition */ 
		    set_copy(set_val(eg,j) , SDKNOW );
		    unkns(j) = TRUE;
		} else {
		  set_copy(set_val(eg,j),strchkset(vname,j));
		  if ( set_eq( set_val(eg,j),SDKNOW) ) {
                        fprintf(stdrep,"%s unknown value for attribute %s\n",
                                        vname,name(j));
                        fprintf(stdrep,"Ignoring this example (%d)\n",
                                        negs+nerrs+1);
                        nerrs++;
                        goto restart;
		  }
		}
	} else {
		/*
		 *   DISCRETE
		 */
		if (*vname == DKNsymb && *(vname+1) == 0) {
		    /* DONTKNOW condition */ 
		    unord_val(eg,j) = DONTKNOW;
		    unkns(j) = TRUE;
	    	} else if ((unord_val(eg,j) = strchkval(vname, j)) == DONTKNOW) {
		    fprintf(stdrep,"\"%s\" unknown value for attribute %s\n",
					vname,name(j));
		    fprintf(stdrep,"Ignoring this example (%d)\n",
					negs+nerrs+1);
		    nerrs++;
		    goto restart;
	        }
	}
	j++;
	get_vname();
	goto  newj;
}

/*
 *  used to read in example file as a single block;
 *  internally encode an example text file;
 *  reads in and assigns space dynamically
 */

egtype *
read_eg_file(str)
char    *str;
{
  egtype    position_in_egs, *made;
  struct    egblock {
	egtype  egs;
	struct egblock *next;
  } *egs_list, *egs_new;
  int	    inc;
  FILE      *egfile, *efopen();
  char      *buf = mems_alloc(char,MAXLINE_R+1);

  if ( !buf )
	error("no memory to store examples","");
  if ( !str || !*str )
    egfile = stdin;
  else 
    egfile = ezopen(str, "r");

  egs_list = egs_new = mem_alloc(struct egblock);
  egs_new->next = 0;
  if ( ! (position_in_egs.unordp = egs_new->egs.unordp = 
	(unordtype *) salloc (enc_blocksize * egsize) ) )
		error("no memory to store examples","");
  negs=0;
  while ( (position_in_egs = encode(egfile, buf, position_in_egs)).unordp ) {
      if ( nerrs > MAX_ERRS )
	uerror("too may errors in reading data file","");
      if ( !((++negs) % enc_blocksize ) ) {
	egs_new->next = mem_alloc(struct egblock);
	egs_new = egs_new->next;
	if ( !(position_in_egs.unordp = egs_new->egs.unordp = 	
		(unordtype *) salloc (enc_blocksize * egsize))  )
		error("no memory to store examples","");
	egs_new->next = 0;
      } else
        position_in_egs.unordp += egsize;
  }
  if ( ezopened )
	pclose(egfile);
  else
	fclose(egfile);
  sfree(buf);
  if  ( negs % enc_blocksize )
	if ( ! (egs_new->egs.unordp = (unordtype *)
		resalloc(egs_new->egs.unordp,(negs % enc_blocksize)* egsize) ) )
		error("no memory to store examples","");
  if ( ! (made = mems_alloc(egtype,negs) ) )
	error("no memory to store examples","");
  position_in_egs.unordp = egs_list->egs.unordp;
  egs_new = egs_list;
  for (inc=0; inc<negs; inc++ ) {
	made[inc].unordp = position_in_egs.unordp ;
	if ( !((inc+1)%enc_blocksize) ) {
		egs_new = egs_new->next;
		sfree(egs_list);
		egs_list = egs_new;
  		position_in_egs = egs_list->egs;
	} else
		position_in_egs.unordp += egsize;
  }
  if ( egs_list )
	sfree(egs_list);
  return made;
}

/*
 *	used to read in example file line by line;
 *	if called with str==0, 
 *	then  initializes things back to beginning
 *      else  if first call since initialization
 *		 open file with name "str" and place first read eg in encoding
 *      else  read next eg into encoding
 *	return 0 on end of file or error
 *	NB.  updates negs
 */
egtype
read_eg(str)
char    *str;
{
	static 	FILE    *egfile;
	FILE    *efopen();
	static	int	first=1;
	static egtype	eg, egs;
	char    buf[MAXLINE_R+1];
	
	/*
	 *     if no input file, then reset to start over 
	 */
	if ( !first && !str ) {
		first = 1;
		egs.unordp = eg.unordp = 0;
		return eg;
	}
	/*   
	 *     else, if starting, then open the file
         */
	if (first) {
	    first = 0;
	    if ( ! (egs.unordp = eg.unordp = mems_alloc(unordtype,egsize) ) )
		error("no memory to store examples","");
	    nerrs = negs = 0;
	    if ( !*str )
		egfile = stdin;
	    else
		egfile = ezopen(str, "r");
	} else if ( !egs.unordp ) {
		eg.unordp = 0;
		return eg;
	}
	if ( (eg=encode(egfile, buf, eg)).unordp ) {
	    if ( nerrs > MAX_ERRS )
	        uerror("too may errors in reading data file","");
	    negs++;
	} else {
	    if ( ezopened )
		pclose(egfile);
	    else
	        fclose(egfile);
	    sfree(egs.unordp);
	    egs.unordp = eg.unordp = 0;
	} 
	return eg;
}

encodeerrs()
{
  return nerrs;
}

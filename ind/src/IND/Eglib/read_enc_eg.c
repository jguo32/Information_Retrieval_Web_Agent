/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"

static  egtype  encoding;
static 	FILE    *egfile;
static  int last;

/*
 *      used to read in encoded example file record by record;
 *	NB.  looks for compressed file (.Z) if can't find normal file
 */
void
init_read_enc_eg(str,train)
char    *str;
int   train;
{
	FILE    *efopen(), *fopen();
	Header	head;
	char    smplname[150];
	Sampler sampler;
	
	if ( !*str ) 
		error("Encoded examples can't be read from stdin", "");
	encoding.unordp = mems_alloc(unordtype,egsize);
	negs=0;
	strcpy(&smplname[0],str);
	strcat (&smplname[0],"spl");
	if ( egfile = fopen(&smplname[0], "r") ) {
            if (fread((char *)&sampler,sizeof(Sampler),1,egfile) != 1)
                uerror("Could not read from sampler file", "");
	    fclose(egfile);
	} else {
	    /*
	     *       no file found, so use "no sampling"
	     */
	    no_sample(&sampler);
	}
	egfile = ezopen(str, "r");
        if (fread((char *)&head,sizeof(Header),1,egfile) != 1)
                uerror("Could not read header from encoded example file", "");
        if (head.h_magicno != MAGICNO)
                uerror("Incorrect example file type -- bad magic number", "");
        if (head.h_ncattrs != ncattrs || (head.h_egsize != egsize) 
		 || head.h_nsattrs != nsattrs )
                uerror("Example file doesn't match attribute file", "");
	init_sample(head.h_negs,train,&sampler);
	last = -1;
}


/*
 *      used to read in encoded example file record by record;
 *      read next eg into eg
 *	when nothing left to read, return 0 and close file
 *      NB.  updates negs
 *      NB.  does random sampling via next_in_sample()
 */
egtype
read_enc_eg()
{
	int	next;
	
	if ( (next = next_in_sample()) < 0 ) {
	    /*
	     *      finished file,
	     */
	    if ( last>=0 ) {
	      if ( egfile != stdin )
  		if ( ezopened )
        		pclose(egfile);
  		else
        		fclose(egfile);
	      last = next;
	      sfree(encoding.unordp);
	    }
	    encoding.unordp = 0;
            return encoding;            
	}
	if ( next > last ) {
	    /*
	     *      fetch a new record
	     */
	    if ( next > last+1 ) {
	      /*
	       *      skip forward to new record, 
	       *      via pipe or random access file
	       */
	      if ( ezopened ) 
		for (last++; last < next; last++) {
		  if ((fread(encoding.unordp, egsize, 1, egfile)) < 1 )
       		    uerror("Could not read from encoded example file","");
		}
	      else
		fseek(egfile, (long)(next-last-1)*egsize, 1);
	    }
  	    if ((fread(encoding.unordp, egsize, 1, egfile)) < 1 )
       		uerror("Could not read from encoded example file","");
	} else 
	    /*
	     *    do nothing, correct record already there
	     */
	    ;
	/*
	 *    normal exit
	 */
	negs++;
	last = next;
	return encoding;
}


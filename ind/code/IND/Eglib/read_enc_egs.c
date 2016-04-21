/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"

/*
 *	read encoded example file as a single block
 */
egtype *
read_enc_egs(str)
char    *str;
{
	FILE    *egfile, *efopen(), *fopen();
	Header  head;
	egtype	egs, egs_save, *members;
	int	next, last, rec;
        char    smplname[150];
        Sampler sampler;
	

	if ( !*str )
		error("Encoded examples can't be read from stdin", "");
	
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
                uerror("Could not read from example file", "");
        if (head.h_magicno != MAGICNO)
                uerror("Incorrect example file type -- bad magic number", "");
	if (head.h_ncattrs != ncattrs || (head.h_egsize != egsize) ||
	    head.h_ndattrs != ndattrs || head.h_nsattrs != nsattrs)
                uerror("Example file doesn't match attribute file", "");
        init_sample(head.h_negs,1,&sampler);
	negs = size_sample();
   	egs.unordp = (unordtype *) salloc(negs * egsize);
	egs_save.unordp = egs.unordp;
	last = -1;
	rec = 0;
	members = mems_alloc(egtype,negs);
        while ( (next = next_in_sample()) >= 0 ) {
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
                  if ((fread(egs.unordp, egsize, 1, egfile)) < 1 )
                    uerror("Could not read from encoded example file","");
                }
              else
                if ( fseek(egfile, (long)(next-last-1)*egsize, 1) < 0 )
		  error("Could not seek on encoded example file","");
            }
            if ((fread(egs.unordp, egsize, 1, egfile)) < 1 )
                uerror("Could not read from encoded example file","");
	    members[rec++].unordp = egs.unordp;
	    egs.unordp += egsize;          
          } else
            /*
             *    do nothing, correct record already there
             */
	    members[rec++].unordp = egs.unordp;
	  last = next;
        }
	if ( rec < negs )
	  egs_save.unordp = (unordtype *)
	    resalloc((void*)egs_save.unordp, rec * egsize);
  	if ( ezopened )
        	pclose(egfile);
  	else
        	fclose(egfile);
	return members;
}

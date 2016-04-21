/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"

static      bool  c4_form;

/***************************************************************************/
/*
 *	write_eg(e) -- write eg to file
 */
static write_eg_std(e, outf)
egtype   e;
FILE	*outf;
{
	int   val;
	int   j;
	bitset  sval;
	char	*field_delim, *line_delim;
	bool dec_at_end = 0;	/*  if set, decision attr. printed at end */

	if ( c4_form ) {
		field_delim = ",";
		line_delim = ".\n";
		dec_at_end = 1;
	} else {
		field_delim = " ";
		line_delim = "\n";
	}
	forattrs(j) {
		if ( dec_at_end && j==decattr )
			continue;
		if (num_type(j))
		    if (ord_val(e, j) == FDKNOW)
			fprintf(outf, "?");
		    else
			fprintf(outf, "%g", ord_val(e, j));
		else if (set_type(j)) {
			sval = set_val(e, j);
			if (sval == SDKNOW)
				fprintf(outf, "?");
			else {
			    write_set(outf, sval, j);
			}
		} else {
			val = unord_val(e, j);
			if (val == DONTKNOW)
				fprintf(outf, "?");
			else
				fprintf(outf, "%s", unordvalname(j, val));
		}
		if ( j<nattrs || dec_at_end )
			fputs(field_delim,outf);
	}
	if ( dec_at_end ) {
		val = unord_val(e, decattr);
		fprintf(outf, "%s", unordvalname(decattr, val));
	}
	fprintf(outf, line_delim);
}

write_eg(e, outf)
egtype   e;
FILE	*outf;
{
	c4_form = 0;
	write_eg_std(e, outf);
}

write_c4eg(e, outf)
egtype   e;
FILE	*outf;
{
	c4_form = 1;
	write_eg_std(e, outf);
}

printeg(e)
egtype e;
{
	c4_form = 0;
	write_eg_std(e, stdrep);
}


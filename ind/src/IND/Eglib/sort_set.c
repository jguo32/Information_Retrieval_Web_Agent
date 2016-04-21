/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"

/*
 *  parameters for sort
 */
#define NSTACK 80
#define FM 7875
#define FA 211
#define FC 1663
 

/*****************************************************************
 *	
 *	sort list on numeric attribute value, placing
 *	unkns. at the very top,  uses qsort 
 */
qsort_set(set, attr)
egset	*set;
int	attr;
{
  int n = setsize(set);
  int l=0,jstack=0,j,ir,iq,i;
  int istack[NSTACK+1];
  int attrind = sindex(attr);
  long int fx=0L;
  float a, awstore;
  egtype astore;
  void nrerror();

  if ( !num_type(attr) )
    error("trying to sort non-numeric attribute in qsort_set","");
  ir=n-1;
  if ( weighted(set) ) {
    for (;;) {
      if (ir-l < 7) {
	for (j=l+1;j<=ir;j++) {
	  a=ord_val_s(astore=set->members[j],attrind);
	  awstore=set->weights[j];
	  for (i=j-1; i>=0 && ord_val_s(set->members[i],attrind)>a; i--) {
	    set->members[i+1]=set->members[i];
	    set->weights[i+1]=set->weights[i];
	  }
	  set->members[i+1]=astore;
	  set->weights[i+1]=awstore;
	}
	if (jstack == 0) 
	  return;
	ir=istack[jstack--];
	l=istack[jstack--];
      } else {
	i=l;
	j=ir;
	fx=(fx*FA+FC) % FM;
	iq=l+((ir-l+1)*fx)/FM;
	a = ord_val_s(astore=set->members[iq],attrind);
	awstore=set->weights[iq];
	set->members[iq]=set->members[l];
	set->weights[iq]=set->weights[l];
	for (;;) {
	  while (j >= 0 && a < ord_val_s(set->members[j],attrind)) j--;
	  if (j <= i) {
	    set->members[i]=astore;
	    set->weights[i]=awstore;
	    break;
	  }
	  set->members[i]=set->members[j];
	  set->weights[i++]=set->weights[j];
	  while (i < n && a > ord_val_s(set->members[i],attrind)) i++;
	  if (j <= i) {
	    set->members[j]=astore;
	    set->weights[(i=j)]=awstore;
	    break;
	  }
	  set->members[j]=set->members[i];
	  set->weights[j--]=set->weights[i];
	}
	if (ir-i >= i-l) {
	  istack[++jstack]=i+1;
	  istack[++jstack]=ir;
	  ir=i-1;
	} else {
	  istack[++jstack]=l;
	  istack[++jstack]=i-1;
	  l=i+1;
	}
	if (jstack > NSTACK) error("stack overran in qsort","");
      }
    }
  } else {
    for (;;) {
      if (ir-l < 7) {
	for (j=l+1;j<=ir;j++) {
	  a=ord_val_s(astore=set->members[j],attrind);
	  for (i=j-1; i>=0 && ord_val_s(set->members[i],attrind)>a ;i--) 
	    set->members[i+1]=set->members[i];
	  set->members[i+1]=astore;
	}
	if (jstack == 0) 
	  return;
	ir=istack[jstack--];
	l=istack[jstack--];
      } else {
	i=l;
	j=ir;
	fx=(fx*FA+FC) % FM;
	iq=l+((ir-l+1)*fx)/FM;
	a = ord_val_s(astore=set->members[iq],attrind);
	set->members[iq]=set->members[l];
	for (;;) {
	  while (j >= 0 && a < ord_val_s(set->members[j],attrind)) j--;
	  if (j <= i) {
	    set->members[i]=astore;
	    break;
	  }
	  set->members[i++]=set->members[j];
	  while (i < n && a > ord_val_s(set->members[i],attrind)) i++;
	  if (j <= i) {
	    set->members[(i=j)]=astore;
	    break;
	  }
	  set->members[j--]=set->members[i];
	}
	if (ir-i >= i-l) {
	  istack[++jstack]=i+1;
	  istack[++jstack]=ir;
	  ir=i-1;
	} else {
	  istack[++jstack]=l;
	  istack[++jstack]=i-1;
	  l=i+1;
	}
	if (jstack > NSTACK) error("stack overran in qsort","");
      }
    }
  }
}


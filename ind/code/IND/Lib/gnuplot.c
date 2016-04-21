/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include <math.h>
#include "Lib.h"
extern  char * progname;

#define DEF_WIDTH  295   /* width of a gain graph  */
#define DEF_HEIGHT 195   /* height of a gain graph */
#define NGP 12           /* number of graph positions in position arrays */

static int     no_graphs;      /* used only to control graph positions */
static int     x_pos[NGP] = {850,850,850,850,550,550,550,550,250,250,250,250};
static int     y_pos[NGP] = {100,300,500,700,100,300,500,700,100,300,500,700};

void init_graph_panes ()
{ no_graphs = 0; }

call_gnuplot(fname,aname)
char   *fname;
char   *aname;
{
  char   cmd[200];
  int fdp[2]; /* fd pairs for pipe(2) */
  int graph_pid;   /*  pid for child gnuplot  */
  extern   char  *progname;
  char   buff[150];

  /*
   *    fork with an output pipe to the new graph process
   *        NB>   should just use popen(), but too late!
   */
  if (pipe(fdp))
    uerror("could not create pipe to gnuplot", "");
  if ((graph_pid = fork()) == -1)
    uerror("could not create gnuplot process", "");

  if (graph_pid == 0) /* we're in first child: exec gnuplot */
   {
    (void)dup2(fdp[0], 0); /* dup stdin  to parent */
    (void)close(fdp[0]);
    (void)close(fdp[1]);
    sprintf(buff, "=%dx%d+%d+%d", DEF_WIDTH, DEF_HEIGHT,
          x_pos[no_graphs % NGP], y_pos[no_graphs % NGP]);
    execlp("gnuplot", "gnuplot", "-geometry", buff, 0 );
    fprintf(stderr,"%s: exec on gnuplot failed\n",progname);
    perror(progname);
    /* _exit(errno); */
    _exit(1);
   }
  /* in parent with first child created, close pipe output */
  (void)close(fdp[0]);
  /*   save child pid's for later killing and file for deleting */
  (void)add_childproc(graph_pid);

  /*
   *    send plotting commands to the graph process
   */
  sprintf(buff, "set terminal x11 ; set autoscale xy\n");
  write(fdp[1], &buff[0], strlen(&buff[0]) );
  sprintf(buff, "plot \"%s\" title \"%s\" with points\n", fname, aname);
  write(fdp[1], &buff[0], strlen(&buff[0]) );
  sprintf(buff, "pause 500\n");
  write(fdp[1], &buff[0], strlen(&buff[0]) );
  (void)close(fdp[1]);

  /*
   *    update position for next graph
   */
   no_graphs++;
}


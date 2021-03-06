/*
** copyright (c) notice
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <main.h>

#include <termio.h>    /* struct termios */
#include <sys/time.h>  /* struct timeval */
#include <signal.h>

#include <list.h>
#include <sockets.h>
#include <devicereg.h>
#include <structs.h>
#include <telnet.h>
#include <misc.h>
#include <sredird.h>
#include <prefs.h>


 MODULEID("$Id: misc.c,v 1.2 1999/09/24 04:26:05 ayman Exp $");


 /*
 ** Called from main() as part of the initialization process.
 ** Open the master log file and collect some useful info.
 ** Will abort the program if log file cannot be opened.
 */
 void InitMsredird (void)

 {
  char *s;
  char frmttime [80];
  char path[_POSIX_PATH_MAX];
  extern const char *c_version;
  extern msredird *const masterptr;
  pthread_mutex_t auxmutex=PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t auxcond=PTHREAD_COND_INITIALIZER;
   masterptr->when=time (NULL);

   masterptr->nready=0;
   
   masterptr->nready_mutex=auxmutex;
   masterptr->nready_cond=auxcond;

   strftime (frmttime, 79, "%H:%M%p %Z on %a, %d %b '%y",
             localtime(&masterptr->when));

    /* sanity check... should be removed */  
    if (!(s=valueof("CONFIG_DIR")))
     {
      say ("!!! Reassigning CONFIG_DIR...\n");

      SetPref ("CONFIG_DIR", (s="/etc/msredird"));
     } 

   sprintf (path, "%s/log/%s", s, MASTER_LOG_FILE_NAME);
   masterptr->logf.fname=strdup (path);

  
    if (!(masterptr->logf.file=fopen(path, "a+")))
     {
      say ("### Unable to open session log file %s: %s--aborting...\n",
           path, strerror(errno));

      _exit (1);
     }

   setlinebuf (masterptr->logf.file);

   logf(masterptr, "\n000 Started %s\n\n", frmttime);

   FetchLocalhost ();

   say ("*** Local time %s\n", frmttime);  

 }  /**/


 signal_f *nsignal (int signo, signal_f *func)

 {
  struct sigaction  sa,
                    osa;

   sa.sa_handler=func;
   sigemptyset (&sa.sa_mask);
   sa.sa_flags|=SA_RESTART;

    if (sigaction(signo, &sa, &osa)<0)
     {
      return (SIG_ERR);
     }

   return (osa.sa_handler);

 }  /**/


 void InitSignals (void)

 {
   if (nsignal(SIGPIPE, SIG_IGN)==SIG_ERR)
     {
      psignal (SIGPIPE, "Couldn't install signal handler");
      exit (0);
     }

 }  /**/


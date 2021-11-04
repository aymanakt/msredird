/*
** Multi-modem support added by Ayman Akt of Uniware P/L Australia
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <main.h>

#include <termio.h>    /* struct termios */
#include <sys/time.h>  /* struct timeval */

#include <list.h>
#include <sockets.h>
#include <devicereg.h>
#include <session.h>
#include <sredird.h>

 static Session *Align (void);
 static void Shoot (Session *);


 void *FiringSquad (void *args)

 {
  Session *sesnptr;
  extern msredird *const masterptr;

   pthread_mutex_lock (&masterptr->nready_mutex);

    while (1!=2)
     {
       while (masterptr->nready==0)
	{
         /* lock is released at this point */
	 pthread_cond_wait (&masterptr->nready_cond, &masterptr->nready_mutex);
	}

      /* lock re-acquired... */
       if ((sesnptr=Align()))
        {
         Shoot (sesnptr);
         say ("a thread exited...\n");
         masterptr->nready--;

         PrintSessions ();
        }
     }

   return NULL;

 }  /**/


 static Session *Align (void)

 {
  Session *sesnptr;
  
    while (sesnptr=GetFlaggedSession(FIRING_SQUAD))
     {
       if (sesnptr->stat&FIRING_SQUAD)  /* redundant... */
        {
         CloseSession (sesnptr);

         return (Session *)sesnptr;
        }
     }

   return (Session *)NULL;

 }  /**/


 static void Shoot (Session *sesnptr)

 {
   pthread_join (sesnptr->th, NULL);

   KillSession (sesnptr);

 }  /**/


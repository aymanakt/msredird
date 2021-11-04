/*
** copyright (c) notice
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

 MODULEID("$Id:$");

 static List MasterSession;
 static List *const msesnptr=&MasterSession;

 static Session *_s_LocateSession (List *, int);
 static int _s_CloseSession (List *, Session *);
 static int _s_KillSession (List *, Session *);
 static Session *_s_OpenSession (List *, Modem *, Socket *);
 static Session *_s_GetFlaggedSession (List *, unsigned);

 /*
 ** Wrapped functions for export... -------------------------------------
 */

 Session *LocateSession (int fd)

 {
   return ((Session *)_s_LocateSession(msesnptr, fd));

 }  /**/


 int CloseSession (Session *sesnptr)

 {
   return (_s_CloseSession(msesnptr, sesnptr));

 }  /**/


 int KillSession (Session *sesnptr)

 {
   return (_s_KillSession(msesnptr, sesnptr));
 }  /**/


 Session *OpenSession (Modem *mptr, Socket *sptr)

 {
   return ((Session *)_s_OpenSession(msesnptr, mptr, sptr));

 }  /**/


 Session *GetFlaggedSession (unsigned flag)

 {
   return ((Session *)_s_GetFlaggedSession(msesnptr, flag));

 }  /**/


 /*
 ** End of export functions... --------------------------------------------
 */


 /*
 ** Return the session which is monitoring fd (be it socket or device...)
 */
 static Session *_s_LocateSession (List *lstptr, int fd)

 {
  register ListEntry *eptr;
  extern msredird *const masterptr;

    if (lstptr->nEntries==0)  return (Session *)NULL;

    for (eptr=lstptr->head; eptr; eptr=eptr->next)
     {
       if ((((Session *)(eptr->whatever))->sptr->sock==fd)||
           (((Session *)(eptr->whatever))->mptr->devfd==fd))
        {
         return ((Session *)(eptr->whatever));
        }
     }

   return (Session *)NULL;

 }  /**/


 static Session *_s_OpenSession (List *lstptr, Modem *mptr, Socket *sptr)

 {
  ListEntry *eptr;
  Session *sesnptr;

   xmalloc(sesnptr, (sizeof(Session)));
   memset (sesnptr, 0, sizeof(Session));
   eptr=AddtoList (lstptr);
   (Session *)eptr->whatever=sesnptr;

   sesnptr->when=time (NULL);
   sesnptr->mptr=mptr;
   sesnptr->sptr=sptr;
   
    if (!(OpenSessionLogFile(sesnptr)))
     {
      /*logf(masterptr, "005 Couldn't open log file for incoming connection"
	   "from %s dev(%s)--closing conection...\n",
	   sptr->haddress, mptr->devname);*/

      free (sesnptr);
      RemovefromList (lstptr, eptr);

      return (Session *)NULL;
     }

   return (Session *)sesnptr;

 }  /**/


 /*
 ** The session is functionally closed and all data structures are freed
 ** but the sesnptr is still in the list.
 */
 static int _s_CloseSession (List *lstptr, Session *sesnptr)

 {
  char frmttime[10];
  register ListEntry *eptr;
  time_t now;
  extern msredird *const masterptr;

    for (eptr=lstptr->head; eptr; eptr=eptr->next)
     {
       if (((Session *)eptr->whatever)==sesnptr)
        {
         now=time (NULL);
         logf(masterptr, "014 Closing connection from %s. %s will be shutdown"
              ", too.\n", sesnptr->sptr->haddress, sesnptr->mptr->devname);

         CloseDevice (sesnptr->mptr->devfd);

         /* do socket... */
         close (sesnptr->sptr->sock);
         free (sesnptr->sptr);

         fflush (sesnptr->logf.file);
         fclose (sesnptr->logf.file);
         //RemovefromList (lstptr, eptr);

         return 1;
        }
     }

   return 0;

 }  /**/


 /*
 ** Remove all traces of session
 */
 static int _s_KillSession (List *lstptr, Session *sesnptr)

 {
  register ListEntry *eptr;

    for (eptr=lstptr->head; eptr; eptr=eptr->next)
     {
       if (((Session *)eptr->whatever)==sesnptr)
        {
         RemovefromList (lstptr, eptr);

         return 1;
        }
     }

   return 0;

 }  /**/


 static Session *_s_GetFlaggedSession (List *lstptr, unsigned flag)

 {
  register ListEntry *eptr;
  Session *sesnptr;

    for (eptr=lstptr->head; eptr; eptr=eptr->next)
     {
       if (((Session *)eptr->whatever)->stat&flag)
        {
         return ((Session *)eptr->whatever);
        }
     }
   
   return ((Session *)NULL); 

 }  /**/


 void CloseSessionLogFile (Session *sesnptr)

 {
  char path[_POSIX_PATH_MAX];
  char frmttime[80];
  time_t now=time (NULL);

   strftime (frmttime, 79, "%H:%M%p %Z on %a, %d %b '%y",
             localtime(&now));

   fprintf (sesnptr->logf.file,
            "\n---Logging ended %s.\n---Source IP: %s."
            " (devfd[%d] sock[%d])\n",
            frmttime, sesnptr->sptr->haddress, sesnptr->mptr->devfd,
            sesnptr->sptr->sock);

   fclose (sesnptr->logf.file);

 }  /**/


 int OpenSessionLogFile (Session *sesnptr)

 {
  char path[_POSIX_PATH_MAX];
  char frmttime[80];

    if (!(valueof("CONFIG_DIR")))
     {
      SetPref ("CONFIG_DIR", "/etc/msredird");
     }

   sprintf (path, "%s/log/%s.log", valueof("CONFIG_DIR"),
            strrchr (sesnptr->mptr->devname, '/'));

    if (!(sesnptr->logf.file=fopen(path, "a+")))
     {
      say ("### Unable to open session log file for %s: %s\n",
           path, strerror(errno));

      return 0;
     }

   setlinebuf (sesnptr->logf.file);

   strftime (frmttime, 79, "%H:%M%p %Z on %a, %d %b '%y",
             localtime(&sesnptr->when));

   fprintf (sesnptr->logf.file,
            "\n---Logging started %s.\n---Source IP: %s."
            " (devfd[%d] sock[%d])\n",
            frmttime, sesnptr->sptr->haddress, sesnptr->mptr->devfd,
            sesnptr->sptr->sock);

   return 1;

 }  /**/

 
 void PrintSessions (void)

 {
  char s[11];
  register ListEntry *eptr;
  register Session *sesnptr;
  extern msredird *const masterptr;

   say ("*** Number of maintained devices %d\n", HowManyInDeviceReg());

    if (msesnptr->nEntries==0)
     {
      say ("*** Connections Table empty.\n");

      return;
     }

   say ("*** Number of active connections %d\n", msesnptr->nEntries);

   say ("+-----+-----+------------+--------------------+-----+\n");
   say ("|Sock |Devfd|%-12.12s|%-20.20s|When |\n",
        "Dev Name", "Connecting Host");
   say ("+-----+-----+------------+--------------------+-----+\n");

    for (eptr=msesnptr->head; eptr; eptr=eptr->next)
     {
      sesnptr=(Session *)eptr->whatever;

      strftime (s, 10, "%H:%M", localtime(&sesnptr->when));

      say ("|%-5d|%-5d|%-12.12s|%-20.20s|%-5.5s|\n",
           sesnptr->sptr->sock,
           sesnptr->mptr->devfd,
           sesnptr->mptr->devname,
           sesnptr->sptr->haddress,
           s);
     }

   say ("+-----+-----+------------+--------------------+-----+\n");

 }  /**/
 

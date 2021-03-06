/*
** Multi-modem support added by Ayman Akt of Uniware P/L Australia
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <main.h>

#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <syslog.h>
#include <termios.h>
#include <termio.h>

#include <sockets.h>
#include <list.h>
#include <structs.h>
#include <prefs.h>
#include <devicereg.h>
#include <session.h>
#include <misc.h>
#include <telnet.h>
#include <sredird.h>
#include <firingsquad.h>


MODULEID("$Id: main.c,v 1.2 1999/09/24 04:25:47 ayman Exp $");

 static int ReadFromSocket (Session *);
 static int ReadFromDevice (Session *);
 static void CleanUpUnverviedDevices (void);
 static void Banner (void);

 const char *c_version="0.2";

 /* encapsulates all global variables */
 msredird master;
 msredird *const masterptr=&master;


 /* HOOK */
 int main (int argc, char *argv[])

  {
   pthread_t firingsquad;
   extern int ProcessConfigfile (void);

    Banner ();

    BuildPreferencesTable ();
    ProcessConfigfile ();

    InitMsredird ();

    BuildDeviceRegistry ();
    VerifyDeviceRegistry ();
    CleanUpUnverviedDevices ();

    InitSignals ();
    InitNet ();


    pthread_create (&firingsquad, NULL, FiringSquad, NULL);

    /* Register the function to be called on break condition */
    //signal (SIGINT, BreakFunction);

    {
     int x;
     fd_set fd,
            xfd;

      again:
      FD_ZERO(&fd);
      FD_ZERO(&xfd);

      FD_SET(STDIN_FILENO, &fd);
      FD_SET(masterptr->telnetsock->sock, &fd);

       while (1!=2)
        {
         x=select(FD_SETSIZE, &fd, NULL, NULL, NULL);

          if (x>0)  /* readable input */
           {
             if (FD_ISSET(STDIN_FILENO, &fd))
              {
              }
             else
             if (FD_ISSET(masterptr->telnetsock->sock, &fd))
              {
               AnswerTelnetRequest ();
              }
           }
          else
          if (x==0)  /* timeout */
           {
           }
          else
          if ((x==-1)&&(errno!=EINTR))  /* select error */
           {
            say ("### select error: %s\n", strerror(errno));

            goto again;
           }

        }  /* while (1==1) */

    } /* END OF BLOCK */

 }  /*EOM*/


 void *Thread (void *ptr)

 {
  int x,
      max;
  fd_set rfd,
         wfd,
         xfd;
  Session *sesnptr=(Session *)ptr;
  Modem *mptr;
  Socket *sptr;

    if (!sesnptr)
     {
      say ("sesnptr=0!\n");

      pthread_exit (NULL);      
     }

   sesnptr->pid=getpid();

   mptr=sesnptr->mptr;
   sptr=sesnptr->sptr;

   /* polling timing stuff... */
   mptr->ETimeout=&mptr->RTimeout;
   mptr->OutBTicks=times(NULL);
  
   mptr->BTimeout.tv_sec=0;
   mptr->BTimeout.tv_usec=ModemStatePolling*1000;
   mptr->MaxBTicks=(mptr->BTimeout.tv_usec*CLOCKS_PER_SEC)/(1000*1000);

    if (mptr->ETimeout!=NULL)  *mptr->ETimeout=mptr->BTimeout;

   (sesnptr->mptr->devfd>sesnptr->sptr->sock)?
    (max=sesnptr->mptr->devfd):(max=sesnptr->sptr->sock);
 
   again:
   FD_ZERO (&rfd);
   FD_ZERO (&wfd);
   FD_ZERO (&xfd);

   FD_SET(sptr->sock, &rfd);
   FD_SET(mptr->devfd, &rfd);

    if (IsBufferEmpty(&mptr->InBuffer)==False)
     { /* only set writefd if we got something to send to the modem */
      FD_SET(mptr->devfd, &wfd);
     }


   {
     while (1!=2)
      {
       x=select(max+1, &rfd, &wfd, &xfd, mptr->ETimeout);

	if (x>0)  /* readable input */
	 {
	   if (FD_ISSET(sptr->sock, &rfd)) /* socket... */
	    {
             ReadFromSocket (sesnptr);
	    }
	   else
	   if (FD_ISSET(mptr->devfd, &rfd))  /* device... */
	    {
	     ReadFromDevice (sesnptr);
	    }

	   if (FD_ISSET(mptr->devfd, &wfd))
	    {
	     EscRedirectChar (mptr->devfd, GetFromBuffer(&mptr->InBuffer));
	    }

           /* Check for output buffer timeout */
           if ((mptr->MaxBTicks>0)&&(times(NULL)-mptr->OutBTicks)>=mptr->MaxBTicks)
            {
             EscWriteBuffer (sptr->sock, mptr->OutBuffer, mptr->OutBPos, 1);
             mptr->OutBPos=0;
             mptr->OutBTicks=times(NULL);
            }

          /* check for exception errors */

	 }
	else
	if (x==0)  /* timeout */
	 {
          /* send output buffer-stuff from modem */
          EscWriteBuffer(sptr->sock, mptr->OutBuffer, mptr->OutBPos, 2);
          mptr->OutBPos=0;
          mptr->OutBTicks=times(NULL);
	 }
	else
	if (x==-1)  /* select error */
	 {
	  say ("### select error: %s\n", strerror(errno));

	  goto again;
	 }

       {   /* FALL-THROUGH BLOCK */

	 if ((mptr->TCPCEnabled==True)&&(mptr->InputFlow==True)) 
	  {
	    if (((GetModemState(mptr->DeviceFd, mptr->ModemState)&
		 (mptr->ModemStateMask)&(ModemStateECMask))) !=
		 ((mptr->ModemState)&(mptr->ModemStateMask)&
		 (ModemStateECMask)))
	     {
	      mptr->ModemState=GetModemState(mptr->devfd, mptr->ModemState);

	      SendCPCByteCommand (sesnptr, TNASC_NOTIFY_MODEMSTATE,
	       (mptr->ModemState&mptr->ModemStateMask));
	      //log ("Sent modem state: %u",
		  //(unsigned int) (mptr->ModemState&mptr->ModemStateMask));
	      } 
	  }

         if (IsBufferEmpty(&mptr->InBuffer)==False)
          { /* only set writefd if we got something to send to the modem */
           FD_SET(mptr->devfd, &wfd);
          }
         else
          {
           FD_ZERO(&wfd);
          }

       }   /* END-OF FALL-THROUGH BLOCK */

       FD_SET(sptr->sock, &rfd);
       FD_SET(mptr->devfd, &rfd);

        if (mptr->ETimeout!=NULL)  *mptr->ETimeout=mptr->BTimeout;

      }  /* while (1!=2) */

   }  /* END OF BLOCK */

 }  /**/


 static int ReadFromSocket (Session *sesnptr)

 {
  Modem *mptr;
  Socket *sptr;

    if (!sesnptr)
     {
      return -1;
     }

   /* sanity check? */
   mptr=sesnptr->mptr;
   sptr=mptr->sptr;

    if (read(sptr->sock, &mptr->C, 1)==1)
     {
      AddToBuffer (&mptr->InBuffer, mptr->C);

      //log ("read %s from socket %d\n",
      //     ((Modem *)sptr->index)->InBuffer.Buffer,
      //     sptr->sock);
      return 1;
     }
    else
     {
      say ("Communication closed (input) %d\n", sptr->sock, mptr->devfd);

      sesnptr->stat|=FIRING_SQUAD;

      pthread_mutex_lock (&masterptr->nready_mutex);
      masterptr->nready++;
      pthread_cond_signal (&masterptr->nready_cond);
      pthread_mutex_unlock (&masterptr->nready_mutex);

      pthread_exit (NULL);
     }

 }  /**/


 static int ReadFromDevice (Session *sesnptr)

 {
  Modem *mptr;
  Socket *sptr;

    if (!sesnptr)
     {
      return -1;
     }

   /* sanity check? */
   mptr=sesnptr->mptr;
   sptr=mptr->sptr;
 
    if (read(mptr->devfd, &mptr->C, 1)==1)
     {
       if (mptr->MaxBTicks>0)  /* always true */
	{
	 mptr->OutBuffer[mptr->OutBPos]=mptr->C;
	 mptr->OutBPos++;

	  if (mptr->OutBPos==OutBufferSize)
	   {
	    logf(sesnptr, "mptr->OutBPos==OutBufferSize--"
		 "calling EscWriteBuffer()\n");

	     EscWriteBuffer (sptr->sock, mptr->OutBuffer,
			     mptr->OutBPos, 0);
	     mptr->OutBPos=0;
	     mptr->OutBTicks=times (NULL);
	    }
	}
       else
	{
	 EscWriteChar (sptr->sock, mptr->C);
	}

      return 1;
     }
    else
     {
      say ("Error reading from device, may be power lost");

      logf(sesnptr, "Error reading from device %d-"
	   "Power loast?\n", mptr->devfd);

      sesnptr->stat|=FIRING_SQUAD;

      pthread_mutex_lock (&masterptr->nready_mutex);
      masterptr->nready++;
      pthread_cond_signal (&masterptr->nready_cond);
      pthread_mutex_unlock (&masterptr->nready_mutex);

      pthread_exit (NULL);
     }

 }  /**/


 static void Banner (void)

 {
   say ("*** msredird %s.\n", c_version);

 }  /**/


 static void CleanUpUnverviedDevices (void)

 {
    if (masterptr->discard)
     {
      say ("*** Cleaning up registry...\n");

      DiscardUnverifiedDevices ();

      masterptr->discard=0;  /* reset */
     }

 }  /**/


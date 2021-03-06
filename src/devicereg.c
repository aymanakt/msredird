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


 MODULEID("$Id: devicereg.c,v 1.2 1999/09/23 08:23:12 ayman Exp $");

 static List DeviceRegistry;
 static List *const devregptr=&DeviceRegistry;


 static void _s_BuildDeviceRegistry  (List *);
 static void _s_VerifyDeviceRegistry  (List *);
 static void _s_DiscardUnverifiedDevices (List *);
 static int _s_HowManyInDeviceReg (List *);
 static ListEntry *_s_GetTaggedDeviceRegEntry (List *, unsigned);
 static Modem *_s_GetFlaggedDevice (List *, unsigned);
 static Modem *_s_LocateDevice (List *, int);
 static Modem *_s_LocateFreeDevice (List *);
 static Modem *_s_OpenDevice (List *);
 static int _s_CloseDevice (List *, int);



 /*
 ** Wrappers for export... ------------------------------------------------
 */
 void BuildDeviceRegistry (void)

 {
   _s_BuildDeviceRegistry (devregptr);

 }  /**/


 void VerifyDeviceRegistry (void)

 {
   _s_VerifyDeviceRegistry (devregptr);

 }  /**/


 void DiscardUnverifiedDevices (void)

 {
   _s_DiscardUnverifiedDevices (devregptr);

 }  /**/


 Modem *OpenDevice (void)

 {
   return ((Modem *)_s_OpenDevice(devregptr));

 }  /**/


 int HowManyInDeviceReg (void)

 {
   return _s_HowManyInDeviceReg (devregptr);

 }  /**/


 Modem *GetTaggedDeviceRegEntry (unsigned tag)

 {
  ListEntry *eptr;

   eptr=_s_GetTaggedDeviceRegEntry (devregptr, tag);

   return eptr?((Modem *)eptr->whatever):((Modem *)NULL);

 }  /**/


 Modem *GetFlaggedDevice (unsigned flag)

 {
   return ((Modem *)_s_GetFlaggedDevice(devregptr, flag));

 }  /**/


 Modem *LocateFreeDevice (void)

 {
   return ((Modem *)_s_LocateFreeDevice(devregptr));

 }  /**/


 Modem *LocateDevice (int fd)

 {
   return (_s_LocateDevice(devregptr, fd));

 }  /**/


 int CloseDevice (int devfd)

 {
   return (_s_CloseDevice(devregptr, devfd));

 }  /**/

 /*
 ** End of export functions ------------------------------------------------
 */


 /*
 ** Builds up a linked list of devices
 ** Will abort if no device list is defined.
 */
 static void _s_BuildDeviceRegistry  (List *lstptr)

 {
  register int i;
  char *devlst;
  register ListEntry *eptr=NULL;
  TokensHdr tokshdr;
  extern msredird *const masterptr;

    if (!(devlst=getenv ("MSREDIRD_DEVLST")))
     {
      say("### Enviroment variable `MSREDIRD_DEVLST'"
          " isn't defined--aborting...\n");

      _exit (-1);
     }

   SplitTokens (&tokshdr, devlst, 16, ':');

    if (tokshdr.ntoks==0)
     {
      say ("001 No devices to be opened--aborting...\n");

      fprintf (masterptr->logf.file,
               "001 No devices to be opened--aborting...\n");

      _exit (-1);
     }

    for (i=1; i<=tokshdr.ntoks; i++)
     {
      Modem *mptr;

       xmalloc(mptr, (sizeof(Modem)));
       memset (mptr, 0, sizeof(Modem));

       eptr=AddtoList (lstptr);
       (Modem *)eptr->whatever=mptr;

       mptr->devname=mptr->DeviceName=strdup(TOK(&tokshdr, i));
       say ("*** Building device list...(%d)\r", i);
     }

   say ("\n");
 
   logf(masterptr, "002 Built %d devic%s into list.\n",  --i, i>1?"es":"e");

 }  /**/


 /*
 ** Called in main() after BuildDeviceRegistry() has finished building up
 ** the device registry.
 ** Each node in the list is checked, if cannot be opened, it will be
 ** removed from the list.
 */
 static void _s_VerifyDeviceRegistry  (List *lstptr)

 {
  int fd,
      n=0;
  register ListEntry *eptr;
  register Modem *mptr;
  extern msredird *const masterptr;

    if (lstptr->nEntries==0)
     {
      say ("### Unable to verify device registry: registry empty!\n");

      return;
     } 

    for (eptr=lstptr->head; eptr; eptr=eptr->next)
     {
      mptr=(Modem *)eptr->whatever; 

       if (mptr)
        {
          if ((fd=open(mptr->DeviceName, O_RDWR|O_NOCTTY, 0))==(-1))
           {
            say ("003 Device %s failed verify test: (%s)--discarding...\n",
                 mptr->DeviceName, strerror(errno));

            logf(masterptr, 
                 "003 Device %s failed verify test: (%s)--discarding...\n",
                 mptr->DeviceName, strerror(errno));

            mptr->tmpstat|=1;

            continue;
           }

         close (fd);

         n++;
        }
     }

   say("004 Verfied %d devic%s...\n", n, n>1?"es":"e");
   logf(masterptr,
        "004 Verfied %d devic%s...\n", n, n>1?"es":"e");

    if (n<lstptr->nEntries)  
     {
      extern msredird *const masterptr;

       masterptr->discard=1;
     }

 }  /**/


 static void _s_DiscardUnverifiedDevices (List *lstptr)

 {
  int n=0;
  register ListEntry *eptr;
  register Modem *mptr;
  extern msredird *const masterptr;

    if (lstptr->nEntries==0)
     {
      say ("### Unable to clean up registry: registry empty!\n");

      return;
     }

    while (1==1)
     {
      eptr=_s_GetTaggedDeviceRegEntry (lstptr, 1);

       if (eptr)
        {
         mptr=(Modem *)eptr->whatever;
         say("006 Removing ``%s'' from registry...\n", mptr->devname);
         logf(masterptr,
              "006 Removing ``%s'' from registry...\n", mptr->devname);
         free (mptr->DeviceName);
         free (eptr->whatever);
         RemovefromList (lstptr, eptr);

         n++;
        }
       else
        {
          if (lstptr->nEntries==0)
           {
            say ("*** No more devices left in registry--"
                 "operation aborted...\n");
           }

         break;
        }
     }

   say("008 Discarded %d entr%s from registry.\n", n, n>1?"ies":"y");
   logf(masterptr,
        "008 Discarded %d entr%s from registry.\n", n, n>1?"ies":"y");

 }  /**/


 static int _s_HowManyInDeviceReg (List *lstptr)

 {
   return lstptr->nEntries;

 }  /**/


 /*
 ** Returns the node tagged with 'tag'
 */
 static ListEntry *_s_GetTaggedDeviceRegEntry (List *lstptr, unsigned tag)

 {
  register ListEntry *eptr; 
  register Modem *mptr;

    for (eptr=lstptr->head; eptr; eptr=eptr->next)
     {
      mptr=(Modem *)eptr->whatever;

       if (mptr)
        {
          if (mptr->tmpstat&tag)  /* tagged as unusable */
           {
            return eptr;
           }
        }
     }

   return (ListEntry *)NULL;

 }  /**/


 static Modem *_s_GetFlaggedDevice (List *lstptr, unsigned flag)

 {
  register ListEntry *eptr;
  register Modem *mptr;

    for (eptr=lstptr->head; eptr; eptr=eptr->next)
     {
       if (eptr->whatever)
        {
          if (((Modem *)eptr->whatever)->flags&flag)
           {
            return ((Modem *)eptr->whatever);
           }
        }
     }

   return (Modem *)NULL;

 }  /**/


 /*
 ** Return ptr to free modem for use in new connection
 */
 static Modem *_s_LocateFreeDevice (List *lstptr)

 {
  register ListEntry *eptr;
  register Modem *mptr;

    for (eptr=lstptr->head; eptr; eptr=eptr->next)
     {
       if ((mptr=(Modem *)eptr->whatever), mptr)
        {
          if (!mptr->inuse)
           {
            return (Modem *)mptr;
           }
        }
     }

   return (Modem *)NULL;

 }  /**/


 /*
 ** Open modem and set required parameters - almost ready for use
 */
 static Modem *_s_OpenDevice (List *lstptr)

 {
  register Modem *mptr;
  extern msredird *const masterptr;
  char s[]="~ATZ~";

    if (!(mptr=_s_LocateFreeDevice(lstptr)))
     {
      return (Modem *)NULL;
     }

   mptr->devfd=mptr->DeviceFd=open (mptr->devname, O_RDWR|O_NOCTTY, 0);

    if (mptr->devfd==-1)
     {
      say("007 Unable to open device %s: %s\n", 
          mptr->devname, strerror(errno));

      logf(masterptr, "007 Unable to open device %s: %s\n", 
           mptr->devname, strerror(errno));
      /* tag for clean-up ? */

      return (Modem *)NULL;
     }

   mptr->inuse=mptr->DeviceOpened=1;
   masterptr->inuse++;

   tcflush (mptr->devfd,TCIOFLUSH);
   /* Get the actual port settings */
   tcgetattr (mptr->devfd, &mptr->InitialPortSettings);
   mptr->InitPortRetrieved=True;
   tcgetattr (mptr->devfd, &mptr->PortSettings);

   /* Set the serial port to raw mode */
   cfmakeraw (&mptr->PortSettings);

   /* Enable HUNGUP on close and disable modem control line handling */
   mptr->PortSettings.c_cflag=(mptr->PortSettings.c_cflag|HUPCL)|CLOCAL;

   /* Enable break handling */
   mptr->PortSettings.c_iflag=(mptr->PortSettings.c_iflag&~IGNBRK)|BRKINT;

   /* Write the port settings to device */
   tcsetattr (mptr->devfd, TCSANOW, &mptr->PortSettings);

   mptr->IACEscape=IACNormal;
   mptr->TCPCEnabled=False;
   mptr->ModemStateMask=((unsigned char )255);
   mptr->LineStateMask=((unsigned char )0);
   mptr->ModemState=((unsigned char )0);
   mptr->InputFlow=True;
   mptr->OutBPos=0;
   mptr->InitPortRetrieved=False;
   InitBuffer (&mptr->InBuffer);

   return (Modem *)mptr;

 }  /**/


 /*
 ** return the open device represented by devfd
 */
 static Modem *_s_LocateDevice (List *lstptr, int devfd)

 {
  register ListEntry *eptr;
  register Modem *mptr;

    for (eptr=lstptr->head; eptr; eptr=eptr->next)
     {
       if ((mptr=(Modem *)eptr->whatever), mptr)
        {
          if (mptr->devfd==devfd)
           {
            return (Modem *)mptr;
           }
        }
     }

   return (Modem *)NULL;

 }  /**/


 static int _s_CloseDevice (List *lsptr, int devfd)

 {
  Modem *mptr;
  register Session *sesnptr;
  extern List *sesptr;
  extern msredird *const masterptr;

    if (!(mptr=_s_LocateDevice(lsptr, devfd)))
     {
      logf(masterptr, 
           "-009- Unable to close device %d-not in the list!\n", devfd);

      return 0;
     }

   close (mptr->devfd);

   mptr->inuse=0;

   return 1;

 }  /**/


 int SendToModem (int fd, char *str)

 {
  register char *s=str;

   while (*s)
    {
     write (fd, s, 1); s++;
    }
 
 }  /**/


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
#include <protocol.h>

MODULEID("$Id:$");


/* Line state mask set by the client */
static unsigned char LineStateMask=((unsigned char) 0);



 /* Initialize a buffer for operation */
 void InitBuffer (BufferType *B)

 {
   /* Set the initial buffer positions */
   B->RdPos=0;
   B->WrPos=0;

 }  /**/


 /* Check if the buffer is empty */
 Boolean IsBufferEmpty (BufferType *B)
 
 {
   return ((Boolean) B->RdPos==B->WrPos);

  }  /**/


 /* Check if the buffer is full */
 Boolean IsBufferFull (BufferType *B)

 {
   return ((Boolean) B->WrPos==(B->RdPos+BufferSize-1)%BufferSize);

 }  /**/


 /* Add a byte to a buffer */
 void AddToBuffer (BufferType *B, unsigned char C)

 {
   B->Buffer[B->WrPos]=C; 
   B->Buffer[B->WrPos+1]=0;

   B->WrPos=(B->WrPos+1)%BufferSize;

 }  /**/

    
 /* Get a byte from a buffer */
 unsigned char GetFromBuffer (BufferType *B)

 {
  unsigned char C=B->Buffer[B->RdPos];
   
   B->RdPos=(B->RdPos+1)%BufferSize;

   return (C);

 }  /**/

  
 /* Generic log function with log level control. Uses the same log levels
 ** of the syslog(3) system call 
 */
 void LogMsg (int LogLevel, char *Msg)

 {
 static int MaxLogLevel=LOG_DEBUG+1;

    if (LogLevel<=MaxLogLevel)  syslog(LogLevel,Msg);

 }  /**/


 /* Try to lock the file given in LockFile as pid LockPid using the classical
  HDB (ASCII) file locking scheme */
 int HDBLockFile(char * LockFile, pid_t LockPid)

  {
#if 0
   pid_t Pid;
   int FileDes;
   int N;
   char HDBBuffer[HDBHeaderLen + 1];
   char LogStr[TmpStrLen];

    /* Try to create the lock file */
    while ((FileDes = open(LockFile,O_CREAT | O_WRONLY | O_EXCL,
      LockFileMode)) == OpenError)
      {
        /* Check the kind of error */
        if ((errno == EEXIST) && 
          ((FileDes = open(LockFile,O_RDONLY,0)) != OpenError))
          {
            /* Read the HDB header from the existing lockfile */
            N = read(FileDes,HDBBuffer,HDBHeaderLen);
            close(FileDes);

            /* Check if the header has been read */
            if (N <= 0)
              {
                /* Emtpy lock file or error: may be another application
                  was writing its pid in it */
                sprintf(LogStr,"Can't read pid from lock file %s",LockFile);
                LogMsg(LOG_NOTICE,LogStr);

                /* Lock process failed */
                return(LockKo);
              }

            /* Gets the pid of the locking process */
            HDBBuffer[N] = '\0';
            Pid = atoi(HDBBuffer);

            /* Check if it is our pid */
            if (Pid == LockPid)
              {
                /* File already locked by us */
                sprintf(LogStr,"Read our pid from lock %s",LockFile);
                LogMsg(LOG_DEBUG,LogStr);

                /* Lock process succeded */
                return(LockOk);
              }

            /* Check if hte HDB header is valid and if the locking process
              is still alive */
            if ((Pid == 0) || ((kill(Pid,0) != 0) && (errno == ESRCH)))
              /* Invalid lock, remove it */
              if (unlink(LockFile) == NoError)
                {
                  sprintf(LogStr,"Removed stale lock %s (pid %d)",
                    LockFile,Pid);
                  LogMsg(LOG_NOTICE,LogStr);
                }
              else
                {
                  sprintf(LogStr,"Couldn't remove stale lock %s (pid %d)",
                     LockFile,Pid);
                  LogMsg(LOG_ERR,LogStr);
                  return(LockKo);
                }
            else
              {
                /* The lock file is owned by another valid process */
                sprintf(LogStr,"Lock %s is owned by pid %d",LockFile,Pid);
                LogMsg(LOG_INFO,LogStr);

                /* Lock process failed */
                return(Locked);
              }
          }
        else
          {
            /* Lock file creation problem */
            sprintf(LogStr,"Can't create lock file %s",LockFile);
            LogMsg(LOG_ERR,LogStr);

            /* Lock process failed */
            return(LockKo);
          }
      }

    /* Prepare the HDB buffer with our pid */
    sprintf(HDBBuffer,"%10d\n",(int) LockPid);

    /* Fill the lock file with the HDB buffer */
    if (write(FileDes,HDBBuffer,HDBHeaderLen) != HDBHeaderLen)
      {
        /* Lock file creation problem, remove it */
        close(FileDes);
        sprintf(LogStr,"Can't write HDB header to lock file %s",LockFile);
        LogMsg(LOG_ERR,LogStr);
        unlink(LockFile);
        
        /* Lock process failed */
        return(LockKo);
      }

    /* Closes the lock file */
    close(FileDes);
    
    /* Lock process succeded */
    return(LockOk);
#endif
  }  /**/


 /* Remove the lock file created with HDBLockFile */
 void HDBUnlockFile (char *LockFile, pid_t LockPid)
  {
#if 0
   char LogStr[TmpStrLen];

    /* Check if the lock file is still owned by us */
    if (HDBLockFile(LockFile,LockPid) == LockOk)
      {
        /* Remove the lock file */
        unlink(LockFile);
        sprintf(LogStr,"Unlocked lock file %s",LockFile);
        LogMsg(LOG_NOTICE,LogStr);
      }
#endif
  }  /**/


 /* Function executed when the program exits */
 void ExitFunction(void)
  {
#if 0
    /* Restores initial port settings */
    if (InitPortRetrieved == True)
      tcsetattr(DeviceFd, TCSANOW,&InitialPortSettings);

    /* Closes the device */
    if (DeviceOpened == True)     
      close(DeviceFd);

    /* Closes the sockets */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);

    /* Removes the lock file */
    HDBUnlockFile(LockFileName,getpid());

    /* Program termination notification */
    LogMsg(LOG_NOTICE,"SRedird stopped");
    
    /* Closes the log */
    closelog();
#endif
  }  /**/

  
/* Function called on many signals */
 void SignalFunction (int Act)
  {
    /* Same as the exit function */
    ExitFunction ();

  }  /**/


 /* Function called on break signal */
 void BreakFunction (int Act)

 {
  unsigned char LineState;
  register ListEntry *eptr;
  register Session *sesnptr;
#if 0  //REVIEW update for threads
    for (eptr=sesptr->head; eptr; eptr=eptr->next)
     {
      sesnptr=(Session *)eptr->whatever;

       if (sesnptr->mptr->BreakSignaled==True)
        {
         sesnptr->mptr->BreakSignaled=False;
         LineState=0;
        }
       else
        {
         sesnptr->mptr->BreakSignaled=True;
         LineState=16;
        }
     }
#endif
#if 0
    if (BreakSignaled==True)        
     {
      BreakSignaled=False;      
      LineState=0;
     }
    else
     {
      BreakSignaled=True;
      LineState=16;
     }
#endif

#if 0 //REVIEW update for threads
    for (eptr=sesptr->head; eptr; eptr=eptr->next)
     {
      sesnptr=(Session *)eptr->whatever;

       if ((sesnptr->mptr->LineStateMask&(unsigned char)16)!=0)
        {
         SendCPCByteCommand (sesnptr->sptr->sock, 
                             TNASC_NOTIFY_LINESTATE,
                             LineState);
        }
     }
#endif

#if 0
    /* Notify client of break change */
    if ((LineStateMask & (unsigned char) 16) != 0)
     {
      LogMsg(LOG_DEBUG,"Notifying break change");
      SendCPCByteCommand(STDOUT_FILENO,TNASC_NOTIFY_LINESTATE,LineState);
     }
#endif

  }  /**/


 /* Retrieves the port speed from DestFd */
 unsigned long int GetPortSpeed (int DestFd)

 {
  struct termios PortSettings;
  speed_t Speed;

   tcgetattr (DestFd, &PortSettings);
   Speed=cfgetospeed (&PortSettings);

   switch (Speed)
    {
     case B50:
      return(50UL);

     case B75:
      return(75UL);

     case B110:
      return(110UL);

     case B134:
      return(134UL);

     case B150:
      return(150UL);

     case B200:
      return(200UL);

     case B300:
      return(300UL);

     case B600:
      return(600UL);

     case B1200:
      return(1200UL);

     case B1800:
      return(1800UL);

     case B2400:
      return(2400UL);

     case B4800:
      return(4800UL);

     case B9600:
      return(9600UL);

     case B19200:
      return(19200UL);

     case B38400:
      return(38400UL);

     case B57600:
      return(57600UL);

     case B115200:
      return(115200UL);

     case B230400:
      return(230400UL);

     case B460800:
      return(460800UL);

     default:
      return(0UL);
    }

  }  /**/


 /* Retrieves the data size from DestFd */
 unsigned char GetPortDataSize (int DestFd)

 {
  struct termios PortSettings;
  tcflag_t DataSize;

   tcgetattr (DestFd, &PortSettings);
   DataSize=PortSettings.c_cflag&CSIZE;

   log ("*** Retrieving Port Data Size ofr %d...\n", DestFd);

   switch (DataSize)
    {
     case CS5:
      return ((unsigned char) 5);

     case CS6:
      return ((unsigned char) 6);

     case CS7:
      return ((unsigned char) 7);

     case CS8:
      return ((unsigned char) 8);

     default:
      return ((unsigned char) 0);
    }

  }  /**/


/* Retrieves the parity settings from DestFd */
 unsigned char GetPortParity (int DestFd)

 {
  struct termios PortSettings;

   tcgetattr (DestFd,&PortSettings);

   log ("*** Retrieving Port Parity for %d...\n", DestFd);

    if ((PortSettings.c_cflag&PARENB)==0)  return ((unsigned char) 1);

    if (((PortSettings.c_cflag & PARENB)!=0) &&
        ((PortSettings.c_cflag & PARODD)!=0))
     return ((unsigned char) 2);
      
   return ((unsigned char) 3);

  }  /**/


 /* Retrieves the stop bits size from DestFd */
 unsigned char GetPortStopSize (int DestFd)

 {
  struct termios PortSettings;

   tcgetattr (DestFd, &PortSettings);

   say ("*** Retrieving Port Stop size for %d...\n", DestFd);

    if ((PortSettings.c_cflag&CSTOPB)==0)
     return ((unsigned char)1);
    else
     return ((unsigned char)2);

  }  /**/

  
/* Retrieves the flow control status, including DTR and RTS status,
  from DestFd */
unsigned char GetPortFlowControl(int DestFd, unsigned char Which)
  {
    struct termios PortSettings;
    int MLines;
    Session *sesnptr;

    /* Gets the basic informations from the port */
    tcgetattr(DestFd,&PortSettings);
    ioctl(DestFd,TIOCMGET,&MLines);

    sesnptr=LocateSession (DestFd); /*NEW*/

    say ("*** Retrieving Port Flow Control for %d...\n", DestFd);

    /* Check wich kind of information is requested */
    switch (Which)
      {
        /* Com Port Flow Control Setting (outbound/both) */
        case 0:
          if ((PortSettings.c_iflag & IXON) == 0)
            return((unsigned char) 2);
          if ((PortSettings.c_cflag & CRTSCTS) == 0)
            return((unsigned char) 3);
          return((unsigned char) 0);
        break;
    
        /* BREAK State  */
        case 4:
          if (sesnptr->mptr->BreakSignaled == True)
            return((unsigned char) 5);
          else
            return((unsigned char) 6);
        break;
        
        /* DTR Signal State */
        case 7:
          if ((MLines & TIOCM_DTR) == 0)
            return((unsigned char) 9);
          else
            return((unsigned char) 8);
        break;

        /* RTS Signal State */
        case 10:
          if ((MLines & TIOCM_RTS) == 0)
            return((unsigned char) 12);
          else
            return((unsigned char) 11);
        break;

        /* Com Port Flow Control Setting (inbound) */
        case 13:
          if ((PortSettings.c_iflag & IXOFF) == 0)
            return((unsigned char) 15);
          if ((PortSettings.c_cflag & CRTSCTS) == 0)
            return((unsigned char) 16);
          return((unsigned char) 14);
        break;

        default:
          if ((PortSettings.c_iflag & IXON) == 0)
            return((unsigned char) 2);
          if ((PortSettings.c_cflag & CRTSCTS) == 0)
            return((unsigned char) 3);
          return((unsigned char) 0);
        break;
      }
  }

/* Return the status of the modem control lines (DCD, CTS, DSR, RNG) */
unsigned char GetModemState(int DevFd,unsigned char PMState)
  {
    int MLines;
    unsigned char MState = (unsigned char) 0;
    
    ioctl(DevFd,TIOCMGET,&MLines);

    //say ("*** Retrieving Modem state for %d...\n", DevFd);


    if ((MLines & TIOCM_CAR) != 0)
      MState += (unsigned char) 128;
    if ((MLines & TIOCM_RNG) != 0)
      MState += (unsigned char) 64;
    if ((MLines & TIOCM_DSR) != 0)
      MState += (unsigned char) 32;
    if ((MLines & TIOCM_CTS) != 0)
      MState += (unsigned char) 16;
    if ((MState & 128) != (PMState & 128))
      MState += (unsigned char) 8;
    if ((MState & 64) != (PMState & 64))
      MState += (unsigned char) 4;
    if ((MState & 32) != (PMState & 32))
      MState += (unsigned char) 2;
    if ((MState & 16) != (PMState & 16))
      MState += (unsigned char) 1;

    return(MState);
  }

/* Set the serial port data size */
void SetPortDataSize(int DestFd, unsigned char DataSize)
  {
    struct termios PortSettings;
    tcflag_t PDataSize;

    say ("*** Retrieving Port Data size for %d...\n", DestFd);

    switch (DataSize)
      {
        case 5:
          PDataSize = CS5;
        break;
        case 6:
          PDataSize = CS6;
        break;
        case 7:
          PDataSize = CS7;
        break;
        case 8:
          PDataSize = CS8;
        break;
        default:
          PDataSize = CS8;
        break;
      }

    tcgetattr(DestFd,&PortSettings);
    PortSettings.c_cflag = PortSettings.c_cflag &
      ((PortSettings.c_cflag & ~CSIZE) | PDataSize);
    tcsetattr(DestFd, TCSANOW, &PortSettings);
  }

  /* Set the serial port parity */
  void SetPortParity(int DestFd, unsigned char Parity)

  {
    struct termios PortSettings;

    tcgetattr(DestFd,&PortSettings);

    say ("*** Setting Port Parity for %d...\n", DestFd);

    switch (Parity)
      {
        case 1:
          PortSettings.c_cflag = PortSettings.c_cflag & ~PARENB;
        break;
        case 2:
          PortSettings.c_cflag = PortSettings.c_cflag | PARENB | PARODD;
        break;
        case 3:
          PortSettings.c_cflag = (PortSettings.c_cflag | PARENB) & ~PARODD;
        break;
        /* There's no support for MARK and SPACE parity so sets no parity */
        default:
          LogMsg(LOG_WARNING,"Requested unsupported parity. Set to no parity");
          PortSettings.c_cflag = PortSettings.c_cflag & ~PARENB;
        break;
      }

    tcsetattr (DestFd, TCSANOW, &PortSettings);

  }  /**/


/* Set the serial port stop bits size */
void SetPortStopSize(int DestFd, unsigned char StopSize)
  {
    struct termios PortSettings;

    tcgetattr(DestFd,&PortSettings);

    switch (StopSize)
      {
        case 1:
          PortSettings.c_cflag = PortSettings.c_cflag & ~CSTOPB;
        break;
        case 2:
          PortSettings.c_cflag = PortSettings.c_cflag | CSTOPB;
        break;
        case 3:
          PortSettings.c_cflag = PortSettings.c_cflag & ~CSTOPB;
          LogMsg(LOG_WARNING,"Requested unsupported 1.5 bits stop size. "
            "Set to 1 bit stop size");
        break;
        default:
          PortSettings.c_cflag = PortSettings.c_cflag & ~CSTOPB;
        break;
      }

    tcsetattr(DestFd, TCSANOW, &PortSettings);
  }


/* Set the port flow control and DTR and RTS status */
void SetPortFlowControl(int DestFd,unsigned char How)
  {
    struct termios PortSettings;
    int MLines;
    Session *sesnptr;

    /* Gets the base status from the port */
    tcgetattr(DestFd,&PortSettings);
    ioctl(DestFd,TIOCMGET,&MLines);

    sesnptr=LocateSession (DestFd);  /*NEW*/

    /* Check which settings to change */
    switch (How)
      {
        /* No Flow Control (outbound/both) */
        case 1:
          PortSettings.c_iflag = PortSettings.c_iflag & ~IXON;
          PortSettings.c_iflag = PortSettings.c_iflag & ~IXOFF;
          PortSettings.c_cflag = PortSettings.c_cflag & ~CRTSCTS;
        break;
        /* XON/XOFF Flow Control (outbound/both) */
        case 2:
          PortSettings.c_iflag = PortSettings.c_iflag | IXON;
          PortSettings.c_iflag = PortSettings.c_iflag | IXOFF;
        break;
        /* HARDWARE Flow Control (outbound/both) */
        case 3:
          PortSettings.c_cflag = PortSettings.c_cflag | CRTSCTS;
        break;
        /* BREAK State ON */
        case 5:
          tcsendbreak(DestFd,0);
          sesnptr->mptr->BreakSignaled = True;
        break;
        /* BREAK State OFF */
        case 6:
          tcsendbreak(DestFd,0);
          sesnptr->mptr->BreakSignaled = False;
        break;
        /* DTR Signal State ON */
        case 8:
          MLines = MLines | TIOCM_DTR;
        break;
        /* DTR Signal State OFF */
        case 9:
          MLines = MLines & ~TIOCM_DTR;
        break;
        /* RTS Signal State ON */
        case 11:
          MLines = MLines | TIOCM_RTS;
        break;
        /* RTS Signal State OFF */
        case 12:
          MLines = MLines & ~TIOCM_RTS;
        break;
        /* No Flow Control (inbound) */
        case 14:
          PortSettings.c_iflag = PortSettings.c_iflag & ~IXOFF;
        break;
        /* XON/XOFF Flow Control (inbound) */
        case 15:
          PortSettings.c_iflag = PortSettings.c_iflag | IXOFF;
        break;
        /* HARDWARE Flow Control (inbound) */
        case 16:
          PortSettings.c_cflag = PortSettings.c_cflag | CRTSCTS;
        break;
        default:
          LogMsg(LOG_WARNING,"Requested unsupported flow control, "
            "setting to no flow control");
          PortSettings.c_iflag = PortSettings.c_iflag & ~IXON;
          PortSettings.c_iflag = PortSettings.c_iflag & ~IXOFF;
          PortSettings.c_cflag = PortSettings.c_cflag & ~CRTSCTS;
        break;
      }
    
    tcsetattr (DestFd, TCSANOW, &PortSettings);
    ioctl (DestFd,TIOCMSET,&MLines);

  }  /**/


 /* Set the serial port speed */ 
 void SetPortSpeed (int DestFd, unsigned long BaudRate)
 {
  struct termios PortSettings;
  speed_t Speed;

    say ("*** Setting port speed for %d...\n", DestFd);

    switch (BaudRate)
      {
        case 50UL:
          Speed = B50;
        break;
        case 75UL:
          Speed = B75;
        break;
        case 110UL:
          Speed = B110;
        break;
        case 134UL:
          Speed = B134;
        break;
        case 150UL:
          Speed = B150;
        break;
        case 200UL:
          Speed = B200;
        break;
        case 300UL:
          Speed = B300;
        break;
        case 600UL:
          Speed = B600;
        break;
        case 1200UL:
          Speed = B1200;
        break;
        case 1800UL:
          Speed = B1800;
        break;
        case 2400UL:
          Speed = B2400;
        break;
        case 4800UL:
          Speed = B4800;
        break;
        case 9600UL:
          Speed = B9600;
        break;
        case 19200UL:
          Speed = B19200;
        break;
        case 38400UL:
          Speed = B38400;
        break;
        case 57600UL:
          Speed = B57600;
        break;
        case 115200UL:
          Speed = B115200;
        break;
        case 230400UL:
          Speed = B230400;
        break;
        case 460800UL:
          Speed = B460800;
        break;
        default:
          LogMsg(LOG_WARNING,"Unknwon baud rate requested. Setting to 38400");
          Speed = B38400;
        break;
      }
      
    tcgetattr(DestFd,&PortSettings);
    cfsetospeed(&PortSettings,Speed);
    cfsetispeed (&PortSettings, Speed);
    tcsetattr (DestFd, TCSANOW, &PortSettings);

  }  /**/
 

 /* BUG--DestFd passed is modem's! should be client's socket!--ayman 21/7/99 */ 
 /* Send the signature Sig to the client */
 void SendSignature (Session *sesnptr, char *Sig)
 {
  unsigned char C;

   C=TNIAC;
   write (sesnptr->sptr->sock, &C, 1);

   C=TNSB;
   write (sesnptr->sptr->sock, &C, 1);
   C=TNCOM_PORT_OPTION;
   write (sesnptr->sptr->sock, &C, 1);
   C=TNASC_SIGNATURE;
   write (sesnptr->sptr->sock, &C, 1);

   SendStr (sesnptr->sptr->sock, Sig);

   C=TNIAC;
   write (sesnptr->sptr->sock, &C, 1);
   C=TNSE;
   write (sesnptr->sptr->sock, &C, 1);

 }  /**/


 /* Write a char to DestFd performing IAC escaping */
 void EscWriteChar (int DestFd, unsigned char C)

 {
    if (C==TNIAC)
     {
      write (DestFd, &C, 1);
      write (DestFd, &C, 1);
     }
    else
     {
      write (DestFd, &C, 1);
     }

 }  /**/


 /* Redirect char C to DestFd checking for IAC escape sequences */
 void EscRedirectChar (int DestFd, unsigned char C)

 { 
  Modem *mptr=LocateDevice (DestFd);  
  Session *sesnptr;


   
    /* Check the IAC escape status */
    switch (mptr->IACEscape)
      {
        /* Normal status */
        case IACNormal:
         log ("In EscRedirChar, switch (mptr->IACEscape)==case IACNormal"); 
          if (C == TNIAC)
           { 
            mptr->IACEscape = IACReceived;
            log ("In EscRedirChar, C== TNIAC, setting mptr->IACEscape = IACReceived"); 
           }
          else
           {
            write(DestFd,&C,1); 
            log ("In EscRedirChar, C!=TNIAC, Wrting %c to fd %d", C, DestFd);
           }
        break;

        /* IAC previously received */
        case IACReceived:
          if (C == TNIAC)
            {
              write(DestFd,&C,1); /* escaped IAC */
              mptr->IACEscape = IACNormal;
            }
          else
            {
              mptr->IACCommand[0] = TNIAC;
              mptr->IACCommand[1] = C;
              mptr->IACPos = 2;
              mptr->IACEscape = IACComReceiving;
              mptr->IACSigEscape = IACNormal;
            }
        break;

        /* IAC Command reception */
        case IACComReceiving:
          /* Telnet suboption, could be only CPC */
          if (mptr->IACCommand[1] == TNSB)
            {
              /* Get the suboption signature */
              if (mptr->IACPos < 4)
                {
                  mptr->IACCommand[mptr->IACPos] = C;
                  mptr->IACPos++;
                }
              else
                {
                  /* Check which suboption we are dealing with */
                  switch (mptr->IACCommand[3])
                    {
                      /* Signature, which needs further escaping */
                      case TNCAS_SIGNATURE:
                        switch (mptr->IACSigEscape)
                          {
                            case IACNormal:
                              if (C == TNIAC)
                                mptr->IACSigEscape = IACReceived;
                              else
                                {
                                  if (mptr->IACPos < TmpStrLen)
                                    {
                                      mptr->IACCommand[mptr->IACPos] = C;
                                      mptr->IACPos++;
                                    }
                                }
                            break;

                            case IACComReceiving:
                              mptr->IACSigEscape = IACNormal;
                            break;
    
                            case IACReceived:
                              if (C == TNIAC)
                                {
                                  if (mptr->IACPos < TmpStrLen)
                                    {
                                      mptr->IACCommand[mptr->IACPos] = C;
                                      mptr->IACPos++;
                                    }
                                  mptr->IACSigEscape = IACNormal;
                                }
                              else
                                {
                                  if (mptr->IACPos < TmpStrLen)
                                    {
                                      mptr->IACCommand[mptr->IACPos] = TNIAC;
                                      mptr->IACPos++;
                                    }
    
                                  if (mptr->IACPos < TmpStrLen)
                                    {
                                      mptr->IACCommand[mptr->IACPos] = C;
                                      mptr->IACPos++;
                                    }
    
                                  HandleIACCommand(DestFd,mptr->IACCommand,mptr->IACPos);
                                  mptr->IACEscape = IACNormal;
                                }
                            break;
                          }
                      break;
    
                      /* Set baudrate */
                      case TNCAS_SET_BAUDRATE:
                        mptr->IACCommand[mptr->IACPos] = C;
                        mptr->IACPos++;
    
                        if (mptr->IACPos == 10)
                          {
                            HandleIACCommand(DestFd,mptr->IACCommand,mptr->IACPos);
                            mptr->IACEscape = IACNormal;
                          }
                      break;
                      
                      /* Flow control command */
                      case TNCAS_FLOWCONTROL_SUSPEND:
                      case TNCAS_FLOWCONTROL_RESUME:
                        mptr->IACCommand[mptr->IACPos] = C;
                        mptr->IACPos++;
    
                        if (mptr->IACPos == 6)
                          {
                            HandleIACCommand(DestFd,mptr->IACCommand,mptr->IACPos);
                            mptr->IACEscape = IACNormal;
                          }
                      break;
                      
                      /* Normal CPC command with single byte parameter */
                      default:
                        mptr->IACCommand[mptr->IACPos] = C;
                        mptr->IACPos++;
    
                        if (mptr->IACPos == 7)
                          {
                            HandleIACCommand(DestFd,mptr->IACCommand,mptr->IACPos);
                            mptr->IACEscape = IACNormal;
                          }
                      break;
                    }                                     
                }
          }
        else
          {
            /* Normal 3 byte IAC option */
            mptr->IACCommand[mptr->IACPos] = C;
            mptr->IACPos++;

            if (mptr->IACPos == 3)
              {
                HandleIACCommand(DestFd,mptr->IACCommand,mptr->IACPos);
                mptr->IACEscape = IACNormal;
              }
          }
        break;
      }

  }  /**/


 /* Send the specific telnet option to DestFd using Command as command */
 /* BUG-DestFd should be the client's socket--not modem's! ayman */
 void SendTelnetOption (Session *sesnptr, unsigned char Command, char Option)

 {
  unsigned char IAC=TNIAC;
    
   write (sesnptr->sptr->sock, &IAC, 1);
   write (sesnptr->sptr->sock, &Command, 1);
   write (sesnptr->sptr->sock, &Option, 1);

 }  /**/


 /* Send a string to DevFd performing IAC escaping */
 void SendStr (int DestFd, char *Str)

 {
  size_t I;
  size_t L;

   L=strlen (Str);

    for (I=0; I<L; I++)
     {
      EscWriteChar (DestFd, (unsigned char) Str[I]);
     }

 }  /**/


/* Send the baud rate BR to DestFd */
/* BUG-DestFd should be the client's socket--not modem's! ayman */
 void SendBaudRate (Session *sesnptr, unsigned long int BR)
 
 {
  unsigned char C;
  unsigned long int NBR;

    if (!sesnptr)  return;

   NBR=htonl (BR);

   C=TNIAC;
   write (sesnptr->sptr->sock, &C, 1);
   C=TNSB;
   write (sesnptr->sptr->sock, &C, 1);
   C=TNCOM_PORT_OPTION;
   write (sesnptr->sptr->sock, &C, 1);
   C=TNASC_SET_BAUDRATE;
   write (sesnptr->sptr->sock, &C, 1);
   write (sesnptr->sptr->sock, &NBR, 4);
   C=TNIAC;
   write (sesnptr->sptr->sock, &C, 1);
   C=TNSE;
   write (sesnptr->sptr->sock, &C, 1);

 }  /**/


 /* Send the flow control command Command */
 /* BUG-DestFd should be the client's socket--not modem's! ayman */
 void SendCPCFlowCommand (int DestFd, unsigned char Command)

  {
   unsigned char C;

    C = TNIAC;
    write(DestFd,&C,1);
    C = TNSB;
    write(DestFd,&C,1);
    C = TNCOM_PORT_OPTION;
    write(DestFd,&C,1);
    write(DestFd,&Command,1);
    C = TNIAC;
    write(DestFd,&C,1);
    C = TNSE;
    write(DestFd,&C,1);

    if (Command == TNASC_FLOWCONTROL_SUSPEND)
      LogMsg(LOG_DEBUG,"Sent flow control suspend command");
    else
      LogMsg(LOG_DEBUG,"Sent flow control resume command");
    
  }  /**/

  
/* Send the CPC command Command using Parm as parameter */
/* BUG-DestFd should be the client's socket--not modem's! ayman */
 void 
SendCPCByteCommand (Session *sesnptr, unsigned char Command, unsigned char Parm)

 {
  unsigned char C;

   C=TNIAC, write (sesnptr->sptr->sock, &C, 1);
   C=TNSB, write (sesnptr->sptr->sock, &C, 1);
   C=TNCOM_PORT_OPTION, write (sesnptr->sptr->sock, &C, 1);
   write (sesnptr->sptr->sock, &Command, 1);
   write (sesnptr->sptr->sock, &Parm, 1);
   C=TNIAC, write (sesnptr->sptr->sock, &C, 1);
   C=TNSE, write (sesnptr->sptr->sock, &C, 1);

  }  /**/


 /* Handling of COM Port Control specific commands */
 void HandleCPCCommand (Session *sesnptr, unsigned char * Command, size_t CSize)

 {
  char LogStr[TmpStrLen];
  char SigStr[TmpStrLen];
  unsigned long int BaudRate;
  unsigned char DataSize;
  unsigned char Parity;
  unsigned char StopSize;
  unsigned char FlowControl;
 
    /* Check wich command has been requested */
    switch (Command[3])
      {
        /* Signature */
        case TNCAS_SIGNATURE:
          if (CSize == 6)
            {
             char *s,
                  *t;

              s="msredir";

               if (!(t=valueof("SIGNATURE_STR")))  t="-";
                
              /* Void signature, client is asking for our signature */
              /* potential overflow--sanity checks please... */
              sprintf(SigStr,"%s %s %s", s, t, sesnptr->mptr->DeviceName);
              /* BUG-DestFd should be the client's socket! */
              SendSignature (sesnptr, SigStr);
              sprintf(LogStr,"Sent signature: %s",SigStr);
              LogMsg(LOG_INFO,LogStr);
            }
          else
            {
              /* Received client signature */
              /* should be saved somewhere--ayman */
              strncpy (SigStr, &Command[4], CSize-6);
              sprintf (LogStr, "Received client signature: %s", SigStr);
              /* save it somewhere... */
              LogMsg (LOG_INFO, LogStr);
            }
        break;

        /* Set serial baud rate */
        case TNCAS_SET_BAUDRATE:
          /* Retrieve the baud rate which is in network order */
          BaudRate = ntohl(*((unsigned long int *) &Command[4]));

          if (BaudRate == 0)
            {
              /* Client is asking for current baud rate */
             
             log ("- Request for port speed for %s@%d", 
                  sesnptr->sptr->haddress, sesnptr->mptr->devfd);
            }
          else
            {
              /* Change the baud rate */
             log ("- Request for change port speed for %s@%d new value: %lu", 
                  sesnptr->sptr->haddress, sesnptr->mptr->devfd, BaudRate);

             SetPortSpeed (sesnptr->mptr->devfd, BaudRate);          
            }

          /* Send confirmation */
          BaudRate = GetPortSpeed (sesnptr->mptr->devfd);
          /*BUG-DestFd sould be socket ayman*/
          SendBaudRate (sesnptr, BaudRate);
          log ("- Send port speed value for %s@%d -> %lu", 
               sesnptr->sptr->haddress, sesnptr->mptr->devfd, BaudRate);
        break;
        
        /* Set serial data size */
        case TNCAS_SET_DATASIZE:
          if (Command[4] == 0)
            {
              /* Client is asking for current data size */
              LogMsg(LOG_DEBUG,"Data size notification requested");             
            }
          else
            {
              /* Set the data size */
              sprintf(LogStr,"Port data size change to %u requested",
                (unsigned int) Command[4]);
              LogMsg(LOG_DEBUG,LogStr);
              SetPortDataSize(sesnptr->mptr->devfd, Command[4]);             
            }

          /* Send confirmation */
          DataSize = GetPortDataSize(sesnptr->mptr->devfd);
          SendCPCByteCommand (sesnptr, TNASC_SET_DATASIZE, DataSize);
          sprintf(LogStr,"Port data size: %u",(unsigned int) DataSize);
          LogMsg(LOG_DEBUG,LogStr);
        break;

        /* Set the serial parity */
        case TNCAS_SET_PARITY:
          if (Command[4] == 0)
            {
              /* Client is asking for current parity */
              LogMsg(LOG_DEBUG,"Parity notification requested");            
            }
          else
            {
              /* Set the parity */
              sprintf(LogStr,"Port parity change to %u requested",
                (unsigned int) Command[4]);
              LogMsg(LOG_DEBUG,LogStr);
              SetPortParity (sesnptr->mptr->devfd, Command[4]);             
            }

          /* Send confirmation */
          Parity = GetPortParity (sesnptr->mptr->devfd);
          SendCPCByteCommand (sesnptr, TNASC_SET_PARITY, Parity);
          sprintf(LogStr,"Port parity: %u",(unsigned int) Parity);
          LogMsg(LOG_DEBUG,LogStr);
        break;
        
        /* Set the serial stop size */
        case TNCAS_SET_STOPSIZE:
          if (Command[4] == 0)
            {
              /* Client is asking for current stop size */
              LogMsg(LOG_DEBUG,"Stop size notification requested");             
            }
          else
            {
              /* Set the stop size */
              sprintf(LogStr,"Port stop size change to %u requested",
                (unsigned int) Command[4]);
              LogMsg(LOG_DEBUG,LogStr);
              SetPortStopSize (sesnptr->mptr->devfd, Command[4]);
            }

          /* Send confirmation */
          StopSize = GetPortStopSize (sesnptr->mptr->devfd);
          SendCPCByteCommand (sesnptr, TNASC_SET_STOPSIZE, StopSize);
          sprintf(LogStr,"Port stop size: %u",(unsigned int) StopSize);
          LogMsg(LOG_DEBUG,LogStr);
        break;

        /* Flow control and DTR/RTS handling */
        case TNCAS_SET_CONTROL:
          if (Command[4] == 0 || Command[4] == 4 || Command[4] == 7 ||
            Command[4] == 10 || Command[4] == 13)
            {
              /* Client is asking for current flow control or DTR/RTS status */
              LogMsg(LOG_DEBUG,"Flow control notification requested");
              FlowControl = GetPortFlowControl (sesnptr->mptr->devfd, Command[4]);
              SendCPCByteCommand (sesnptr, TNASC_SET_CONTROL, FlowControl);
              sprintf(LogStr,"Port flow control: %u",
                (unsigned int) FlowControl);
              LogMsg(LOG_DEBUG,LogStr);
            }
          else
            {
              /* Set the flow control */
              sprintf(LogStr,"Port flow control change to %u requested",
                (unsigned int) Command[4]);
              LogMsg(LOG_DEBUG,LogStr);
              SetPortFlowControl (sesnptr->mptr->devfd, Command[4]);

              /* Flow control status confirmation */
              if (Command[4] <= 16)
                FlowControl = Command[4];
              else
                FlowControl=GetPortFlowControl (sesnptr->mptr->devfd, 0);
              SendCPCByteCommand (sesnptr, TNASC_SET_CONTROL, FlowControl);
              sprintf(LogStr,"Port flow control: %u",
                (unsigned int) FlowControl);
              LogMsg(LOG_DEBUG,LogStr);
            }
        break;
        
        /* Set the line state mask */
        case TNCAS_SET_LINESTATE_MASK:          
          sprintf(LogStr,"Line state set to %u",(unsigned int) Command[4]);
          LogMsg(LOG_DEBUG,LogStr);
          /* Only break notification supported */
          LineStateMask = Command[4] & (unsigned char) 16;
          SendCPCByteCommand (sesnptr, TNASC_SET_LINESTATE_MASK,
                              (unsigned char)LineStateMask);
        break;

        /* Set the modem state mask */
        case TNCAS_SET_MODEMSTATE_MASK:
          sprintf(LogStr,"Modem state mask set to %u",
            (unsigned int) Command[4]);
          LogMsg(LOG_DEBUG,LogStr);
          sesnptr->mptr->ModemStateMask = Command[4];
          SendCPCByteCommand (sesnptr, TNASC_SET_MODEMSTATE_MASK,
                              sesnptr->mptr->ModemStateMask);
        break;

        /* Port flush requested */
        case TNCAS_PURGE_DATA:
          sprintf(LogStr,"Port flush %u requested",(unsigned int) Command[4]);
          LogMsg(LOG_DEBUG,LogStr);
          switch (Command[4])
            {
              /* Inbound flush */
              case 1:
                tcflush (sesnptr->mptr->devfd, TCIFLUSH);
              break;
              /* Outbound flush */
              case 2:
                tcflush (sesnptr->mptr->devfd, TCOFLUSH);
              break;
              /* Inbound/outbound flush */
              case 3:
                tcflush (sesnptr->mptr->devfd, TCIOFLUSH);
              break;
            }

          SendCPCByteCommand (sesnptr, TNASC_PURGE_DATA, Command[4]);
        break;

        /* Suspend output to the client */
        case TNCAS_FLOWCONTROL_SUSPEND:
          LogMsg(LOG_DEBUG,"Flow control suspend requested");
          sesnptr->mptr->InputFlow=False;
        break;
        
        /* Resume output to the client */
        case TNCAS_FLOWCONTROL_RESUME:
          LogMsg(LOG_DEBUG,"Flow control resume requested");
          sesnptr->mptr->InputFlow = True;
        break;

        /* Unknown request */
        default:
          sprintf(LogStr,"Unhandled request %u",(unsigned int) Command[3]);
          LogMsg(LOG_DEBUG,LogStr);
        break;
      }

  }  /**/


 /* Common telnet IAC commands handling */
 void HandleIACCommand (int DestFd, unsigned char *Command, size_t CSize)

 {
  char LogStr[TmpStrLen];
  Modem *mptr=LocateDevice (DestFd); 
  Session *sesnptr;


     if (!(sesnptr=LocateSession(DestFd)))  /*NEW*/
     {
      log ("ERROR-In HandleIACCommand(), no sesnptr for DestFd:%d!\n", DestFd);

      return;
     }

    /* Check which command */
    switch(Command[1])
      {
        /* Suboptions */
        case TNSB:
          switch (Command[2])
            {
              /* RFC 2217 COM Port Control Protocol option */
              case TNCOM_PORT_OPTION:
                HandleCPCCommand (sesnptr, Command, CSize);
              break;
              
              default:
                logf(sesnptr, "Unknown suboption received: %u",
                              (unsigned int) Command[2]);

              break;
            }
        break;

        /* Requests for options */
        case TNWILL:
          switch (Command[2])
            {
              /* COM Port Control Option */
              case TNCOM_PORT_OPTION:
               mptr->TCPCEnabled=True;
               SendTelnetOption (sesnptr, TNDO, TNCOM_PORT_OPTION);
               logf(sesnptr, "Telnet COM Port Control Enabled (WILL)");

               break;

              /* Telnet Binary mode */
              case TN_TRANSMIT_BINARY:
               LogMsg(LOG_INFO,"Telnet Binary Transfer Enabled (WILL)");
               SendTelnetOption (sesnptr, TNDO, TN_TRANSMIT_BINARY);
               
               break;

              /* Echo request not handled */
              case TN_ECHO:
               SendTelnetOption (sesnptr, TNDONT, TN_TRANSMIT_BINARY);
               logf(sesnptr, "Rejecting Telnet Echo Option (WILL)");

               break;

              /* No go ahead needed */
              case TN_SUPPRESS_GO_AHEAD:
               SendTelnetOption (sesnptr, TNDO, TN_TRANSMIT_BINARY);
               logf(sesnptr, "Suppressing Go Ahead characters (WILL)");

               break;

              /* Reject everything else */
              default:
               SendTelnetOption (sesnptr, TNDONT, Command[2]);
               logf(sesnptr, "Rejecting option WILL: %u",
                              (unsigned int) Command[2]);
               break;
            }
        break;

        /* Confirmations for options */
        case TNDO:
          switch (Command[2])
            {
              /* COM Port Control Option */
              case TNCOM_PORT_OPTION:
               mptr->TCPCEnabled=True;
               logf(sesnptr, "Telnet COM Port Control Enabled (DO)");
                /* TNWILL already sent on startup */

               break;

              /* Telnet Binary mode */
              case TN_TRANSMIT_BINARY:
               SendTelnetOption (sesnptr, TNWILL, TN_TRANSMIT_BINARY);
               logf(sesnptr, "Telnet Binary Transfer Enabled (DO)");
              
               break;

              /* Echo reques1t not handled */
              case TN_ECHO:
               SendTelnetOption (sesnptr, TNWONT, TN_TRANSMIT_BINARY);
               logf(sesnptr, "Rejecting Telnet Echo Option (DO)");

               break;

              /* No go ahead needed */
              case TN_SUPPRESS_GO_AHEAD:
               SendTelnetOption (sesnptr, TNWILL, TN_TRANSMIT_BINARY);
               logf(sesnptr, "Suppressing Go Ahead characters (DO)");

               break;

              /* Reject everything else */
              default:
               SendTelnetOption (sesnptr, TNWONT, Command[2]);
               logf(sesnptr, "Rejecting option DO: %u",
                             (unsigned int) Command[2]);

               break;
            }
        break;
        
        /* Notifications of rejections for options */
        case TNWONT:
        case TNDONT:
          if (Command[2] == TNCOM_PORT_OPTION)
            {
             //"Protocol Option (RFC 2217) exiting");
             logf(sesnptr, "Client doesn't support Telnet COM Port\n");
             //maybe close it?
             //exit(1);
            }
          else
            {
             logf(sesnptr, "Received rejection for option: %u",
                           (unsigned int) Command[2]);
            }
        break;
      }

  }  /**/


 /* Write a buffer to DestFd with IAC escaping */
 void 
 EscWriteBuffer (int DestFd, unsigned char *Buffer, unsigned int BSize, int y)

  {
   unsigned int I;
   unsigned char TmpBuf[2*OutBufferSize];
   unsigned int BPos=0;

     if (BSize>0)
      {
       log ("-In EscWriteBuffer(): sending Outbut buffer to sock %d (%s)\n", DestFd, y==1?"Output Buffer timeout":"Polling timeout");
       log ("Contents of outbuf: ``%s''", Buffer);

        for (I=0; I<BSize; I++)
          if (Buffer[I]==TNIAC)
           {
             TmpBuf[BPos]=TNIAC;
             TmpBuf[BPos+1]=TNIAC;
             BPos+=2;
           }
          else
           {
             TmpBuf[BPos]=Buffer[I];
             BPos++;
           }
   
        I=write(DestFd, TmpBuf, BPos);
      }

  }  /**/


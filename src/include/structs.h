/*
**
MODULEID("$Id: structs.h,v 1.1 1999/07/26 01:46:59 ayman Exp $")
**
*/
#if 0
/* Locking constants */
#define LockOk 0
#define Locked 1
#define LockKo 2

/* Error conditions constants */
#define NoError 0
#define Error 1
#define OpenError -1

/* Maximum length of temporary strings */
#define TmpStrLen 255

/* Buffer size */
#define BufferSize 512

/* Output buffer size */
#define OutBufferSize 512

/* File mode and file length for HDB (ASCII) stile lock file */
#define LockFileMode 0644
#define HDBHeaderLen 11

#endif

/* Base Telnet protocol constants (STD 8) */
#define TNSE ((unsigned char) 240)
#define TNNOP ((unsigned char) 241)
#define TNSB ((unsigned char) 250)
#define TNWILL ((unsigned char) 251)
#define TNWONT ((unsigned char) 252)
#define TNDO ((unsigned char) 253)
#define TNDONT ((unsigned char) 254)
#define TNIAC ((unsigned char) 255)

/* Base Telnet protocol options constants (STD 27, STD 28, STD 29) */
#define TN_TRANSMIT_BINARY ((unsigned char) 0)
#define TN_ECHO ((unsigned char) 1)
#define TN_SUPPRESS_GO_AHEAD ((unsigned char) 3)

/* Base Telnet Com Port Control (CPC) protocol constants (RFC 2217) */
#define TNCOM_PORT_OPTION ((unsigned char) 44)

/* CPC Client to Access Server constants */
#define TNCAS_SIGNATURE ((unsigned char) 0)
#define TNCAS_SET_BAUDRATE ((unsigned char) 1)
#define TNCAS_SET_DATASIZE ((unsigned char) 2)
#define TNCAS_SET_PARITY ((unsigned char) 3)
#define TNCAS_SET_STOPSIZE ((unsigned char) 4)
#define TNCAS_SET_CONTROL ((unsigned char) 5)
#define TNCAS_NOTIFY_LINESTATE ((unsigned char) 6)
#define TNCAS_NOTIFY_MODEMSTATE ((unsigned char) 7)
#define TNCAS_FLOWCONTROL_SUSPEND ((unsigned char) 8)
#define TNCAS_FLOWCONTROL_RESUME ((unsigned char) 9)
#define TNCAS_SET_LINESTATE_MASK ((unsigned char) 10)
#define TNCAS_SET_MODEMSTATE_MASK ((unsigned char) 11)
#define TNCAS_PURGE_DATA ((unsigned char) 12)

/* CPC Access Server to Client constants */
#define TNASC_SIGNATURE ((unsigned char) 100)
#define TNASC_SET_BAUDRATE ((unsigned char) 101)
#define TNASC_SET_DATASIZE ((unsigned char) 102)
#define TNASC_SET_PARITY ((unsigned char) 103)
#define TNASC_SET_STOPSIZE ((unsigned char) 104)
#define TNASC_SET_CONTROL ((unsigned char) 105)
#define TNASC_NOTIFY_LINESTATE ((unsigned char) 106)
#define TNASC_NOTIFY_MODEMSTATE ((unsigned char) 107)
#define TNASC_FLOWCONTROL_SUSPEND ((unsigned char) 108)
#define TNASC_FLOWCONTROL_RESUME ((unsigned char) 109)
#define TNASC_SET_LINESTATE_MASK ((unsigned char) 110)
#define TNASC_SET_MODEMSTATE_MASK ((unsigned char) 111)
#define TNASC_PURGE_DATA ((unsigned char) 112)
/* Modem state effective change mask */
#define ModemStateECMask ((unsigned char) 240)

/* Default modem state polling in milliseconds (100 msec should be enough) */
#define ModemStatePolling 100
#if 0
/* Status enumeration for IAC escaping and interpretation */
typedef enum { IACNormal, IACReceived, IACComReceiving } IACState;


/* Standard boolean definition */
typedef enum { False, True } Boolean;


/* Buffer structure */
  struct BufferType {
          unsigned char Buffer[BufferSize];
          unsigned int RdPos;
          unsigned int WrPos;
         };
 typedef struct BufferType BufferType;

struct ModemDevice {
       BufferType buftyp; /* struct */

       unsigned char C;
       /* buffers business */
       /* Input buffer */
       BufferType InBuffer;

       /* Output buffer */
       unsigned char OutBuffer[OutBufferSize];

       /* Output buffer write position */
       unsigned int OutBPos;/*=0;*/


       char *LockFileName,
            *devlockname;
       char *DeviceName,
            *devname;

       int DeviceFd,
           devfd;

       Boolean DeviceOpened;  /*default=False*/
       Boolean TCPCEnabled;  /*default=False*/
       Boolean InitPortRetrieved;/*default=false true after settings retrieved*/  
       struct termios InitialPortSettings; /*initial serial port settings*/
       struct termios PortSettings; /* actual port settings */

       /* Base timeout for stream reading */
       struct timeval BTimeout;

       /* Timeout for stream reading */
       struct timeval RTimeout;

       /* Pointer to timeout structure to set */
       struct timeval *ETimeout;/* = &RTimeout;*/
 
       /* Out buffer ticks count */
       clock_t OutBTicks; /* = times(NULL);*/

       /* Out buffer clock ticks limit */
       clock_t MaxBTicks;
 
       IACState IACEscape;
       IACState IACSigEscape;

       unsigned char IACCommand[TmpStrLen];
       size_t IACPos; /* Position of insertion into IACCommand[] */

       unsigned char ModemStateMask;/* Modem state mask set by the client =255*/       unsigned char ModemState; /*current stst of modem control lines */
       unsigned char LineStateMask; /* Line state mask set by the client */

       Boolean BreakSignaled; /*def false*/
       Boolean InputFlow; /*def true*/

       Boolean RemoteFlowOff;
       /* Socket setup flag */
       int SockParmEnable; /* = 1 */

       /* Generic socket parameter */
       int SockParm;

       unsigned tmpstat,
                inuse;
       unsigned stat,  /* FIRING_SQUAD */
                flags; /* RESERVED...*/ 
       Socket *sptr;
      };
 typedef struct ModemDevice Modem;

 typedef struct LogFile {
          FILE *file;
          char *fname;
         } LogFile;

 /* dynamically built linkedlist of connected socket-device pairs */
 typedef struct Session {
          time_t when;
          unsigned stat;  /* FIRING_DQUAD */
          LogFile logf;

          Modem *mptr;
          Socket *sptr;
         } Session;

#endif
 int SocketClose (Socket *, const int);
 int TELNET_Readme (Socket *sptr);
 

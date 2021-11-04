
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

 /* Status enumeration for IAC escaping and interpretation */
 typedef enum { IACNormal, IACReceived, IACComReceiving } IACState;

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


 void BuildDeviceRegistry (void);
 void VerifyDeviceRegistry (void);
 void DiscardUnverifiedDevices (void);
 Modem *OpenDevice (void);
 int HowManyInDeviceReg (void);
 Modem *GetTaggedDeviceRegEntry (unsigned);
 Modem *GetFlaggedDevice (unsigned);
 Modem *LocateFreeDevice (void);
 Modem *LocateDevice (int);
 int CloseDevice (int);


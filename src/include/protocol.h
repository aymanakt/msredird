
/* Initialize a buffer for operation */
void InitBuffer (BufferType *);

/* Check if the buffer is empty */
Boolean IsBufferEmpty (BufferType *);

/* Check if the buffer is full */
Boolean IsBufferFull (BufferType *);

/* Add a byte to a buffer */
void AddToBuffer (BufferType *, unsigned char);

/* Get a byte from a buffer */
unsigned char GetFromBuffer (BufferType *);

/* Generic log function with log level control. Uses the same log levels
of the syslog(3) system call */
void LogMsg (int, char *);

/* Try to lock the file given in LockFile as pid LockPid using the classical
  HDB (ASCII) file locking scheme */
int HDBLockFile(char *, pid_t);

/* Remove the lock file created with HDBLockFile */
void HDBUnlockFile (char *, pid_t);

/* Function executed when the program exits */
void ExitFunction (void);

/* Function called on many signals */
void SignalFunction (int);
  
/* Function called on break signal */
void BreakFunction (int);

/* Retrieves the port speed from DestFd */
unsigned long int GetPortSpeed (int);

/* Retrieves the data size from DestFd */
unsigned char GetPortDataSize (int);

/* Retrieves the parity settings from DestFd */
unsigned char GetPortParity (int);

/* Retrieves the stop bits size from DestFd */
unsigned char GetPortStopSize (int);
  
/* Retrieves the flow control status, including DTR and RTS status,
from DestFd */
unsigned char GetPortFlowControl (int, unsigned char);

/* Return the status of the modem control lines (DCD, CTS, DSR, RNG) */
unsigned char GetModemState (int, unsigned char);

/* Set the serial port data size */
void SetPortDataSize (int, unsigned char);

/* Set the serial port parity */
void SetPortParity (int, unsigned char);

/* Set the serial port stop bits size */
void SetPortStopSize (int, unsigned char);

/* Set the port flow control and DTR and RTS status */
void SetPortFlowControl (int,unsigned char);

/* Set the serial port speed */ 
void SetPortSpeed (int, unsigned long);

/* Send the signature Sig to the client */
void SendSignature (Session *, char *);

/* Write a char to DestFd performing IAC escaping */
void EscWriteChar (int, unsigned char);

/* Redirect char C to DestFd checking for IAC escape sequences */
void EscRedirectChar (int, unsigned char);

/* Send the specific telnet option to DestFd using Command as command */
void SendTelnetOption (Session *, unsigned char, char);
  
/* Send a string to DevFd performing IAC escaping */
void SendStr (int, char *);

/* Send the baud rate BR to DestFd */
void SendBaudRate (/*int*/Session *, unsigned long int);

/* Send the flow control command Command */
void SendCPCFlowCommand (int, unsigned char);

/* Send the CPC command Command using Parm as parameter */
void SendCPCByteCommand (Session *, unsigned char, unsigned char);

/* Handling of COM Port Control specific commands */
void HandleCPCCommand (Session *, unsigned char *, size_t);

/* Common telnet IAC commands handling */
void HandleIACCommand (int, unsigned char *, size_t);

/* Write a buffer to DestFd with IAC escaping */
void EscWriteBuffer (int, unsigned char *, unsigned int, int);



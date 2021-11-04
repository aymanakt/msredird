
 typedef struct Session {
          time_t when;
          unsigned stat;  /* FIRING_DQUAD */

          LogFile logf;

          Modem *mptr;
          Socket *sptr;

          pthread_t th;
          pid_t pid;
         } Session;


 Session *OpenSession (Modem *, Socket *);
 Session *GetFlaggedSession (unsigned);
 int OpenSessionLogFile (Session *);
 void CloseSessionLogFile (Session *);
 int CloseSession (Session *);
 int KillSession (Session *);
 Session *LocateSession (int);
 void PrintSessions (void);


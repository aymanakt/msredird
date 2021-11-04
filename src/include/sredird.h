/*
**
**
MODULEID("$Id: sredird.h,v 1.1 1999/09/01 00:42:00 ayman Exp $")
**
*/


 typedef struct msredird {
          time_t when;  /* up-time... */
          unsigned discard; /* if true, cleanup registry */
          unsigned inuse;   /* how many devices currently in use */

          LogFile logf;  /* session wide logging... */

          Socket *telnetsock; /* the listening TCP socket */

          int nready;
          pthread_mutex_t nready_mutex;
          pthread_cond_t nready_cond;
         } msredird;


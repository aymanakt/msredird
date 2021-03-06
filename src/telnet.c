/*
** telnet.c Copyright (c) 1998,99 Ayman Akt
**
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <main.h>

#include <termio.h>    /* struct termios */
#include <sys/time.h>  /* struct timeval */

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <list.h>
#include <sockets.h>
#include <socketio.h>
#include <telnet.h>
#include <prefs.h>
#include <devicereg.h>
#include <session.h>
#include <structs.h>
#include <sredird.h>

 MODULEID("$Id: telnet.c,v 1.2 1999/09/23 08:12:42 ayman Exp $");


 unsigned telnetport=7000,
          telnetsock;
 

 int InitTelnet (void)

 {
  int telnet_sock,
      reuse=1;
  unsigned long port, oport;
  time_t now;
  struct Socket *s;
  struct sockaddr_in telnet;
  extern msredird *masterptr;

    if (!(port=oport=atoi(valueof("DEFAULT_TELNET_PORT"))))
     {
      say ("*** Assigning DEFAULT_TELNET_PORT...\n");

      SetPref ("DEFAULT_TELNET_PORT", "7000");
      oport=port=7000;
     }

  memset ((char *)&telnet, 0, sizeof(telnet));
  telnet.sin_family=AF_INET;
  telnet.sin_port=htons (port);
  telnet.sin_addr.s_addr=htonl (INADDR_ANY);

  telnet_sock=RequestSocket();

  setsockopt (telnet_sock, 
	      SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

   while (bind(telnet_sock, (struct sockaddr *)&telnet,sizeof(telnet))<0)           {
      if (errno==EADDRINUSE)
       {
	say ("Port %u in use - Incrementing port number...\n", port);

	port+=10;
	telnet.sin_port=htons(port);
       }
      else
       {
	say ("Unable to setup Telnet service - %s.\n",
	      strerror(errno));

	close (telnet_sock);

	return 0;
       }
    }

   if ((listen(telnet_sock, 1)<0)&&(errno!=EINTR))
    {
     say ("Unable to setup telnet service - %s.\n",
	  strerror(errno));

     close (telnet_sock);

     return 0;
    }

   if (oport!=port)
    {
     char s[10];

      say ("*** Resetting DEFAULT_TELNET_PORT...\n");

      sprintf (s, "%ul", port);
      SetPref ("DEFAULT_TELNET_PORT", s);
    }

  xmalloc(s, (sizeof(Socket)));
  memset (s, 0, sizeof(Socket));

  s->sock=(telnetsock=telnet_sock);
  s->when=time (NULL);
  s->port=ntohs (telnet.sin_port);
  strcpy (s->address, "localhost");

  telnetport=ntohs (telnet.sin_port);
  masterptr->telnetsock=s;
 
  say ("Now listening on port %u\n", /*telnet*/port);

  return telnet_sock;

 }  /**/


 int AnswerTelnetRequest (void)
 
 {
  int i=0,
      nsocket,
      sin_size,
      opt=1;
  char *host;
  Socket *sptr;
  time_t now;
  struct sockaddr_in hisaddr;
  struct hostent *hp;
  extern char *c_version;
  extern int CheckDeviceAvailability (Socket *);

   sin_size=sizeof (struct sockaddr_in);
   nsocket=accept (telnetsock, (struct sockaddr *)&hisaddr, &sin_size); 

    if ((nsocket<0)&&(errno!=EWOULDBLOCK))
     {
      say ("Unable to accept telnet connection (accept()).\n");

      return 0;
     }
 
   host=(char *)inet_ntoa (hisaddr.sin_addr);
 
   setsockopt (nsocket, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof(int));

   xmalloc(sptr, (sizeof(Socket)));
   memset (sptr, 0, sizeof(Socket));

   sptr->sock=nsocket;

   sptr->when=time(NULL);
   strcpy (sptr->haddress, host);
   sptr->type=SOCK_CONNECTEDTELNET;
   sptr->hport=ntohs(hisaddr.sin_port);
   sptr->port=7000;

   say ("Telnet connection from %s.\n",  sptr->haddress);

    if (!(CheckDeviceAvailability(sptr)))
     {
      close (sptr->sock);
      free (sptr);

      return 0;
     }

   return nsocket;

 }  /**/

 
 int CheckDeviceAvailability (Socket *sptr)

 {
  int ndev;
  Session *sesnptr;
  Modem *mptr;
  extern msredird *const masterptr;

   say ("*** Checking device availability...\n");


   ndev=HowManyInDeviceReg ();

    if (ndev==0)
     {
      return 0;  /* no devices at all */
     }

    if (masterptr->inuse==ndev)
     {
      logf(masterptr, "010 Fullhouse--connection from %s rejected!\n",
           sptr->haddress);

      return 0;  /* all devices in use! */
     }

    if (!(mptr=OpenDevice()))  return 0;

    if (!(sesnptr=OpenSession(mptr, sptr)))  return 0;

   sptr->index=(void *)mptr;
   mptr->sptr=sptr;

   mptr->SockParm=IPTOS_LOWDELAY;
   setsockopt (sptr->sock, SOL_SOCKET, SO_KEEPALIVE, &mptr->SockParmEnable,
	       sizeof(mptr->SockParmEnable));
   setsockopt (sptr->sock, SOL_TCP, TCP_NODELAY, &mptr->SockParmEnable,
	       sizeof(mptr->SockParmEnable));
   setsockopt (sptr->sock, SOL_IP, IP_TOS, &mptr->SockParm,
	       sizeof(mptr->SockParm));

    /* Lock device */

   say ("012 Allocated %s for %s...\n", mptr->devname, sptr->haddress);
   logf(masterptr, "012 Allocated %s for %s...\n",
        mptr->devname, sptr->haddress);

   SendTelnetOption (sesnptr, TNWILL, TNCOM_PORT_OPTION);

   {
    pthread_attr_t pthread_attr_default;

     pthread_create (&sesnptr->th, NULL, 
                     Thread, (void *)sesnptr);
   }
 
   return 1;
  
 }  /**/


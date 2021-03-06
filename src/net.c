/*
** net.c Copyright (c) 1998,99 Ayman Akt
**
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <main.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sockets.h>
#include <net.h>
#include <telnet.h>

 MODULEID("$Id: net.c,v 1.1 1999/09/23 08:20:06 ayman Exp $");

 char *localhost="";
 unsigned long localip;
 struct resolved result;
 struct resolved *const r_ptr=&result;
 static f=1;


 void InitNet (void)

 {
   InitTelnet ();

 }  /**/


#include <sys/stat.h>

 void FetchLocalhost (void)

 {
  int x;
  char p[MAXHOSTLEN]={0},
       *s;
  struct hostent *hp;
  struct in_addr in;

   x=gethostname (p, sizeof(p));

    if (x==-1)
     {
      s=(char *)getenv ("HOSTNAME");

       if (!s)
        {
         say ("### Host self-lookup failed.\n");
        }
     }
    else
     {
       if (hp=gethostbyname(p))
        {
         memcpy (&in, hp->h_addr, sizeof(in));
         localhost=strdup((char *)hp->h_name);
         localip=ntohl (in.s_addr);
        }
       else
        {
         say ("%s %s\n",
              "### Someting seriously wrong with this machine or",
              "with you or with both of you!");
        }
     }

   say ("*** Local net %s %s (%lu)\n",
        localhost, RawIPToDotted(localip), localip);

 }  /**/


 int ResolveAddress (const char *address, ResolvedAddress *raddr)

  {

  }  /**/


 int RequestSocket (void)

 {
  int nsocket;
  
    if ((nsocket=socket(AF_INET, SOCK_STREAM, 0))<0)
     {
      say ("Unable to obtain socket -  %s.\n", strerror(errno));

      return 0;
     }

    return nsocket;

 }  /**/


 int SetSocketFlags (int socket, int ON_OFF, int flags)

 {
  int cur;

     if ((cur=fcntl(socket, F_GETFL, 0))<0)
      {
       say ("Unable to F_GETFL socket - %s.\n", strerror(errno));

       return 0;
      }

   switch (ON_OFF)
     {
      case 1:
       cur|=flags;
       break;

      case 0:
       cur&=~flags;
       break;
      }
    
    if ((fcntl(socket, F_SETFL, cur))<0)
     {
      say ("Unable to set socket flags - %s.\n", strerror(errno));

      return 0;
     }

   return 1;

 }  /**/


 char *RawIPToDotted (unsigned long raw)

 {
  static char rv[16];
  unsigned long ip;
 
   ip=htonl (raw);
 
   sprintf (rv, "%d.%d.%d.%d",
            (int)((ip>>0)&0xFF),
            (int)((ip>>8)&0xFF),
            (int)((ip>>16)&0xFF),
            (int)((ip>>24)&0xFF));

   return rv;

 }  /**/


 char *HostToDotted (char *host)
 
 {
  unsigned long ip;
  struct hostent *hp;
  struct in_addr addr;
   
    if (isdottedquad(host))  return host;

   hp=gethostbyname (host);

    if (hp==(struct hostent *)NULL)  return ((char *)NULL);

  memcpy ((void *)&addr, (void *)hp->h_addr, sizeof(addr));

  ip=ntohl (addr.s_addr);

   if (!ip)  return ((char *)NULL);

  return (RawIPToDotted(ip));

 }  /**/


 char *DottedToHost (char *dip) 
 {
  struct sockaddr_in addr;
  struct hostent *hp;

   addr.sin_addr.s_addr=inet_addr (dip);

   hp=gethostbyaddr ((char *)&addr.sin_addr, sizeof(addr.sin_addr), AF_INET);
    if (hp==(struct hostent *)NULL)  return ((char *)dip);

   return ((char *)hp->h_name);

 }  /**/


 int isdottedquad (const char *address)

 {
  register int n,
           numbered=1;

   n=strlen(address)-1;

    while ((address[n]!='.')&&(n))
    {
      if ((address[n]<'0')||(address[n]>'9'))
        {
         numbered=0;
         break;
        }

     n--;
    }

   return numbered;

 }  /**/


 static char *_h_errlist[]={
    "You should be fine",
    "Host not found",
    "Host name lookup failure",
    "Unknown server error",
    "No address associated with name",
    "Service unavailable",
};

static int _h_nerr = sizeof(_h_errlist)/sizeof(_h_errlist[0]);

 char *h_strerror (int error)

 {
  static char aux[35];

    if ((error<0)||(error>_h_nerr)) 
       {
        sprintf (aux, "Unknown resolver error");

        return (char *)aux;
       }

   return (char *)_h_errlist[error];

 }  /**/


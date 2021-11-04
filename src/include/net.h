/*
** net.h Copyright (c) 1998 Ayman Akt
**
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: net.h,v 1.1 1999/07/26 01:46:59 ayman Exp $")
**
*/

 struct resolved {
         char dns[MAXHOSTLEN];
         char dotted[14];
         unsigned long ip;
        };

 struct ResolvedAddress {
        char *dns,
             *dotted;
        unsigned flags;
        struct in_addr *inetaddr;
       };
 typedef struct ResolvedAddress ResolvedAddress;


 void InitNet (void); 
 void FetchLocalhost (void);
 int InitTelnet (void);
 int isdottedquad (const char *);
 int RequestSocket (void);
 int SetSocketFlags (int, int, int);
 int ResolveAddress (const char *, ResolvedAddress *);
 void nslookup (char *);
 char *RawIPToDotted (unsigned long);
 char *HostToDotted (char *);
 char *DottedToHost (char *);
 char *h_strerror (int);


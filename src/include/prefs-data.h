/*
**
MODULEID("$Id: prefs-data.h,v 1.1 1999/08/03 02:09:34 ayman Exp $")
**
*/

 #include "prefsfunc.h"

 #define ENTRIES_IN_PREFSTABLE 7

 /* template from which a binary tree is built */
 static const struct Preference Preferences[]=
 {
  /* sig. str to be sent as id */
  {"SIGNATURE_STR",       NULL,          NULL,               OTHER  },
  /* tcp port on which the server listens */
  {"DEFAULT_TELNET_PORT", NULL,          "7000",             DIGITS },
  /* configuration dir-default working dir for the server */
  {"CONFIG_DIR",          NULL,          "/etc/msredird",     OTHER  },
  /* a ':' seperated list of serial devices corresponding to modems-not in use*/
  {"DEVICE_LIST",         NULL,          NULL,               OTHER  },
  /* if ON, a user can use more than one modem at a time-not in use */
  {"ALLOW_MULTIBLE",      NULL,          "OFF",              BOOLEAN},
  /* To be reviewed-not in use */
  {"AUTO_SHOOT",          NULL,          "OFF",              BOOLEAN},
  /* to be reviewed-not in use */
  {"COMMAND_CHAR",        NULL,          "/",                OTHER  },
 };


/*
** Copyright (c) 1998 Ayman Akt
**
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: prefs.h,v 1.1 1999/08/03 02:11:10 ayman Exp $")
**
*/

#define READONLY  1
#define READWRITE 2

#define BOOLEAN 1
#define DIGITS  2
#define OTHER   3

 struct Preference  {
         const char *preference;
         void (*func)(struct Preference *, char *);
         char *value;
         int level;
        };
 typedef struct Preference Preference;

 struct PrefsTable {
         struct Preference entry;
         struct PrefsTable *right;
         struct PrefsTable *left;
        };
 typedef struct PrefsTable PrefsTable;

 PrefsTable *BuildPreferencesTable (void);
 void PrintEntries (const PrefsTable *);
 void ShowPreferences (const PrefsTable *, int);
 PrefsTable *FindEntry (const char *);
 char *valueof (const char *);
 int SetPref (const char *, char *);
 boolean set (char *);
 boolean sets (char *);


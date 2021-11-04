/*
** perfs.c Copyright (c) 1998 Ayman Akt
**
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <main.h>
#include <prefs.h>
#include <prefs-data.h>

 MODULEID("$Id: prefs.c,v 1.1 1999/08/03 02:13:30 ayman Exp $");

 static PrefsTable *AllocEntry (const struct Preference *);
 static PrefsTable *AddtoPrefsTable (PrefsTable *);

 PrefsTable *preferences;

 PrefsTable *BuildPreferencesTable (void)

 {
  register int i;
  float j;
  register PrefsTable *ptr;

   preferences=NULL;

    for (i=0; i<=(ENTRIES_IN_PREFSTABLE-1); i++)
     {
      say ("*** Building Preferences Table: %d entries...\r", i+1);
         
       if ((ptr=AllocEntry(&Preferences[i]))!=NULL)
        {
         preferences=AddtoPrefsTable (ptr);
        }
     }

   printf ("\n");

   return (PrefsTable *)preferences;

 }  /**/


 static PrefsTable *AllocEntry (const struct Preference *entry)

 {
  register PrefsTable *ptr;

    if ((ptr=(PrefsTable *)malloc(sizeof(PrefsTable)))!=NULL)
     {
       if (entry->value)
        {
         ptr->entry.value=(char *)malloc(strlen(entry->value)+1);
         strcpy (ptr->entry.value, entry->value);
        }
       else
        {
         ptr->entry.value=NULL;
        }

      ptr->entry.func=entry->func;
      ptr->entry.preference=entry->preference;
      ptr->entry.level=entry->level;

      ptr->right=NULL;
      ptr->left=NULL;

       if (ptr->entry.func)
        {
         ptr->entry.func(&ptr->entry, ptr->entry.value);
        }

      return (PrefsTable *)ptr;
     }
    else
     {
      return (PrefsTable *)NULL;
     }

 }  /**/


 static PrefsTable *AddtoPrefsTable (PrefsTable *new)

 {
  register PrefsTable *aux;

   aux=preferences;

    while (aux!=NULL)
     {
       if (strcasecmp(new->entry.preference, aux->entry.preference)<0)
        {
          if (aux->left)
           {
            aux=aux->left;
           }
          else
           {
            aux->left=new;

            break;
           }
        }
       else
       if (strcasecmp(new->entry.preference, aux->entry.preference)>0)
        {
          if (aux->right)
           {
            aux=aux->right;
           }
          else
           {
            aux->right=new;

            break;
           }
        }
       else
        {
         say ("*** Dublicate entry in Preferences (%s)!\n",
              new->entry.preference);
        }
     }

   new->left=new->right=NULL;

    if (preferences==NULL)
     {
      preferences=new;
     }

   return (PrefsTable *)preferences;

 }  /**/   
   

 static int counter;

 #define PREFS_FORMAT " %$179%-2d%$179%-20.20s%$179%-25.25s%$179\n"
 #define PREFS_ARGS \
  counter++, ptr->entry.preference, \
  ptr->entry.value?ptr->entry.value:"Unset"

 void PrintEntries (const PrefsTable *ptr)

 {
#if 0
    if (ptr)
     {
      PrintEntries (ptr->left);
      say (PREFS_FORMAT, PREFS_ARGS); 
      PrintEntries (ptr->right);
     }
#endif
 }  /**/


 void ShowPreferences (const PrefsTable *ptr, int how)

 {
#if 0
   counter=1;

   DisplayHeader (11, 3);

    if (how)
       {
        PrintEntries (ptr);
       }
    else
       {
       
       }

   finish2 (11, 3);
 #endif 
 }  /**/


 __inline__ PrefsTable *FindEntry (const char *key)

 {
  register int r,
               found=0;
  register PrefsTable *aux;

  aux=preferences;

   while ((aux)&&(!found))
    {
     if ((r=strcasecmp(key, aux->entry.preference))==0)
      {
       found=1;
      }
     else
     if (r<0)
      {
       aux=aux->left;
      }
     else
      {
       aux=aux->right;
      }
    }

   if (!found)
    {
     return (PrefsTable *)NULL;
    }

  return (PrefsTable *)aux;

 }  /**/

 
 char *valueof (const char *pref)

 {
  register PrefsTable *ptr;

    if ((!pref)||(!*pref))  return (char *)NULL;

   ptr=FindEntry (pref);

    if (ptr)
     {
      if (ptr->entry.value)
       {
        return ptr->entry.value;
       }
      else
       {
        return (char *)NULL;
       }
     }

   return (char *)NULL;

 }  /**/
    

 int SetPref (const char *pref, char *nvalue)

 {
  char *aux=NULL;  /* points to old value */
  register PrefsTable *ptr;
 
   ptr=FindEntry (pref);

    if (ptr)
       {
         if (ptr->entry.level==DIGITS)
          {
            if (!isdigits(nvalue))
             {
              return -1;
             }
          }
         else
         if (ptr->entry.level==BOOLEAN)
          {
            if ((!arethesame(nvalue, "ON"))&&(!arethesame(nvalue, "OFF")))
             {
              return -2;
             }
          }

         if ((ptr->entry.value)&&(arethesame(ptr->entry.value, nvalue)))
          {
           return;
          }

        aux=ptr->entry.value;

         if (!(ptr->entry.value=strdup(nvalue)))
          {
           say ("### Memory exhaustion--exiting...\n");

           exit (1);
          }

         if (ptr->entry.func)
          {
           ptr->entry.func(&ptr->entry, aux); /* pass it the new & old */
          }

         if (aux)  free (aux);
       }
    else
       {
        return 0; 
       }

   return 1;

 }  /**/


 boolean set (char *value)

 {
   return ((boolean)(strcasecmp(valueof(value), "ON")==0));

 }  /**/


 boolean sets (char *value)

 {
   return ((boolean)(!valueof(value)?0:1));

 }  /**/


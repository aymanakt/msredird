/*
** configfile.c Copyright (c) 1999 Ayman Akt
**
** See the COPYING file for terms of use and conditions.
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <main.h>
#include <list.h>
#include <prefs.h>
#include <configfile.h>

#include <sys/stat.h>

 MODULEID("$Id: configfile.c,v 1.1 1999/08/05 03:30:26 ayman Exp $");


 /*
 ** should return the size of the file
 */
 static unsigned long FileExist (char *file)

 {
  char *s;
  char path[_POSIX_PATH_MAX];
  struct stat st;

    if (!(s=valueof("CONFIG_DIR")))
     {
      say ("*** Resetting CONFIG_DIR to default value...\n");

      s="/etc/msredird";

      SetPref ("CONFIG_DIR", s);
     }

   sprintf (path, "%s/%s", s, CONFIG_FILE_NAME);

   strcpy (file, path);

    if ((access(path, F_OK|R_OK))<0)
     {
      say ("### Configuration file's missing! Reverting to defaults - %s.\n",
           strerror(errno));

      return 0;
     }

    if ((stat(path, &st))<0)
     {
      say ("### Configuration file's missing! Reverting to defaults - %s.\n", 
            strerror(errno));

      return 0;
     }

    return st.st_size;

 }  /**/


 static  ProcessSETS (char *line)

 {
  char *s;

   s=splitw (&line);

    if (!line)
     {
      /* `DEFAULT_NAME' empty set definition */
      return;
     }

   wipespaces (line);

   SetPref (s, line);  /* check for errors here */

   return;

 }  /**/


 static void ActuallyProcess (char *line, int mode)

 {
    if (0)
     {
      ;
     }
    else
    if (mode&SETS)
     {
      ProcessSETS (line);
     }

 }  /**/


 int ProcessConfigfile (void)

 {
  char line[80],
       path[_POSIX_PATH_MAX];
  int n=0;
  unsigned long size,
                section=0;
  float j;
  FILE *fp;
  struct stat st;

   size=FileExist (path);
      
    if (!size)  return;

    if (!(fp=fopen(path, "r")))
     {
      say ("*** Configuration file (%s) missing! Reverting to defaults.\n",
           path);

      return;
     }

   say ("*** Processing Configuration File\r");

    while (!feof(fp))
     {
      fgets (line, 79, fp);
      n++;

       if (!feof(fp))
        {
         say ("*** Processing Configuration File (%s) %d lines...\r", path, n);

         wipespaces (line);

          if ((line[0]!='#')&&(line[0]))
           {
             if (line[0]=='[')
              {
                if ((strncasecmp(&line[0], "[SETS]", 6)==0) &&
                    !(section&SETS))
                 {
                  section|=SETS;
                  section&=~(SERVERS|UTMPNOTIFY);
                  continue;
                 }

               section&=~THELOT;
              }
             else
             if (line[0])
              {
               ActuallyProcess (line, section);
              }
           }
        }
     }

   fclose (fp);
  
   say ("\n");

 }  /**/


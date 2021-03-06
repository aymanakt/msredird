/*
** list.c Copyright (c) 1998 Ayman Akt
**
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: list.h,v 1.1 1999/07/26 01:46:59 ayman Exp $")
**
*/

 struct ListEntry {
         void  *whatever;
         struct ListEntry *next;
        };
 typedef struct ListEntry ListEntry;


 struct List {
         int  glock,
              nEntries;
         ListEntry *head,
                   *tail;
        };
 typedef struct List List;


 struct dListEntry {
         void *whatever;
         struct dListEntry *next,
                           *prev;
        };
 typedef struct dListEntry dListEntry;

 struct dList {
         int nEntries;
         dListEntry *head,
                    *tail;
        };
 typedef struct dList dList;

#define LISTNODE(x) ((x)->whatever)
#define NO_ENTRIES(x) ((x)->nEntries==0)

 ListEntry *AddtoList (List *);
 int RemovefromList (List *, ListEntry *);
 int RemoveListHead (List *, int, void (*)(ListEntry *));
 List *ListfromArray (void *, size_t, size_t);
 void CleanupListfromArray (List *);
 ListEntry *LocateEntry (List *, ListEntry *);
 boolean ListEmpty (const List *);
 dListEntry *AddtodList (dList *);
 dListEntry *RemovefromdList (dList *, dListEntry *);
 boolean dListEmpty (const dList *);


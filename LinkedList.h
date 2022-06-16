//LinkedList.c Copyright 2008, 2009, 2010, 2011 OnyxSoft.
#ifndef ONYXSOFT_LINKEDLIST
#define ONYXSOFT_LINKEDLIST

#include <proto/exec.h>

/* Spec:
IF length THEN head AND cur ELSE head AND cur = NIL
IF length>1 THEN prev ELSE prev = NIL
IF length=1 THEN head=cur
IF length THEN pstn == cur

All operations on LL_FIRST and LL_CURRENT is O(1)

llNew(ll,pool,storedata) is the constructor and returns TRUE for success.
Must be called first with a ptr to a LinkedList, optional exec pool and
a flag if data is strings that are to be allocated.
If pool=NULL one will be allocted and llEnd must be called to free the pool
at end of usage.

llEnd() frees the list contents and the pool if internally allocated.

pos() returns actual position set
pos(LL_LAST):  O(N)
pos(pos):      O(N)

get() returns NIL if list is empty else LLType
get(LL_FIRST): does NOT update LL_CURRENT to LL_FIRST
get(LL_LAST):  O(N)
get(pos):      O(N) unless pos=LL_CURRENT+1 then O(1)

set() returns success bool (FALSE if out or mem of no entries in list)
set(): always updates LL_CURRENT

add() returns success bool
add(LL_FIRST): does NOT update LL_CURRENT to LL_FIRST
add(LL_LAST):  O(N) first time, then O(1)
add(pos):      O(N) unless pos=LL_CURRENT+1 then O(1)

rem() returns success bool
rem(LL_FIRST): does NOT update LL_CURRENT to LL_FIRST
rem(LL_LAST):  always O(N)
rem(pos):      O(N)

inc() returns advance success bool
inc():         O(1)

dequeue() doesn't return anything when using strings. Use get() first.

String usage removed in C++-version.
String usage added in C-version.
'LLType' must be typedef'ed or #define'd to the datatype used.
*/

typedef struct MapData
{
   WORD x1;
   WORD y1;
   WORD x2;
   WORD y2;
   WORD pos;
} MapData;


#ifndef LLType
 #define LLType struct MapData*
#endif

#ifndef NIL
 #define NIL 0
#endif

#define LL_CURRENT -1
#define LL_LAST -2
#define LL_FIRST -3

typedef struct ll_list {
 struct ll_list *next;
 LLType data;
} ll_list;

typedef struct linkedlist {
 long len;      //public
 long position; //public
 ll_list *head, *cur, *prev;
 long length;
 long pstn;
 void *pool;
 char intpool;
 char str;
} LinkedList;

char llNew(LinkedList *ll,void *pool,char copydata);
long llPos(LinkedList*,long);
LLType llGet(LinkedList*,long);
char llSet(LinkedList*,LLType,long);
char llAdd(LinkedList*,LLType,long);
char llRem(LinkedList*,long);
char llInc(LinkedList*);
void llClear(LinkedList*);
LLType llDequeue(LinkedList*);
char llEnqueue(LinkedList *ll,LLType x);
char llPush(LinkedList *ll,LLType x);
char llPop(LinkedList *ll);
LLType llTop(LinkedList *ll);
void llEnd(LinkedList *ll);

#endif


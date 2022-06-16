/* LinkedList.c Copyright 2008, 2009, 2010, 2011 OnyxSoft.
cd Project:SDK/Local/common/include/onyxsoft/
gcc -c -O2 -o linkedlist.o linkedlist.c
ar rc libll.a linkedlist.o
move libll.a //lib/
*/
#include "LinkedList.h"
#include <sys/types.h>
#include <string.h>
#include <proto/exec.h>
#ifdef __amigaos4__
#include <inline4/exec.h>
#endif

char llNew(LinkedList *ll,void *pool,char copydata) {
 ll->length=0;
 ll->len=0;
 ll->head=NIL;
 ll->cur=NIL;
 ll->prev=NIL;
 ll->intpool=!pool;
 if(pool)
   ll->pool=pool;
 else {
   ll->pool=CreatePool(MEMF_ANY,4096,256);
 }
 ll->str=copydata;
 return ll->pool!=NULL;
}

long llPos(LinkedList *ll,long pos) {
 if(ll->length) {
  if(pos<-3) pos=LL_FIRST;
  switch(pos) {  
  case LL_FIRST:
   ll->cur=ll->head;
   ll->prev=NIL;
   ll->pstn=0;
   break;
  case LL_LAST:
   while(ll->cur->next) {
    ll->prev=ll->cur;
    ll->cur=ll->cur->next;
    ll->pstn++;
   }
   break;
  case LL_CURRENT:
   break;
  default:
   if(ll->pstn<pos) {
    while(ll->pstn<pos && ll->cur->next) {
     ll->prev=ll->cur;
     ll->cur=ll->cur->next;
     ll->pstn++;
    }
   }
   else if(ll->pstn>pos) {
    ll->cur=ll->head;
    ll->prev=NIL;
    ll->pstn=0;
    while(ll->pstn<pos && ll->cur->next) {
     ll->prev=ll->cur;
     ll->cur=ll->cur->next;
     ll->pstn++;
    }
   }
  }
  ll->position=ll->pstn;
  return ll->pstn;
 }
 return -1;
}

LLType llGet(LinkedList *ll,long p) {
 if(ll->length) {
  switch(p) {
  case LL_CURRENT: return ll->cur->data;
  case LL_FIRST: return ll->head->data;
  default:
   llPos(ll,p);
   return ll->cur->data;
  }
 }
 return NIL;
}

char llSet(LinkedList *ll,LLType data,long p) {
 ll_list *t;
 long l;
 if(llPos(ll,p)>=0) {
  if(ll->str) {
   l=strlen(data);
   if(t=AllocPooled(ll->pool,sizeof(ll_list)+l+1)) {
    t->data=t+1;//((char*)t)+sizeof(ll_list);
    t->next=ll->cur->next;
    if(p==LL_FIRST) ll->head=t; else ll->prev->next=t;
    FreePooled(ll->pool,ll->cur,sizeof(ll_list)+strlen(ll->cur->data)+1);
    ll->cur=t;
    strcpy(ll->cur->data,data);
   }
   else
    return FALSE;
  }
  else
   ll->cur->data=data;
  return TRUE;
 }
 return FALSE;
}

char llAdd(LinkedList *ll,LLType data,long p) {
 ll_list *t;
 if(t=AllocPooled(ll->pool,ll->str?sizeof(ll_list)+strlen(data)+1:sizeof(ll_list))) {
  if(ll->str) {
   t->data=t+1;//((char*)t)+sizeof(ll_list);
   strcpy(t->data,data);
  }
  else
   t->data=data;
  if(ll->length) {
   switch(p) {
   case LL_FIRST:
    t->next=ll->head;
    ll->head=t;
    if(ll->pstn==0) ll->prev=ll->head;
    ll->pstn++;
    break;
   case LL_LAST:
    while(ll->cur->next) {
     ll->prev=ll->cur;
     ll->cur=ll->cur->next;
     ll->pstn++;
    }
    t->next=NIL;
    ll->cur->next=t;
    ll->prev=ll->cur;
    ll->cur=ll->cur->next;
    ll->pstn++;
    break;
   default:
    llPos(ll,p);
    t->next=ll->cur->next;
    ll->cur->next=t;
   }
  }
  else {
   t->next=NIL;
   ll->head=t;
   ll->cur=t;
   ll->pstn=0;
  }
  ll->length+=1;
  ll->len=ll->length;
  ll->position=ll->pstn;
 }
 return t!=NIL;
}

char llRem(LinkedList *ll,long p) {
 ll_list *t=NIL;
 if(ll->length) {
  if(ll->length==1) {
   t=ll->head;
   llNew(ll,ll->pool,ll->str);
  }
  else {
   if(p==LL_CURRENT) ll->position=ll->pstn;
   if(p+1>=ll->length)
    p=LL_LAST;
   else if(p==0 || (p<-3))
    p=LL_FIRST;
   switch(p) {
   case LL_FIRST:
    t=ll->head;
    ll->head=ll->head->next;
    if(ll->pstn==0)
     ll->cur=ll->head;
    else if(ll->pstn==1) {
     ll->prev=NIL;
     ll->pstn=0;
    }
    break;
   case LL_LAST:
    if(ll->cur->next==NIL) {
     ll->cur=ll->head;
     ll->prev=NIL;
     ll->pstn=0;
    }
    while(ll->cur->next->next) {
     ll->prev=ll->cur;
     ll->cur=ll->cur->next;
     ll->pstn++;
    }
    t=ll->cur->next;
    ll->cur->next=NIL;
    break;
   default:
    llPos(ll,p);
    t=ll->cur;
    ll->prev->next=ll->cur->next;
   }
   ll->length--;
   ll->len=ll->length;
   ll->position=ll->pstn;
  }
  FreePooled(ll->pool,t,ll->str?sizeof(ll_list)+strlen(t->data)+1:sizeof(ll_list));
 }
 return t!=NIL;
}

char llInc(LinkedList *ll) {
 if(ll->pstn+1<ll->length && ll->length) {
  ll->prev=ll->cur;
  ll->cur=ll->cur->next;
  ll->pstn++;
  ll->position=ll->pstn;
  return TRUE;
 }
 return FALSE;
}

void llClear(LinkedList *ll) {
 ll_list *t=NIL;
 while(t=ll->head) {
  ll->head=ll->head->next;
  FreePooled(ll->pool,t,ll->str?sizeof(ll_list)+strlen(t->data)+1:sizeof(ll_list));
 }
 llNew(ll,ll->pool,ll->str);
}

LLType llDequeue(LinkedList *ll) {
 LLType x;
 if(!ll->str) x=llGet(ll,LL_FIRST);
 llRem(ll,LL_FIRST);
 return x;
}

char llEnqueue(LinkedList *ll,LLType x) { return llAdd(ll,x,LL_LAST); }
char llPush(LinkedList *ll,LLType x) { return llAdd(ll,x,LL_FIRST); }
char llPop(LinkedList *ll) { return llRem(ll,LL_FIRST); }
LLType llTop(LinkedList *ll) { return llGet(ll,LL_FIRST); }

void llEnd(LinkedList *ll) {
  llClear(ll);
  if(!ll->intpool && ll->pool) DeletePool(ll->pool);
  ll->pool=NULL;
}


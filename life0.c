/*____________________________________________________________________________
**
** File:          life.c
**
** Description:   This is a realisation of the game `LIFE' on practicaly
**                unlimited field
**
** Author:        Alexander V. Lukyanov
**____________________________________________________________________________
*/

#include <stdio.h>
#ifdef __TURBOC__
	#include <alloc.h>
#else
	#include <malloc.h>
#endif
#include <ctype.h>
#include "life0.h"
#include "check.h"

struct   count
{
   long     x;
   char     cnt;
   struct   count *next;
};

struct   y_list   *Field=NULL;

struct   count    *Count;
struct   count    **Base;


void     FreeLine(struct x_list *line)
{
   struct   x_list   *save;

   while(line!=NULL)
   {
      save=line->next;
      free(line);
      line=save;
   }
}

void  FreeField(void)
{
   void     *SavePtr;

   while(Field!=NULL)
   {
      FreeLine(Field->line);
      SavePtr=Field->next;
      free(Field);
      Field=SavePtr;
   }
}

void  Increment(long x)
{
   struct   count **Scan;
   struct   count *NewNode;

   while(*Base!=NULL && (*Base)->x<x-1)
      Base=&((*Base)->next);

   for(Scan=Base; *Scan!=NULL && (*Scan)->x<x; Scan=&((*Scan)->next));
   if(*Scan!=NULL && (*Scan)->x==x)
      (*Scan)->cnt++;
   else
   {
      CheckPtr(NewNode=malloc(sizeof(*NewNode)));
      NewNode->cnt=1;
      NewNode->next=*Scan;
      NewNode->x=x;
      *Scan=NewNode;
   }
}

void  Step(void)
{
   struct   y_list   *Input;
   struct   y_list   **Output;

   struct   x_list   *PrevLine;
   struct   x_list   *CurrLine;
   struct   x_list   *NextLine;

   struct   y_list   *NewYNode;
   struct   x_list   *NewXNode;

   struct   x_list   *NewLine;
   struct   x_list   **Add;

   void     *SavePtr;

   struct   x_list   *ScanX;

   long  Y;

   PrevLine=CurrLine=NextLine=NULL;
   NewLine=NULL;
   Count=NULL;

   Input=Field;
   Output=&Field;

   do
   {
      /* check if we can jump over empty space */
      if(PrevLine==NULL && CurrLine==NULL && NextLine==NULL && NewLine==NULL)
      {
         if(Input==NULL)   /* if there is no more spots then it is done */
            break;
         NextLine=Input->line;
         Y=Input->y-1;
         Input=Input->next;
      }

      /* build list of neighbour counts for current line */
      Base=&Count;
      for(ScanX=PrevLine; ScanX!=NULL; ScanX=ScanX->next)
      {
         Increment(ScanX->x-1);
         Increment(ScanX->x);
         Increment(ScanX->x+1);
      }
      Base=&Count;
      for(ScanX=CurrLine; ScanX!=NULL; ScanX=ScanX->next)
      {
         Increment(ScanX->x-1);
         Increment(ScanX->x+1);
      }
      Base=&Count;
      for(ScanX=NextLine; ScanX!=NULL; ScanX=ScanX->next)
      {
         Increment(ScanX->x-1);
         Increment(ScanX->x);
         Increment(ScanX->x+1);
      }

		while(*Output!=NULL && (*Output)->y<Y-1)
			Output=&((*Output)->next);

		/* build into field the new line which was created for previous line */
		if(NewLine!=NULL)
		{
			if(PrevLine==NULL)
			{
				/* insert new record into Y list */
				CheckPtr(NewYNode=malloc(sizeof(*NewYNode)));
            NewYNode->y=Y-1;
            NewYNode->next=*Output;
            NewYNode->line=NewLine;
            *Output=NewYNode;
         }
         else
         {
            FreeLine((*Output)->line);
            (*Output)->line=NewLine;
         }
      }
      else
      {
         if(PrevLine!=NULL)
         {
            /* delete record from Y list */
            SavePtr=*Output;
            FreeLine((*Output)->line);
            *Output=(*Output)->next;
            free(SavePtr);
         }
      }

      /* create new line for current line */
      NewLine=NULL;
      Add=&NewLine;
      ScanX=CurrLine;

      while(Count!=NULL)
      {
         while(ScanX!=NULL && ScanX->x<Count->x)
            ScanX=ScanX->next;
         /* apply rules of surviving and borning */
         if(Count->cnt==3 || (Count->cnt==2 && ScanX!=NULL
                                                   && Count->x==ScanX->x))
         {
            CheckPtr(NewXNode=malloc(sizeof(*NewXNode)));
            NewXNode->next=NULL;
            NewXNode->x=Count->x;
            *Add=NewXNode;
            Add=&(NewXNode->next);
         }
         SavePtr=Count->next;
         free(Count);
         Count=SavePtr;
      }

      /* move to the next line */
      PrevLine=CurrLine;
      CurrLine=NextLine;
      Y++;
      if(Input!=NULL && Input->y==Y+1)
      {
         NextLine=Input->line;
         Input=Input->next;
      }
      else
      {
         NextLine=NULL;
      }
   }
   while(1);
}

void  Print(FILE *file)
{
   struct   y_list   *ScanY;
   struct   x_list   *ScanX;
   long     Leftmost;
   long     X,Y;

   if(Field==NULL)
      return;

   Y=Field->y;

   Leftmost=Field->line->x;
   for(ScanY=Field->next; ScanY!=NULL; ScanY=ScanY->next)
      if(ScanY->line->x<Leftmost)
         Leftmost=ScanY->line->x;

   for(ScanY=Field; ScanY!=NULL; ScanY=ScanY->next)
   {
      while(Y<ScanY->y)
      {
         putc('\n',file);
         Y++;
      }
      X=Leftmost;
      for(ScanX=ScanY->line; ScanX!=NULL; ScanX=ScanX->next)
      {
         while(X<ScanX->x)
         {
            putc(' ',file);
            X++;
         }
         putc('*',file);
         X++;
      }
      putc('\n',file);
      Y++;
   }
}

void  Load(FILE *file)
{
   struct   y_list   **AddY;
   struct   x_list   **AddX;

   struct   y_list   *NewYNode;
   struct   x_list   *NewXNode;

   long  X,Y;
   int   ch;

   FreeField();

   AddY=&Field;

   Y=0;

   do
   {
      /* process a line */

      X=0;

      do    /* skip leading spaces */
      {
         ch=getc(file);
         if(ch=='\t')
            X=(X|7)+1;
         else if(ch=='\n')
            break;
         else if(isspace(ch))
            X++;
         else
            break;
      }
      while(1);

      if(ch!='\n' && ch!=EOF)
      {
         CheckPtr(NewYNode=malloc(sizeof(*NewYNode)));
         NewYNode->y=Y;
         NewYNode->next=NULL;
         AddX=&(NewYNode->line);
         *AddY=NewYNode;
         AddY=&(NewYNode->next);

         do
         {
            if(ch=='\t')
               X=(X|7)+1;
            else if(isspace(ch))
               X++;
            else
            {
               CheckPtr(NewXNode=malloc(sizeof(*NewXNode)));
               NewXNode->x=X;
               NewXNode->next=NULL;
               *AddX=NewXNode;
               AddX=&(NewXNode->next);
               X++;
            }
            ch=getc(file);
         }
         while(ch!='\n' && ch!=EOF);
      }
      Y++;
   }
   while(ch!=EOF);
}

int   CheckCell(long x,long y)
{
   struct   y_list   *ScanY;
   struct   x_list   *ScanX;

   ScanY=Field;
   while(ScanY!=NULL && ScanY->y<y)
      ScanY=ScanY->next;
   if(ScanY==NULL || ScanY->y!=y)
      return(0);
   ScanX=ScanY->line;
   while(ScanX!=NULL && ScanX->x<x)
      ScanX=ScanX->next;
   if(ScanX==NULL || ScanX->x!=x)
      return(0);
   return(1);
}

void  ChangeCell(long x,long y,int op)
{
   struct   y_list   **ScanY;
   struct   y_list   *NewYNode;
   struct   x_list   **ScanX;
   struct   x_list   *NewXNode;

   ScanY=&Field;
   while(*ScanY!=NULL && (*ScanY)->y<y)
      ScanY=&((*ScanY)->next);
   if(*ScanY==NULL || (*ScanY)->y!=y)
   {
      if(op==SET || op==REV)
      {
         CheckPtr(NewYNode=malloc(sizeof(*NewYNode)));
         CheckPtr(NewXNode=malloc(sizeof(*NewXNode)));
         NewYNode->next=*ScanY;
         NewYNode->y=y;
         NewYNode->line=NewXNode;
         NewXNode->next=NULL;
         NewXNode->x=x;
         *ScanY=NewYNode;
      }
      return;
   }
   ScanX=&((*ScanY)->line);
   while(*ScanX!=NULL && (*ScanX)->x<x)
      ScanX=&((*ScanX)->next);
   if(*ScanX==NULL || (*ScanX)->x!=x)
   {
      if(op==SET || op==REV)
      {
         CheckPtr(NewXNode=malloc(sizeof(*NewXNode)));
         NewXNode->next=*ScanX;
         NewXNode->x=x;
         *ScanX=NewXNode;
      }
      return;
   }
   if(op==REV || op==CLR)
   {
      NewXNode=*ScanX;
      *ScanX=(*ScanX)->next;
      free(NewXNode);
      if((*ScanY)->line==NULL)
      {
         NewYNode=*ScanY;
         *ScanY=(*ScanY)->next;
         free(NewYNode);
      }
   }
}

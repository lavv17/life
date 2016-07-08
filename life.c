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
   #include <unistd.h>
#endif
#include <ctype.h>
#include <string.h>
#include "life.h"
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

unsigned long  Generation=0;
unsigned long  Population=0;

void     FreeLine(struct x_list *line)
{
   free(line);
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
   Generation=Population=0;
}

void  Increment(long x)
{
   register struct count   **Scan;
   register struct count   *NewNode;

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
   long  PrevCnt,CurrCnt,NextCnt;

   struct   y_list   *NewYNode;
   struct   x_list   *NewXNode;

   struct   x_list   *NewLine;
   long  NewCnt;

   void     *SavePtr;

   struct   x_list   *ScanX;
   struct   count    *ScanCount;

   long  Y;
   long  i;

   PrevLine=CurrLine=NextLine=NULL;
   PrevCnt=CurrCnt=NextCnt=0;
   NewLine=NULL;
   NewCnt=0;
   Count=NULL;

   Input=Field;
   Output=&Field;

   for(;;)
   {
      /* check if we can jump over empty space */
      if(PrevLine==NULL && CurrLine==NULL && NextLine==NULL && NewLine==NULL)
      {
         if(Input==NULL)   /* if there is no more spots then it is done */
            break;
         NextLine=Input->line;
         NextCnt=Input->x_cnt;
         Y=Input->y-1;
         Input=Input->next;
      }

      /* build list of neighbour counts for current line */
      Base=&Count;
      for(ScanX=PrevLine,i=PrevCnt; i>0; i--,ScanX++)
      {
         Increment(ScanX->x-1);
         Increment(ScanX->x);
         Increment(ScanX->x+1);
      }
      Base=&Count;
      for(ScanX=CurrLine,i=CurrCnt; i>0; i--,ScanX++)
      {
         Increment(ScanX->x-1);
         Increment(ScanX->x+1);
      }
      Base=&Count;
      for(ScanX=NextLine,i=NextCnt; i>0; i--,ScanX++)
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
            Population+=NewCnt;
            CheckPtr(NewYNode=malloc(sizeof(*NewYNode)));
            NewYNode->y=Y-1;
            NewYNode->next=*Output;
            NewYNode->line=NewLine;
            NewYNode->x_cnt=NewCnt;
            *Output=NewYNode;
         }
         else
         {
            Population+=NewCnt-(*Output)->x_cnt;
            FreeLine((*Output)->line);
            (*Output)->line=NewLine;
            (*Output)->x_cnt=NewCnt;
         }
      }
      else
      {
         if(PrevLine!=NULL)
         {
            /* delete record from Y list */
            Population-=(*Output)->x_cnt;
            SavePtr=*Output;
            FreeLine((*Output)->line);
            *Output=(*Output)->next;
            free(SavePtr);
         }
      }

      /* create new line for current line */
      NewCnt=0;
      ScanX=CurrLine;
      i=CurrCnt;
      for(ScanCount=Count; ScanCount!=NULL; ScanCount=ScanCount->next)
      {
         while(i>0 && ScanX->x<ScanCount->x)
         {
            ScanX++;
            i--;
         }
         /* apply rules of surviving and borning */
         if(ScanCount->cnt==3 || (ScanCount->cnt==2 && i>0 && ScanCount->x==ScanX->x))
         {
            ScanCount->cnt=0;    /* indicate this place is to be filled */
            NewCnt++;
         }
      }

      if(NewCnt>0)
         NewLine=CheckPtr(malloc(NewCnt*sizeof(*NewLine)));
      else
         NewLine=NULL;

      NewXNode=NewLine;
      while(Count!=NULL)
      {
         if(Count->cnt==0)
         {
            NewXNode->x=Count->x;
            NewXNode++;
         }
         SavePtr=Count->next;
         free(Count);
         Count=SavePtr;
      }

      /* move to the next line */
      PrevLine=CurrLine;
      PrevCnt=CurrCnt;
      CurrLine=NextLine;
      CurrCnt=NextCnt;
      Y++;
      if(Input!=NULL && Input->y==Y+1)
      {
         NextLine=Input->line;
         NextCnt=Input->x_cnt;
         Input=Input->next;
      }
      else
      {
         NextLine=NULL;
         NextCnt=0;
      }
   }
   Generation++;  /* we've got the next generation ! */
}

void  Print(FILE *file)
{
   struct   y_list   *ScanY;
   struct   x_list   *ScanX;
   long     Leftmost;
   long     X,Y;
   long     i;

   if(Field==NULL)
      return;

   Y=Field->y;

   Leftmost=Field->line->x;
   for(ScanY=Field->next; ScanY!=NULL; ScanY=ScanY->next)
      if(ScanY->line->x<Leftmost)
	 Leftmost=ScanY->line->x;

   fprintf(file,"G%luX%ldY%ld\n",Generation,Leftmost,Field->y);

   for(ScanY=Field; ScanY!=NULL; ScanY=ScanY->next)
   {
      while(Y<ScanY->y)
      {
         putc('\n',file);
         Y++;
      }
      X=Leftmost;
      for(ScanX=ScanY->line,i=ScanY->x_cnt; i>0; i--,ScanX++)
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

   struct   y_list   *NewYNode;
   struct   x_list   *NewXNode;

   long  X,Y,ShiftX,ShiftY;
   int   ch;
   long  oldpos;

   FreeField();

   AddY=&Field;

   Y=0;

   ShiftX=ShiftY=0;
   fscanf(file,"G%luX%ldY%ld%*c",&Generation,&ShiftX,&ShiftY);

   do
   {
      /* process a line */

      X=0;

      for(;;)  /* skip leading spaces */
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

      if(ch!='\n' && ch!=EOF)
      {
         CheckPtr(NewYNode=malloc(sizeof(*NewYNode)));
         NewYNode->y=Y+ShiftY;
         NewYNode->next=NULL;
         NewYNode->line=NULL;
         NewYNode->x_cnt=0;
         *AddY=NewYNode;
         AddY=&(NewYNode->next);

         ungetc(ch,file);
         oldpos=ftell(file);

         for(;;)
         {
            ch=getc(file);
            if(ch=='\n' || ch==EOF)
               break;
            if(!isspace(ch))
            {
               NewYNode->x_cnt++;
               Population++;
            }
         }

         if(NewYNode->x_cnt>0)
         {
            fseek(file,oldpos,SEEK_SET);
            NewXNode=NewYNode->line=CheckPtr(malloc(NewYNode->x_cnt*sizeof(*NewXNode)));
            for(;;)
            {
               ch=getc(file);
               if(ch=='\n' || ch==EOF)
                  break;
               if(ch=='\t')
                  X=(X|7)+1;
               else if(isspace(ch))
                  X++;
               else
               {
                  NewXNode->x=X+ShiftX;
                  NewXNode++;
                  X++;
               }
            }
         }
      }
      Y++;
   }
   while(ch!=EOF);
}

int   FindInXList(long x,struct x_list *XList,long cnt,long *index)
{
   long  left=-1,right=cnt;
   long  center;
   long  diff;

   while(right-left>1)
   {
      center=(left+right)/2;
      diff=XList[center].x-x;
      if(diff==0)
      {
         if(index)
            *index=center;
         return(1);
      }
      if(diff>0)
         right=center;
      else
         left=center;
   }
   if(index)
      *index=right;
   return(0);
}

int   CheckCell(long x,long y)
{
   static struct y_list *ScanY=NULL;

   if(ScanY==NULL)
      ScanY=Field;
   while(ScanY!=NULL && ScanY->y<y)
      ScanY=ScanY->next;
   if(ScanY==NULL || ScanY->y!=y)
      return(0);
   return(FindInXList(x,ScanY->line,ScanY->x_cnt,NULL));
}

void  ChangeCell(long x,long y,int op)
{
   struct   y_list   **ScanY;
   struct   y_list   *NewYNode;
   struct   x_list   *NewXNode;
   struct   x_list   *ScanX;
   long  ind;

   ScanY=&Field;
   while(*ScanY!=NULL && (*ScanY)->y<y)
      ScanY=&((*ScanY)->next);
   if(*ScanY==NULL || (*ScanY)->y!=y)
   {
      if(op==SET || op==REV)
      {
         Population++;
         CheckPtr(NewYNode=malloc(sizeof(*NewYNode)));
         CheckPtr(NewXNode=malloc(sizeof(*NewXNode)));
         NewYNode->next=*ScanY;
         NewYNode->y=y;
         NewYNode->line=NewXNode;
         NewYNode->x_cnt=1;
         NewXNode->x=x;
         *ScanY=NewYNode;
      }
      return;
   }
   if(!FindInXList(x,(*ScanY)->line,(*ScanY)->x_cnt,&ind))
   {
      if(op==SET || op==REV)
      {
         Population++;
         (*ScanY)->x_cnt++;
         CheckPtr((*ScanY)->line=realloc((*ScanY)->line,
                                         sizeof(*NewXNode)*(*ScanY)->x_cnt));
         ScanX=(*ScanY)->line+ind;
         memmove(ScanX+1,ScanX,((*ScanY)->x_cnt-ind-1)*sizeof(*ScanX));
         ScanX->x=x;
      }
      return;
   }
   if(op==REV || op==CLR)
   {
      Population--;
      if((*ScanY)->x_cnt==1)
      {
         FreeLine((*ScanY)->line);
         NewYNode=*ScanY;
         *ScanY=(*ScanY)->next;
         free(NewYNode);
         return;
      }
      ScanX=(*ScanY)->line+ind;
      (*ScanY)->x_cnt--;
      memmove(ScanX,ScanX+1,((*ScanY)->x_cnt-ind)*sizeof(*ScanX));
      (*ScanY)->line=CheckPtr(realloc((*ScanY)->line,(*ScanY)->x_cnt*sizeof(*ScanX)));
   }
}

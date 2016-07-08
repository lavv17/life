/*____________________________________________________________________________
**
** File:          life_i.c
**
** Description:   Interactive interface for the game `LIFE'
**
** Author:        Alexander V. Lukyanov
**____________________________________________________________________________
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "life.h"
#include "inter.h"
#include "ow.h"

Window   *StatusWin;
Window   *ReadLine;
Cell     root_pal[]={{0,' '}};
Cell     status_pal[]={{REVERSE,' '}};
Cell     readline_pal[]={{REVERSE,' '},{0}};

long  ShiftX=0;
long  ShiftY=0;
long  PosX=0;
long  PosY=0;

char filename[256]="field.lif";

void  StatusLine(void)
{
   char     str[80];

   OwUse(StatusWin);

   sprintf(str,"%lu:%lu (%ld,%ld)",Generation,Population,PosX,-PosY);
   OwResize(strlen(str),1);
   OwPutString(0,0,str,UPAL);
}

void  DisplayField(void)
{
   struct   y_list   *ScanY;
   struct   x_list   *ScanX;
   static   Cell     cell={0,'*'};
   static   Cell     blank={0,' '};
   int      x,y;
   long     i;

   OwUse(Root);

   ScanY=Field;
   for(y=0; y<I_ScreenHeight; y++)
   {
      while(ScanY!=NULL && ScanY->y<y+ShiftY)
         ScanY=ScanY->next;
      if(ScanY!=NULL && ScanY->y==y+ShiftY)
      {
         ScanX=ScanY->line;
         i=ScanY->x_cnt;
         for(x=0; x<I_ScreenWidth; x++)
         {
            while(i>0 && ScanX->x<x+ShiftX)
            {
               ScanX++;
               i--;
            }
            if(i>0 && ScanX->x==x+ShiftX)
            {
               OwSetCell(x,y,&cell);
            }
            else
            {
               OwSetCell(x,y,&blank);
            }
         }
      }
      else
      {
         for(x=0; x<I_ScreenWidth; x++)
         {
            OwSetCell(x,y,&blank);
         }
      }
   }
   StatusLine();
   Sync();
}

void  Center(void)
{
   unsigned long  count;
   unsigned line_count;
   double   NewX,NewY;
   struct   x_list   *ScanX;
   struct   y_list   *ScanY;
   long  i;

   NewX=NewY=0.0;
   count=0;

   for(ScanY=Field; ScanY!=NULL; ScanY=ScanY->next)
   {
      line_count=0;
      for(ScanX=ScanY->line,i=ScanY->x_cnt; i>0; ScanX++,i--)
      {
         line_count++;
         NewX+=(double)ScanX->x;
      }
      NewY+=(double)ScanY->y*(double)line_count;
      count+=line_count;
   }
   if(count)
   {
      NewX/=(double)count;
      NewY/=(double)count;
   }

   PosX=(long)(NewX+0.5);
   PosY=(long)(NewY+0.5);
   ShiftX=PosX-(int)(I_ScreenWidth>>1);
   ShiftY=PosY-(int)(I_ScreenHeight>>1);
}

void  SaveField(void)
{
   int         key;

   OwDisplay(ReadLine);
   OwPutString(2,1,"Enter file name for saving the field",UPAL+0);
   OwDrawFrame(FULL,SINGLE,UPAL+0);

   CursorType(C_NORMAL);
   key=OwGetString(2,2,UW-4,filename,sizeof(filename)-1,UPAL+1);
   OwClose();

   if(key!=K_ESC)
   {
      FILE  *f;

      f=fopen(filename,"w");
      if(f==NULL)
      {
         Bell();
         return;
      }
      Print(f);
      fclose(f);
   }
}

void  Life(void)
{
   int   Running=0;
   int   ViewStep=10;
   unsigned key;

   OwDisplay(Root);
   OwDisplay(StatusWin);

   Center();
   DisplayField();
   do
   {
      MoveCursor((unsigned)(PosX-ShiftX),(unsigned)(PosY-ShiftY));
      CursorType(C_NORMAL);
      if(Running)
      {
         if(KeyPressed())
            key=ReadKey();
         else
            key=' ';
      }
      else
         key=ReadKey();
      switch(key)
      {
      case('!'):
         SuspendInterface();
#ifndef MSDOS
/*       system("exec $SHELL");*/
#else
         system(getenv("COMSPEC"));
#endif
         ResumeInterface();
         continue;
      case(K_F2):
         SaveField();
         continue;
      case('c'):
      case('C'):     /* clear field */
         Running=0;
         FreeField();
         Center();
         break;
      case(' '):
         Step();
         break;
      case('\t'):
         ChangeCell(PosX,PosY,REV);
         break;
      case(M_BUTTON):
         switch(I_LastButton)
         {
         case(LEFT_BUTTON):
            ChangeCell(ShiftX+I_MouseX,ShiftY+I_MouseY,REV);
            break;
         case(RIGHT_BUTTON):
            ShiftX+=I_MouseX-(int)(I_ScreenWidth>>1);
            ShiftY+=I_MouseY-(int)(I_ScreenHeight>>1);
            break;
         default:
            continue;
         }
         break;
      case(M_MOVE):
         if(I_Buttons&LEFT_BUTTON)
            ChangeCell(ShiftX+I_MouseX,ShiftY+I_MouseY,REV);
         else
            continue;
         break;
      case('4'):
         ShiftX-=ViewStep;
         break;
      case(K_LEFT):
         PosX--;
         if(PosX<ShiftX)
            ShiftX-=ViewStep;
         else
         {
            StatusLine();
            continue;
         }
         break;
      case('8'):
         ShiftY-=ViewStep;
         break;
      case(K_UP):
         PosY--;
         if(PosY<ShiftY)
            ShiftY-=ViewStep;
         else
         {
            StatusLine();
            continue;
         }
         break;
      case('6'):
         ShiftX+=ViewStep;
         break;
      case(K_RIGHT):
         PosX++;
         if(PosX>=ShiftX+(int)I_ScreenWidth)
            ShiftX+=ViewStep;
         else
         {
            StatusLine();
            continue;
         }
         break;
      case('2'):
         ShiftY+=ViewStep;
         break;
      case(K_DOWN):
         PosY++;
         if(PosY>=ShiftY+(int)I_ScreenHeight)
            ShiftY+=ViewStep;
         else
         {
            StatusLine();
            continue;
         }
         break;
      case('5'):
         Center();
         break;
      case('3'):
         ShiftY+=ViewStep;
         ShiftX+=ViewStep;
         break;
      case('1'):
         ShiftY+=ViewStep;
         ShiftX-=ViewStep;
         break;
      case('7'):
         ShiftY-=ViewStep;
         ShiftX-=ViewStep;
         break;
      case('9'):
         ShiftY-=ViewStep;
         ShiftX+=ViewStep;
         break;
      case('\r'):
         Running=!Running;
         break;
      case('q'):
      case('Q'):
      case(27):
         return;
      default:
         continue;
      }
      if(PosY<ShiftY)
         PosY+=(ShiftY-PosY+ViewStep-1)/ViewStep*ViewStep;
      if(PosX<ShiftX)
         PosX+=(ShiftX-PosX+ViewStep-1)/ViewStep*ViewStep;
      if(PosY>=ShiftY+(int)I_ScreenHeight)
         PosY-=(PosY-ShiftY-(int)I_ScreenHeight+ViewStep)/ViewStep*ViewStep;
      if(PosX>=ShiftX+(int)I_ScreenWidth)
         PosX-=(PosX-ShiftX-(int)I_ScreenWidth+ViewStep)/ViewStep*ViewStep;;
      DisplayField();
   }
   while(1);
}

int   main(int argc,char *argv[])
{
   FILE  *f;
   if(argc>=2)
   {
      f=fopen(argv[1],"r");
      if(f==NULL)
      {
         perror(argv[1]);
         return(1);
      }
      Load(f);
      fclose(f);
   }
   else
   {
      f=fopen("field.lif","r");
      if(f!=NULL)
      {
         Load(f);
         fclose(f);
      }
   }

   OpenInterface();
   OwInitialize(root_pal);
   ShowMouse(TRUE);
   SetEventMask(KEYBOARD_EVENTS|MOUSE_EVENTS);
   OwCreate(&StatusWin,0,DOWN,1,1,status_pal);
   OwCreate(&ReadLine,MIDDLE,MIDDLE+5,60,4,readline_pal);
   Life();
   OwExit();
   CloseInterface();
   return(0);
}

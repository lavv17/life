/*____________________________________________________________________________
**
** File:          life.h
**____________________________________________________________________________
*/

struct   x_list
{
   long     x;
};

struct   y_list
{
   long     y;
   long     x_cnt;
   struct   x_list   *line;
   struct   y_list   *next;
};

extern   struct   y_list   *Field;

extern   void  Step(void);
extern   void  Load(FILE*);
extern   void  Print(FILE*);
extern   void  FreeField(void);
extern   int   CheckCell(long x,long y);
extern   void  ChangeCell(long x,long y,int op);

#define  REV   0
#define  SET   1
#define  CLR   2

extern   unsigned long  Population;
extern   unsigned long  Generation;

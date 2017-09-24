//  loops

extern int N;
extern volatile int *p;
extern volatile int *q;

void ForLoop()
{
   for (int i = 0; i < N; i++)
   {
      *p++;
   }
}

void WhileLoop()
{
   while (*p != 0)
   {
      *p++;
   }
}

void DoWhileLoop()
{
   do
   {
      *p++;
   } 
   while (*p != 0);
}

void GotoLoop()
{
label:
   if (*p != 0)
   {
      *p++;
      goto label;
   }
}

void TailRecursiveLoop()
{
   if (*p != 0)
   {
      *p++;
      TailRecursiveLoop();
   }
}

void ForLoopWithContinue()
{
   for (int i = 0; i < N; i++)
   {
      *p++;

      if (*p != 0)
      {
         continue;
      }

      *q = *p;
   }
}

void ForLoopWithBreak()
{
   for (int i = 0; i < N; i++)
   {
      *p++;

      if (*p != 0)
      {
         break;
      }

      *q = *p;
   }
}

void WhileLoopWithContinue()
{
   while (*p != 0)
   {
      *p++;

      if (*q == 0)
      {
         continue;
      }

      *q = *p;
   }
}

void WhileLoopWithBreak()
{
   while (*p != 0)
   {
      *p++;

      if (*q == 0)
      {
         break;
      }

      *q = *p;
   }
}

void InfiniteWhileLoop()
{
   while(1) {}
}

void InfiniteGotoLoop()
{
loop:
   goto loop;
}

void GotoLoopWithHole()
{
   if (*q == 0)
   {
      goto inside;
   }

start:
   goto test;

inside:
   return;

test:

   *p++;

   if (*p != 0)
   {
      goto start;
   }
}

void TwoGotoLoops()
{
loop1:
   if (*p == 0)
   {
      goto loop2;
   }
   *p++;
   goto loop1;
loop2:
   if (*q == 0)
   {
      return;
   }
   *q++;
   goto loop2;
}
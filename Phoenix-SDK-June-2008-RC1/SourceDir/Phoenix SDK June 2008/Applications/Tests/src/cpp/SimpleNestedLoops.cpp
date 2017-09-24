// Simple nested loops

extern int N;
extern volatile int *p;
extern volatile int *q;

void NestedForLoop()
{
   for (int i = 0; i < N; i++)
   {
      for (int j = 0; j < N; j++)
      {
         *p++;
      }
   }
}

void NestedForLoop2()
{
   for (int i = 0; i < N; i++)
   {
      for (int j = 0; j < N; j++)
      {
         for (int k = 0; k < N; k++)
         {
            *p++;
         }
      }
   }
}

void NestedForLoop3()
{
   for (int i = 0; i < N; i++)
   {
      for (int j = 0; j < N; j++)
      {
         *q++;
      }

      for (int k = 0; k < N; k++)
      {
         *p++;
      }
   }
}

void NestedWhileLoop()
{
   while (*p != 0)
   {
      while (*q != 0)
      {
         *p++ = *q++;
      }
   }
}

void NestedForWhileLoop()
{
   for (int i = 0; i < N; i++)
   {
      while (*q != 0)
      {
         *p++ = *q++;
      }
   }
}

void NestedWhileForLoop()
{
   while (*q != 0)
   {
      for (int i = 0; i < N; i++)
      {
         *p++ = *q++;
      }
   }
}

void NestedLoopWithTwoLevelExit()
{
   for (int i = 0; i < N; i++)
   {
      for (int j = 0; j < N; j++)
      {
         if (*p == 0)
         {
            goto done;
         }

         *p++;
      }
   }

done:
   return;
}

void NonNestedWhileLoops()
{
   while (*p++)
   {
      N++;
   }
   while (*q++)
   {
      N++;
   }
}



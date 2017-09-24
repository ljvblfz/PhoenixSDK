extern int N;
extern volatile int *p;
extern volatile int *q;

void puzzle()
{
   if (*p)
   {
      goto L1;
   }
   else
   {
      goto L2;
   }

L1:
    N++;
    goto L2;

L2:
   if (*q++ != 0)
   {
      goto L1;
   }
}

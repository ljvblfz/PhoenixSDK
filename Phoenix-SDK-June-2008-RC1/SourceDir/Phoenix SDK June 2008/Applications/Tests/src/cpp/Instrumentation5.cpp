#include <cstdio>

int g = 0;

int main()
{
   unsigned int i = 0;

loop:

   if (i >= 10)
   {
      goto done;
   }

   i++;
   g++;

   goto loop;

done:

   printf("g is %d\n", g);
}
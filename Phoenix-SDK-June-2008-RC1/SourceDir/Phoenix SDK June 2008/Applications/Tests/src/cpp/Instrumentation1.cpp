#include <cstdio>

int g = 0;

int main()
{
   unsigned int i = 0;

   while (i++ < 100)
   {
      g++;
   }

   printf("g is %d\n", g);
}
#include <cstdio>

int g = 0;

int main()
{
   unsigned int i = 0;

   while (i++ < 10)
   {
      for (unsigned int j = 0; j < 10; j++)
      {
         g++;
      }
   }

   printf("g is %d\n", g);
}
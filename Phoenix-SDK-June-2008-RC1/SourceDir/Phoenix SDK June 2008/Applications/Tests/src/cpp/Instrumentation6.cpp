#include <cstdio>

int g = 0;

int main()
{
   for (unsigned int i = 0; i < 10; i++)
   {
      for (unsigned int j = i; j < 10; j++)
      {
         g++;
      }
   }

   printf("g is %d\n", g);
}
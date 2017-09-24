#include <cstdio>

int g = 0;

int main()
{
   for (unsigned int i = 0; i < 100; i++)
   {
      g++;
   }

   printf("g is %d\n", g);
}
#include <cstdio>

int g = 0;

void f()
{
   unsigned int i = 0;

   while (i++ < 10)
   {
      g++;
   }
}

int main()
{
   unsigned int i = 0;

   while (i++ < 10)
   {
      f();
   }

   printf("g is %d\n", g);
}
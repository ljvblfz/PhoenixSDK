#include <cstdio>

int g = 0;

void b()
{
   unsigned int i = 0;

   while (i++ < 3)
   {
      g++;
   }
}

void a()
{
   unsigned int i = 0;

   while (i++ < 3)
   {
      b();
   }
}

int main()
{
   unsigned int i = 0;

   while (i++ < 3)
   {
      a();
   }

   printf("g is %d\n", g);
}
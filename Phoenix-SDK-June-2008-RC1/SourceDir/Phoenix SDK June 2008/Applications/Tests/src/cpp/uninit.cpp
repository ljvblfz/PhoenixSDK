//------------------------------------------------------------------
//
// A simple program with many uninitialized uses of local variables
//
// w is fine
// x is used twice without being initialized
// y is used once without being initialized, then once after being initialized
// z is sometimes used without being initialized
//
//------------------------------------------------------------------

#include <stdio.h>

int
main
(
   int argc,
   char **argv
)
{
   int w, x, y, z;

   // We define w, then use it -- no problem here

   w = 1;
   printf("w = %d\n", w);

   // This is an uninitialized use of x

   printf("x = %d\n", x);

   // This is an uninitialized use of y

   printf("y = %d\n", y);

   y = 0;

   // This is an initialized use of y

   printf("y = %d\n", y);

   if (argc > 1)
   {
      z = 0;
   }

   // Since z is only initialized in certain cases, this is a
   // possibly uninitialized use of z

   printf("z = %d\n", z);

   return 0;
}

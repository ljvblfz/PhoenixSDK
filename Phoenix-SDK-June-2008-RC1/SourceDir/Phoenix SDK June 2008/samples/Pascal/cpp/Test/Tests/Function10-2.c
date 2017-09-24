// Tests function/procedure invocations.

#include <stdio.h>

extern int __cdecl e(int x, int y, int z);
extern void __cdecl runtime_init();
extern void __cdecl runtime_exit();

int __cdecl main()
{
   runtime_init();
   printf("%d\r\n", e(1,2,3));
   runtime_exit();
   return 0;
}
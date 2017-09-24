#include <cstdio>

int g = 0;

class S
{
public:

   static const int N = 10;

   S()
   {
      for (unsigned int i = 0; i < N; i++)
      {
         g++;
      }
   }

   ~S()
   {
      for (unsigned int i = 0; i < N; i++)
      {
         g++;
      }
   }
};

int main()
{
   S s;
   
   printf("g is %d\n", g);
}
#include "base.h"

class Derived: public Base {
public:
   Derived(const char * msg) : Base(msg) {}
   virtual void Hello() { printf("%s from Derived\n", this->GetMessage()); }
};

int main()
{
   Base * base1 = new Base("Hello");
   Base * base2 = new Derived("Hello");
   base1->Hello();
   base2->Hello();
}

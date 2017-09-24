#include <cstdio>

class Base {
public:
   Base(const char * msg) : message(msg) {}
   virtual void Hello() { printf("%s from Base\n", message); }
   const char * GetMessage() { return message; }
private:
   const char * message;
};

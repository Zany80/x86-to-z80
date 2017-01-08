#include <cstdint>
#include <functional>

#define inf_int uint64_t

namespace {

// todo:
// - string literals 
// - export c++ methods at fixed address?
// - 

void call(const uint16_t address)
{
  auto func = (void (*)(void))address;
  func();
}

void bdos()
{
  //call(0x5); // assembled as "movl $5, %eax; calll *%eax"'
  asm("call 0x5;");
}

void printf(const char* test, int a, int b, int c)
{
  volatile const char* x = test;
  volatile int a1 = a;
  volatile int b1 = b;
  volatile int c1 = c;
}

void example1()
{
  int a = 10, b;
  //asm("movl %1, %%eax; 
  //  movl %%eax, % 0; "
  //  :"=r"(b)        /* output */
  //  : "r"(a)         /* input */
  //  : "%eax"         /* clobbered register */
  //);
}

struct Z80
{
  volatile uint8_t& memory(const uint16_t address)
  {
    return *reinterpret_cast<uint8_t*>(address);
  }

  // http://wiki.osdev.org/Inline_Assembly/Examples
  // https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#Extended-Asm
  // clang is pretty much compatible with gcc as far as asm() is concerned.

  void writeStdOut(const char* text)
  {
    register const char* textAddres asm("dx") = text; // force it into 'edx'
    asm("                                  \
      movb 0x2, %%cl; # Console output  \n \
      calll 0x5; # bdos                 \n \
      " :: "r"(textAddres) : "%eax");
  }
};
}

int main()
{
  Z80 z80;

  auto useFreeLambda = [&]() {
    //z80.memory(0xa000) = 0x10;
    z80.writeStdOut("dit is een test"); // this goes into the .asciz section
  };
  useFreeLambda();
  asm("# This is a comment inserted in an ASM block");
  return 2;
}
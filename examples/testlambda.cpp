#include <cstdint>
#include <functional>

#define inf_int uint64_t

namespace {


void call(const uint16_t address)
{
  auto func = (void (*)(void))address;
  func();
}

void bdos()
{
  call(0x5);
}

void printf(const char* test, int a, int b, int c)
{
  volatile const char* x = test;
  volatile int a1 = a;
  volatile int b1 = b;
  volatile int c1 = c;
}

struct Z80
{
  volatile uint8_t& memory(const uint16_t address)
  {
    return *reinterpret_cast<uint8_t*>(address);
  }

  void writeStdOut(const char* text)
  {
    uint16_t textAddress = reinterpret_cast<uint16_t>(text);
    asm("movb $6, bl");
    asm("movl textAddress, dx");
    bdos();

    //volatile int t = 0;
    //++t;
    //uint64_t testvar = 0;
    //__asm__( "addq $1, %0 \n\t" : "+r" (testvar));
  }
};
}

int main()
{
  Z80 z80;

  auto writeMemory = [&]() {
    z80.memory(0xa000) = 0x10;
  };
  writeMemory();

  z80.writeStdOut("dit is een test");

  //test();
  return 0;
}
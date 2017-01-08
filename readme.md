# About

Attempts to translate x86 assembly into z80 assembly. The idea is to make C++ available to Z80 based computers like the MSX.
The process works by using the clang (LLVM 3.9.1) compiler to convert C++ to x86 assembly, and then re-assembling this output into z80-opcodes.

# References

- http://releases.llvm.org/download.html
- http://www.nowind.nl
- using Visual Studio 2017 RC1 with clang-support

# Why?

I want to use C++17 to futher nowind development (www.nowind.nl)
Also it is a way for me to learn what it takes to create a re-assembler for a platform where no modern compilers are available.
I think this can have very cool application in modern, small, low-cost, embedded processesors where vendors do not necessarily take the effort to maintain their own toolchain.

# Example

```c++
struct Z80
{
  volatile uint8_t& memory(const uint16_t address)
  {
    return *reinterpret_cast<uint8_t*>(address);
  }
};

int main()
{
  Z80 z80;

  auto useFreeLambda = [&]() {
    z80.memory(0xa000) = 0x10;
  };

  useFreeLambda();
  return 0;
}
```

Is re-assembled as:
```assembly
        ld a,16       ; movb    $16, 40960
        ld (40960),a  ; movb    $16, 40960
        xor a         ; xorl    %eax, %eax
        ret           ; retl
```        




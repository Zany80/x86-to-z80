# About

Attempt to translate x86 assembly into z80 assembly. The idea is to make C++ available to Z80 based computers like the MSX.
The process works by using the clang (LLVM 3.9.1) compiler to convert C++ to x86 assembly and then re-assembling this output into z80-assembly by a pretty static 1:N translation

The project is forked from lefticus/x86-to-6502 and I intend to keep to support for the 6502, so even though I named the project x86-to-z80, it could be x86-to-MultipleCPUs later (or preferably a better name).

Discuss or follow on twitter, if you like:
https://twitter.com/janwilmans

# References

- http://releases.llvm.org/download.html
- http://www.nowind.nl
- using Visual Studio 2017 RC1 with clang-support
- https://www.visualstudio.com/vs/visual-studio-2017-rc/
- http://www.myquest.nl/z80undocumented/z80-documented-v0.91.pdf

# Why?

I want to use C++17 to futher nowind development (www.nowind.nl)
Also it is a way for me to learn what it takes to create a re-assembler for a platform where no modern compilers are available.
I think this can have very cool application in modern, small, low-cost, embedded processesors where vendors do not necessarily take the effort to maintain their own toolchain.

# How?

```
cd %~dp0
:: appearently clang needs visual studio headers to compile on windows, 
:: which doesn't make much sense to me, but it works.
call "c:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\vsvars32.bat"
C:\project\LLVM\bin\clang++.exe %1 -O3 -S

:: http://clang.llvm.org/get_started.html, consider using:
:: -fomit-frame-pointer
:: -emit-llvm  (maybe less optimized?)
```

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

# Progress?

I'm currently exploring different options. Some thoughts:

- is i386 to z80 assembly an relatively easy conversion? or is, let's say a MIPS to z80 conversion easier (suggested since MIPS CPUs have fewer registers and _appearently_ no flag-registers which sounds like a plus for translation). However, that could also mean that the code would get optimized for memory-access operations instead of using registers. 
- Going further along those lines, would it not be better to just have an LLVM backend for the z80 ? That way the code gets optimized of the amount of registers that the z80 actually HAS :)
- I'm currently reading up on how a LLVM backend is constructed at http://jonathan2251.github.io/lbd/llvmstructure.html

My conclusion so far: the i386 to z80 path seems viable and both assembly languages are somewhat familiar to me, so instead of learning how to code backends or learning a new assembly language first, I will continue on this path for now.

# Maintaining the i386 state on the z80

The i386 consists of:

```
Registers: EAX, EBX, ECX, EDX, ESI, EDI, EBP and ESP  (8x4=32 bytes)
Segment registers: CS, SS, DS, ES, FS, GS (6x2=12 bytes) 
More registers: EFLAGS 32bits (Zero, Carry, Overflow, etc.) and EIP (instruction pointer)
```
A x86 instruction can change any of these registers (that I will refer to collectively as 'the i386 state').

The i386 state will have to be maintained/available on, the z80 target. This is because a subsequent instruction's exection/result can be dependend on this state.

Since the z80 has only:
```
Registers: AF, BC, DE, HL, IX, IY
More register: SP (stack pointer), IP (instruction pointer) 
```
Memory (48 bytes) will have to be used to store this i386 state.
This memory can be allocated from the stack, but perhaps static allocation at the start of the 0x100 area can offer better optimization opportunities.

When using the stack the most strait-forward way to update values would be:
```
LD A,5         ; 2-byte operation, 7 T-states
LD (IX+n), A   ; IX and IY allow indexing, where n is the offset in bytes of the register relative to IX pointing to the memory, 20 T-states
```
A total of 6 bytes and 27 T-states.

The downside of this is that LD (IX+n),R is a relatively slow operation.

Alternatively:
```
LD A,5       ; same 2 bytes, 7 T-states  
LD (nn), A   ; 3 bytes, where nn is an absolute memory address, a 13 T-state operation
```
A total of 5 bytes and 20 T-states.

This saves 1 byte and 6 T-states on every register load/save, and moreover, it does only changes the A-register.

# Z80 Process Setup / Memory Layout

The z80 can run MSXDOS 2.xx (developed by Microsoft and Spectravideo) which is a CP/M derivate.
(MSX actually stands for MicroSoft eXtened, or at least, I like to think so :)

MSXDOS uses a fixed memory layout that looks like this (roughly):
```
0-0xFF = reserved area (zero page) for MSXDOS (https://en.wikipedia.org/wiki/MSX-DOS)
0x100-0xdffff (~56Kb) = available for program
0xe000-0xefff = area for stack growing down, not actually a defined area, but I keep out of this area to allow for a 4kB stack for now.
0xf000-0xffff = reserved area for MSX BIOS and MSX BASIC (settings, screen modes, shadow memory for VDP etc.)
```
A program is linked as a .COM file that is exactly like a MS-DOS .com file in that it has no header, no relocation information just code+data, all in one segment, limited to, theoretically, 65KB-256 bytes. However, since we need to repect the 'other stuff' mentioned above, the filesize limit is somewhat smaller even.

Does this mean a program can only be 56kb? Initially: yes, but once started, more data/code can be loaded from other files. Generally 128kB memory is available at least (switchable in 16kB segments called 'banks') and the test-machine on my desk has 4Mb RAM available (256 banks of 16kB).

The .com file is always completely loaded starting at address 0x100.

So this is the environment to link to....

# Awkward

Binding c++ methods to Bios functions is bit awkward, the problem:

The method needs to call a fixed memory address like so:

```
LD   DE, "some text"
LD   C,9    ; command '9' is 'output string to console' / sortof like std::cout 
CALL 0x5    ; call BDOS (a fixed hook address to invoke a bios-function)
```

One option is to create the method in assembly and link to it, hard-coding it against the x86 ABI (https://en.wikipedia.org/wiki/X86_calling_conventions).

Second option is also awkward, but somewhat cleaner (maybe):

```
  void writeStdOut(const char* text)
  {
    register const char* textAddres asm("dx") = text; // force it into 'dx'
    asm("                                  \
      movb 0x9, %%cl; # Console output  \n \
      calll 0x5; # bdos                 \n \
      " :: "r"(textAddres) : "%eax");
  }
```

Basically what I'm doing here is hardcoding against my own re-assembler, using the knowlegde of how CL will map to 'C' and DX will map to 'DE'. This is writing z80 code in i386 syntax :)

The second method has the advantage of giving the compiler knowlegde about what registers are affected, so might offer better optimization opportunities.


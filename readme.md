# About

Attempts to translate x86 assembly into z80 assembly. The idea is to make C++ available to Z80 based computers like the MSX.
The process works by using the clang (LLVM 3.9.1) compiler to convert C++ to x86 assembly, and then re-assembling this output into z80-assembly by a pretty static 1:N translation

Discuss or follow on twitter, if you like:
https://twitter.com/janwilmans

# References

- http://releases.llvm.org/download.html
- http://www.nowind.nl
- using Visual Studio 2017 RC1 with clang-support
- https://www.visualstudio.com/vs/visual-studio-2017-rc/

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

- is i386 to z80 assembly an relatively easy conversion? or is, let's say a MIPS to z80 conversion easier (suggested since MIPS CPUs have fewer registers, _appearently_ no flag-registers with sounds like a plus for translation). However, that could also mean that the code would get optimized for memory-access operations instead of using registers. 
- Going further along those lines, would it not be better to just have an LLVM backend for the z80 ? That way the code gets optimized of the amount of registers that the z80 actually HAS :)
- I'm currently reading up on how a LLVM backend is constructed at http://jonathan2251.github.io/lbd/llvmstructure.html

My conclusion so far: the i386 to z80 path seems viable and both assembly languages are somewhat familiar to me, so instead of learning how to code backends or learning a new assembly language first, I will continue on this path for now.

# Z80 Process Setup / Memory Layout

The z80 can run MSXDOS 2.xx (developed by Microsoft and Spectravideo) which is a CP/M derivate.
(MSX actually stands for MicroSoft eXtened, or at least, I like to think so :)

MSXDOS uses a fixed memory layout that looks like this (roughly):

0-0xFF = reserved area (zero page) for MSXDOS (https://en.wikipedia.org/wiki/MSX-DOS)
0x100-0xdffff (~56Kb) = available for program
0xe000-0xefff = area for stack growing down, not actually a defined area, but I keep out of this area to allow for a 4kB stack for now.
0xf000-0xffff = reserved area for MSX BIOS and MSX BASIC (settings, screen modes, shadow memory for VDP etc.)

A program is linked as a .COM file that is exactly like a MS-DOS .com file in that it has no header, no relocation information just code+data, all in one segment, limited to, theoretically, 65KB-256 bytes. However, since we need to repect the 'other stuff' mentioned above, the filesize limit is somewhat smaller even.

Does this mean a program can only be 56kb? Initially: yes, but once started, more data/code can be loaded from other files. Generally 128kB memory is available at least (switchable in 16kB segments called 'banks') and the test-machine on my desk has 4Mb RAM available (256 banks of 16kB).

The .com file is always completely loaded starting at address 0x100.

So this is the environment to link to....















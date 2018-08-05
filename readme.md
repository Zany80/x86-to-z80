# About

This attempts to cross-compile C++ to the Z80 for Zany80 by first generating x86 assembly, and then transforming the x86 into z80 assembly.

This is forked from janwilmans/x86-to-z80, where just about *nothing* works. This fork actually *functions*, even if it isn't yet complete, 

# Example

```cpp
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0
#define VERSION_TWEAK 0

#define _stringify(x) #x
#define stringify(x) _stringify(x)

#define VERSION_STRING stringify(VERSION_MAJOR) stringify(VERSION_MINOR)

struct Z80 {
	volatile uint8_t& memory(const uint16_t address) {
		return *reinterpret_cast<uint8_t*>(address);
	}
};

int a(const char *str, int i);

int a(const char *str) {
	int i;
	return a(str, i);
}

int main() {
	Z80 z80;
	z80.memory(6) = 0;
	const char *vstr = VERSION_STRING;
	return a(vstr);
}

```

Is re-assembled as:
```assembly
_Z1aPKc:
	unknown       ; subl	$12, %esp
	ld a, (ix + 8)  ; movl	16(%esp), %eax
	ld iyl, a     ; movl	%eax, (%esp)
	ld iyh, 0     ; movl	%eax, (%esp)
	ex (sp), iy   ; movl	%eax, (%esp)
	call _Z1aPKci  ; calll	_Z1aPKci
	unknown       ; addl	$12, %esp
	ret           ; retl
main:
	unknown       ; subl	$12, %esp
	ld a, 0       ; movb	$0, 6
	ld (6), a     ; movb	$0, 6
	ld iy, $.L.str  ; movl	$.L.str, (%esp)
	ex (sp), iy   ; movl	$.L.str, (%esp)
	call _Z1aPKci  ; calll	_Z1aPKci
	unknown       ; addl	$12, %esp
	ret           ; retl

```

# The stack

The stack pointer on the z80, like all pointers, is a 16-bit pointer to 8-bit data.
The i386 stack is a bit different, so all modifications to the stack have to be translated.

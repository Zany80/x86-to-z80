#include <cstdint>

namespace 
{

struct Z80
{
	volatile uint8_t& memory(const uint16_t address)
	{
		return *reinterpret_cast<uint8_t *>(address);
	}
};

}

int main()
{
	Z80 z80;
	
	auto test = [&] () {
		z80.memory(0xa000) = 0x10;
	};
	
	test();
	return 0;
}
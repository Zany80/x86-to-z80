#include "z80.h"
#include "i386.h"

#include <vector>
#include <iostream>
#include <regex>

int parse_8bit_literal(const std::string& s) {
	return std::stoi(std::string(std::next(std::begin(s)), std::end(s)));
}

static Operand translateLiteral(const Operand reg) {
	// Number
	if (reg.value[0] == '$') {
		if (isdigit(reg.value[1])) {
			return Operand(Operand::Type::literal, std::to_string(parse_8bit_literal(reg.value)));
		}
		// Probably a label
		else {
			log(LogLevel::Warning, "Unable to convert operand \"" + reg.value + "\" to literal!");
			return reg;
		}
	}
	std::smatch match;
	if (std::regex_match(reg.value, match, std::regex(R"((\d+)\(%esp\))"))) {
		// This is a stack index (e.g. "16(%esp)"). Convert this to a z80 index.
		int index = std::stoi(match[1]) / 2;
		return Operand(Operand::Type::literal, "(ix + " + std::to_string(index) + ")");
	}
	return reg;
}

static Operand translateAddress(const Operand reg) {
	return Operand(Operand::Type::literal, "(" + reg.value + ")");
}

typedef struct {
	Operand a = Operand(Operand::Type::literal, "a");
	Operand iy = Operand(Operand::Type::literal, "iy");
	Operand iyh = Operand(Operand::Type::literal, "iyh");
	Operand iyl = Operand(Operand::Type::literal, "iyl");
} _registers;
_registers registers;

static Operand translateRegister(const Operand reg) {
	switch (reg.reg_num) {
		case 0x10:
			return registers.a;
	}
	return reg;
}

Operand translateValue(Operand op) {
	if (op.type == Operand::Type::reg) {
		return translateRegister(op);
	}
	else if (op.type == Operand::Type::literal) {
		
	}
	return op;
}

void translate_instruction(std::vector<z80>& instructions, const i386::OpCode op, const Operand o1, const Operand o2) {
	switch (op) {
		case i386::OpCode::xorl:
			if (o2 != registers.a) {
				log(LogLevel::Error, "Attempt to xor a register other than a!");
			}
			instructions.emplace_back(z80(z80::OpCode::_xor, translateValue(o2), translateValue(o1)));
			break;
		case i386::OpCode::movl:
		case i386::OpCode::movb:
			if (o2 == Operand(Operand::Type::literal, "(%esp)")) {
				if (translateValue(o1) == registers.a) {
					instructions.emplace_back(z80::OpCode::ld, registers.iyl, registers.a);
					instructions.emplace_back(z80::OpCode::ld, registers.iyh, Operand(Operand::Type::literal, "0"));
				}
				else {
					instructions.emplace_back(z80::OpCode::ld, registers.iy, translateValue(o1));
				}
				instructions.emplace_back(z80::OpCode::ex, Operand(Operand::Type::literal, "(sp)"), registers.iy);
			}
			else {
				if (o1 != Operand(Operand::Type::reg, 0x10)) {
					instructions.emplace_back(z80(z80::OpCode::ld, registers.a, translateLiteral(o1)));
				}
				if (o2 != Operand(Operand::Type::reg, 0x10)) {
					instructions.emplace_back(z80(z80::OpCode::ld, translateAddress(o2), registers.a));
				}
			}
			break;
		case i386::OpCode::calll:
			instructions.emplace_back(z80(z80::OpCode::call, o1, o2));
			break;
		case i386::OpCode::ret:
		case i386::OpCode::retl:
			instructions.emplace_back(z80::OpCode::ret);
			break;
		default:
			instructions.emplace_back(z80::OpCode::unknown);
			break;
	}
}

void to_z80(const i386& i, std::vector<z80>& instructions) {
	try {
		switch (i.type) {
			case ASMLine::Type::Label:
				instructions.emplace_back(i.type, i.text);
				return;
			case ASMLine::Type::Directive:
				//instructions.emplace_back(i.type, i.text);
				return;
			case ASMLine::Type::Instruction:
				const auto head = instructions.size();
				translate_instruction(instructions, i.opcode, i.operand1, i.operand2);

				auto text = i.line_text;
				if (text[0] == '\t') {
					text.erase(0, 1);
				}
				for_each(std::next(instructions.begin(), head), instructions.end(),
						[text](auto& ins) {
							ins.comment = text;
						}
				);
				return;
		}
	}
	catch (const std::exception& e) {
		log(LogLevel::Error, i, e.what());
	}
}

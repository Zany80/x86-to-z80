#include "z80.h"
#include "i386.h"

#include <vector>
#include <iostream>
#include <regex>

int parse_8bit_literal(const std::string& s) {
	return std::stoi(std::string(std::next(std::begin(s)), std::end(s)));
}

Operand literal(int i) {
	return Operand(Operand::Type::literal, std::to_string(i));
}

Operand literal(std::string s) {
	return Operand(Operand::Type::literal, s);
}

static Operand translateLiteral(const Operand reg) {
	// Number
	if (reg.value[0] == '$') {
		if (isdigit(reg.value[1])) {
			return literal(parse_8bit_literal(reg.value));
		}
	}
	if (reg.value[0] == '.' || (reg.value[0] =='$' && reg.value[1] == '.')) {
		std::string new_string;
		if (reg.value[0] == '$')
			new_string = std::string(std::next(reg.value.begin()), reg.value.end());
		else
			new_string = std::string(reg.value.begin(), reg.value.end());
		std::size_t pos;
		while ((pos = new_string.find(".")) != std::string::npos) {
			new_string.replace(pos, 1, "period_");
		}
		return literal(new_string);
	}
	std::smatch match;
	if (std::regex_match(reg.value, match, std::regex(R"((\d+)\(%esp\))"))) {
		// This is a stack index (e.g. "16(%esp)"). Convert this to a z80 index.
		int index = std::stoi(match[1]) / 2;
		return literal("(ix + " + std::to_string(index) + ")");
	}
	return reg;
}

static Operand translateRegister(const Operand reg) {
	switch (reg.reg_num) {
		case 0x10:
			return registers.a;
		case 0x11:
			return registers.b;
		case 0x14:
			return registers.bc;
		case 0x15:
			return registers.de;
		case 0x16:
			return registers.sp;
		case 0x17:
			return registers.iy;
	}
	return reg;
}

static Operand translateAddress(const Operand reg) {
	if (reg.type == Operand::Type::literal)
		return literal("(" + reg.value + ")");
	else if (reg.type == Operand::Type::reg)
		return translateRegister(reg);
	return reg;
}

Operand translateValue(Operand op) {
	if (op.type == Operand::Type::reg) {
		return translateRegister(op);
	}
	else if (op.type == Operand::Type::literal) {
		return translateLiteral(op);
	}
	return op;
}

void translate_instruction(std::vector<z80>& instructions, const i386::OpCode op, const Operand o1, const Operand o2) {
	switch (op) {
		case i386::OpCode::pushl:
			if (translateValue(o1) == registers.a) {
				instructions.emplace_back(z80(z80::OpCode::ld, registers.iyh, registers.a));
				instructions.emplace_back(z80(z80::OpCode::ld, registers.iyl, literal(0)));
				instructions.emplace_back(z80(z80::OpCode::push, registers.iy, Operand()));
			}
			else if(translateValue(o1) == registers.de || translateValue(o1) == registers.bc || translateValue(o1) == registers.hl) {
				instructions.emplace_back(z80(z80::OpCode::push, translateValue(o1), Operand()));
			}
			else {
				instructions.emplace_back(z80(z80::OpCode::ld, registers.iy, translateValue(o1)));
				instructions.emplace_back(z80(z80::OpCode::push, registers.iy, Operand()));
			}
			instructions.emplace_back(z80(z80::OpCode::inc, registers.sp, Operand()));
			instructions.emplace_back(z80(z80::OpCode::dec, registers.ix, Operand()));
			break;
		case i386::OpCode::popl:
			instructions.emplace_back(z80(z80::OpCode::dec, registers.sp, Operand()));
			instructions.emplace_back(z80(z80::OpCode::inc, registers.ix, Operand()));
			if (translateValue(o1) == registers.a) {
				instructions.emplace_back(z80(z80::OpCode::pop, registers.iy, Operand()));
				instructions.emplace_back(z80(z80::OpCode::ld, registers.a, registers.iyh));
			}
			else if(translateValue(o1) == registers.de || translateValue(o1) == registers.bc || translateValue(o1) == registers.hl) {
				instructions.emplace_back(z80(z80::OpCode::pop, translateValue(o1), Operand()));
			}
			else {
				instructions.emplace_back(z80(z80::OpCode::pop, registers.iy, Operand()));
				instructions.emplace_back(z80(z80::OpCode::ld, translateValue(o1),registers.iy));
			}
			break;
		case i386::OpCode::decl:
			instructions.emplace_back(z80(z80::OpCode::dec, translateValue(o1), Operand()));
			instructions.emplace_back(z80(z80::OpCode::cp, literal(0), Operand()));
			break;
		case i386::OpCode::incl:
			instructions.emplace_back(z80(z80::OpCode::inc, translateValue(o1), Operand()));
			instructions.emplace_back(z80(z80::OpCode::cp, literal(0), Operand()));
			break;
		case i386::OpCode::jne:
			instructions.emplace_back(z80(z80::OpCode::jp, literal("nz"), translateLiteral(o1)));
			break;
		case i386::OpCode::subl:
			if (translateValue(o2) == registers.sp) {
				instructions.emplace_back(z80(z80::OpCode::ld, registers.ix, literal("-" + translateLiteral(o1).value)));
				instructions.emplace_back(z80(z80::OpCode::add, registers.ix, registers.sp));
				instructions.emplace_back(z80(z80::OpCode::ld, registers.sp, registers.ix));
			}
			else {
				instructions.emplace_back(z80(z80::OpCode::sub, translateValue(o2), translateLiteral(o1)));
			}
			break;
		case i386::OpCode::addl:
			if (translateValue(o2) == registers.sp) {
				instructions.emplace_back(z80(z80::OpCode::ld, registers.ix, literal(translateLiteral(o1).value)));
				instructions.emplace_back(z80(z80::OpCode::add, registers.ix, registers.sp));
				instructions.emplace_back(z80(z80::OpCode::ld, registers.sp, registers.ix));
			}
			else {
				instructions.emplace_back(z80(z80::OpCode::add, translateValue(o2), translateLiteral(o1)));
			}
			break;
		case i386::OpCode::xorl:
			if (translateRegister(o2) != registers.a) {
				if (translateRegister(o2) == translateRegister(o1)) {
					// Clear the register
					instructions.emplace_back(z80(z80::OpCode::ld, translateRegister(o1),literal(0)));
				}
				else {
					log(LogLevel::Error, "Attempt to xor a register other than a!" + translateRegister(o2).value);
				}
			}
			else {
				instructions.emplace_back(z80(z80::OpCode::_xor, translateValue(o2), translateValue(o1)));
			}
			break;
		case i386::OpCode::movzwl:
			instructions.emplace_back(z80::OpCode::ld, registers.iy, translateLiteral(o1));
			break;
		case i386::OpCode::movw:
			instructions.emplace_back(z80::OpCode::ld, translateAddress(o2), translateValue(o1));
			break;
		case i386::OpCode::movl:
		case i386::OpCode::movb:
			if (o2 == literal("(%esp)")) {
				instructions.emplace_back(z80::OpCode::ld, registers.iyl, translateValue(o1));
				instructions.emplace_back(z80::OpCode::ld, registers.iyh, literal(0));
				instructions.emplace_back(z80::OpCode::ex, literal("(sp)"), registers.iy);
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
		case i386::OpCode::callw:
			instructions.emplace_back(z80(z80::OpCode::call, o1, o2));
			break;
		case i386::OpCode::ret:
		case i386::OpCode::retl:
			instructions.emplace_back(z80::OpCode::ret);
			break;
		case i386::OpCode::hlt:
			instructions.emplace_back(z80::OpCode::halt);
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
			case ASMLine::Type::Comment:
				instructions.emplace_back(i.type, i.text);
				return;
			case ASMLine::Type::Directive:
				{
					std::smatch match;
					if (std::regex_match(i.text, match, std::regex(R"(\.asciz\s(.*))"))) {
						instructions.emplace_back(i.type, ".asciiz " + std::string(match[1]));
					}
				}
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

_registers registers;

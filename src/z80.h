#pragma once

#include "types.h"
#include <algorithm>
#include <vector>
#include <stdexcept>

struct z80 : ASMLine {
	enum class OpCode {
		unknown,
		halt,
		ret,
		_xor,
		ld,
		jp,
		call,
		ex,
		dec,
		inc,
		add,
		sub,
		cp,
		push,
		pop,
	};

	z80(const Type t, std::string s)
			: ASMLine(t, std::move(s))
			{}

	static std::string to_string(const OpCode o) {
		switch (o) {
			case OpCode::_xor:
				return "xor";
			case OpCode::ld:
				return "ld";
			case OpCode::call:
				return "call";
			case OpCode::ret:
				return "ret";
			case OpCode::ex:
				return "ex";
			case OpCode::jp:
				return "jp";
			case OpCode::halt:
				return "halt";
			case OpCode::dec:
				return "dec";
			case OpCode::inc:
				return "inc";
			case OpCode::cp:
				return "cp";
			case OpCode::add:
				return "add";
			case OpCode::sub:
				return "sub";
			case OpCode::push:
				return "push";
			case OpCode::pop:
				return "pop";
			default:
				return "unknown";
		}
		return "unknown";
	}

	explicit z80(const OpCode o)
			: ASMLine(Type::Instruction, to_string(o))
			, opcode(o)
	// , is_branch(get_is_branch(o))
	// , is_comparison(get_is_comparison(o))
	{}

	z80(const OpCode o, const Operand o1, const Operand o2)
	: ASMLine(Type::Instruction, to_string(o))
		, opcode(o), op1(o1), op2(o2)
	{}


	std::string to_string() const {
		switch (type) {
			case ASMLine::Type::Label:
			case ASMLine::Type::Comment:
				return text; // + ':';
			case ASMLine::Type::Directive:
			case ASMLine::Type::Instruction:
				{
					std::string line;
					if (op1.value.size()) {
						if (op2.value.size())
							line = '\t' + text + ' ' +op1.value + ", " + op2.value;
						else
							line = '\t' + text + ' ' +op1.value;
					}
					else {
						line = '\t' + text;
					}
					line = line + ' ';
					return line + std::string(static_cast<size_t>(std::max(15 - static_cast<int>(line.size()), 1)), ' ') + "; " + comment;
				}
		}
		throw std::runtime_error("Unable to render: " + text);
	}

	OpCode opcode;
	Operand op1, op2;
	std::string comment;
};

struct i386;

void to_z80(const i386& i, std::vector<z80>& instructions);

Operand literal(int i);
Operand literal(std::string s);

typedef struct {
	Operand a = literal("a");
	Operand b = literal("b");
	Operand iyl = literal("iyl");
	Operand iyh = literal("iyh");
	Operand iy = literal("iy");
	Operand ixl = literal("ixl");
	Operand ixh = literal("ixh");
	Operand ix = literal("ix");
	Operand sp = literal("sp");
	Operand bc = literal("bc");
	Operand de = literal("de");
	Operand hl = literal("hl");
} _registers;
extern _registers registers;

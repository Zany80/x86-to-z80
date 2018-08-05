#pragma once

#include <string>
#include <cassert>

enum class LogLevel {
	Warning,
	Error
};

enum class Target {
	Z80
};

struct ASMLine {
	enum class Type {
		Label,
		Instruction,
		Directive,
		Comment
	};

	ASMLine(Type t, std::string te)
			: type(t)
			, text(std::move(te))
	{}

	Type type;
	std::string text;
};

struct Operand {
	enum class Type {
		empty,
		literal,
		reg /*ister*/
	};

	Type type = Type::empty;
	int reg_num = 0;
	std::string value;

	Operand() = default;

	bool operator==(const Operand& other) const {
		return type == other.type && reg_num == other.reg_num && value == other.value;
	}

	bool operator!=(const Operand& other) const {
		return !operator==(other);
	}

	Operand(const Type t, std::string v)
			: type(t)
			, value(std::move(v)) {
		assert(type == Type::literal);
	}

	Operand(const Type t, const int num)
			: type(t)
			, reg_num(num) {
		assert(type == Type::reg);
	}
};

struct i386;

void log(LogLevel ll, const i386& i, const std::string& message);
void log(LogLevel ll, const int line_no, const std::string& line, const std::string& message);
void log(LogLevel ll, const std::string& message);

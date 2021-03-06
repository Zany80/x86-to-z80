#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <regex>
#include <cassert>
#include <map>
#include <cctype>
#include <fstream>

#include "z80.h"
#include "i386.h"
#include "types.h"

std::string to_string(const LogLevel ll) {
	switch (ll) {
		case LogLevel::Warning:
			return "[Warning] ";
		case LogLevel::Error:
			return "[Error] ";
	}
	return "[Log] ";
}

void log(LogLevel ll, const i386& i, const std::string& message) {
	std::cout << to_string(ll) << ": " << i.line_num << ": " << message << ": `" << i.line_text << "`\n";
}

void log(LogLevel ll, const int line_no, const std::string& line, const std::string& message) {
	std::cout << to_string(ll) << ": " << line_no << ": " << message << ": `" << line << "`\n";
}

void log(LogLevel ll, const std::string& message) {
	std::cout << to_string(ll) << message << '\n';
}

int processInput(std::istream& is, Target target) {
	std::regex Comment(R"(\s*\#(.*))");
	std::regex Label(R"(^(\S+):.*)");
	std::regex Directive(R"(^\t(\..+))");
	std::regex UnaryInstruction(R"(^\t(\S+)\s+(\S+))");
	std::regex BinaryInstruction(R"(^\t(\S+)\s+(\S+),\s+(\S+))");
	std::regex Instruction(R"(^\t(\S+))");

	int lineno = 0;

	std::vector<i386> instructions;

	while (is.good()) {
		std::string line;
		getline(is, line);
		
		try {
			std::smatch match;
			if (std::regex_match(line, match, Label)) {
				instructions.emplace_back(lineno, line, ASMLine::Type::Label, match[1]);
			}
			else if (std::regex_match(line, match, Comment)) {
				instructions.emplace_back(lineno, line, ASMLine::Type::Comment, "; " + std::string(match[1]));
			}
			else if (std::regex_match(line, match, Directive)) {
				instructions.emplace_back(lineno, line, ASMLine::Type::Directive, match[1]);
			}
			else if (std::regex_match(line, match, UnaryInstruction)) {
				instructions.emplace_back(lineno, line, ASMLine::Type::Instruction, match[1], match[2]);
			}
			else if (std::regex_match(line, match, BinaryInstruction)) {
				instructions.emplace_back(lineno, line, ASMLine::Type::Instruction, match[1], match[2], match[3]);
			}
			else if (std::regex_match(line, match, Instruction)) {
				instructions.emplace_back(lineno, line, ASMLine::Type::Instruction, match[1]);
			}
			else if (line == "") {
				//std::cout << "EmptyLine\n";
			}
			else {
				throw std::runtime_error("Unparsed Input, Line: " + std::to_string(lineno));
			}
		}
		catch (const std::exception& e) {
			log(LogLevel::Error, lineno, line, e.what());
		}

		++lineno;
	}

	std::set<std::string> needed_objects, labels;

	for (auto &i : instructions) {
		if (i.type == ASMLine::Type::Label) {
			labels.emplace(i.text);
		}
	}
	
	for (const auto &i : instructions) {
		if (i.type == ASMLine::Type::Directive) {
			std::smatch match;
			if (std::regex_match(i.text, match, std::regex(R"(\.type\s(.*),(.*))"))) {
				needed_objects.emplace(match[1]);
			}
		}
		if (i.type == ASMLine::Type::Instruction) {
			if (labels.count(i.operand1.value)) {
				needed_objects.emplace(i.operand1.value);
			}
			if (labels.count(i.operand2.value)) {
				needed_objects.emplace(i.operand2.value);
			}
		}
	}
	
	instructions.erase(
		std::remove_if(std::begin(instructions), std::end(instructions),
				[&needed_objects](const auto& i) {
					if (i.type == ASMLine::Type::Label) {
						return needed_objects.count(i.text) == 0;
					}
					return false;
				}),
		std::end(instructions));
	
	for (auto &i : instructions) {
		if (i.type == ASMLine::Type::Label) {
			i.text = i.text + ":";
			std::size_t pos;
			while ((pos = i.text.find(".")) != std::string::npos) {
				i.text.replace(pos, 1, "period_");
			}
		}
	}
	
	if (target == Target::Z80) {
		
		std::vector<z80> translated_instructions;
		
		for (const auto& i : instructions) {
			to_z80(i, translated_instructions);
		}
		
		for (const auto i : translated_instructions) {
			std::cout << i.to_string() << '\n';
		}
		
	}

	return 0;
}

int main(int argc, char* argv[]) {
	if (argc > 1) {
		std::fstream fs;
		fs.open(argv[1], std::fstream::in);
		if (!fs) {
			std::cerr << "could not open file: " << argv[1] << "\n";
			return -1;
		}
		return processInput(fs, Target::Z80);
	}
	else {
		return processInput(std::cin, Target::Z80);
	}
}

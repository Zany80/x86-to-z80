#include "z80.h"
#include "i386.h"
#include <vector>

void translate_instruction(std::vector<z80>& instructions, const i386::OpCode op, const Operand& o1, const Operand& o2)
{
  switch (op)
  {
  case i386::OpCode::xorl:
	  instructions.emplace_back(z80::OpCode::xor);
	  break;
  case i386::OpCode::movb:
	  instructions.emplace_back(z80::OpCode::ld);
	  instructions.emplace_back(z80::OpCode::ldxxxx);
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

void to_z80(const i386& i, std::vector<z80>& instructions)
{
  try
  {
    switch (i.type)
    {
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
        if (text[0] == '\t')
        {
          text.erase(0, 1);
        }
        for_each(std::next(instructions.begin(), head), instructions.end(),
            [text](auto& ins) {

              ins.comment = text;
            });
        return;
    }
  }
  catch (const std::exception& e)
  {
    log(LogLevel::Error, i, e.what());
  }
}

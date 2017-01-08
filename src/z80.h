#pragma once

#include "types.h"
#include <algorithm>
#include <vector>

struct z80 : ASMLine
{
  enum class OpCode
  {
    unknown,
    ret,
    xor,
    ld,
    ldxxxx,
    call
  };

  z80(const Type t, std::string s)
      : ASMLine(t, std::move(s))
  {
  }

  static std::string to_string(const OpCode o)
  {
    switch (o)
    {
      case OpCode:: xor:
        return "xor a";
      case OpCode::ld:
        return "ld a,16";
      case OpCode::ldxxxx:
        return "ld (40960),a";
      case OpCode::call:
        return "call";
      case OpCode::ret:
        return "ret";
      case OpCode::unknown:
        return "unknown";
    }
    return "unknown";
  }

  explicit z80(const OpCode o)
      : ASMLine(Type::Instruction, to_string(o))
      , opcode(o)
  // , is_branch(get_is_branch(o))
  // , is_comparison(get_is_comparison(o))
  {
  }


  std::string to_string() const
  {
    switch (type)
    {
      case ASMLine::Type::Label:
        return text; // + ':';
      case ASMLine::Type::Directive:
      case ASMLine::Type::Instruction:
      {
        const std::string line = '\t' + text + ' '; // +op.value;
        return line + std::string(static_cast<size_t>(std::max(15 - static_cast<int>(line.size()), 1)), ' ') + "; " + comment;
      }
    };
    throw std::runtime_error("Unable to render: " + text);
  }

  OpCode opcode;
  std::string comment;
};

struct i386;

void to_z80(const i386& i, std::vector<z80>& instructions);

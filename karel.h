#ifndef KAREL_H
#define KAREL_H

#include <limits>
#include <optional>
#include <string_view>
#include <vector>
#include <cstdint>

namespace karel {

constexpr int32_t kInfinity = 1'000'000'005; /**Value used to represent infinity in Karel */

constexpr int32_t kMaxInt = 999'999'999; /**Maximum integer value allowed in a Karel context */
constexpr int32_t kMinInt = -999'999'999;/**Minimum value allowed in a Karel context */

enum class Opcode : uint32_t {
  HALT,
  LINE,
  LEFT,
  WORLDWALLS,
  ORIENTATION,
  ROTL,
  ROTR,
  MASK,
  NOT,
  AND,
  OR,
  EQ,
  EZ,
  JZ,
  JMP,
  FORWARD,
  WORLDBUZZERS,
  BAGBUZZERS,
  PICKBUZZER,
  LEAVEBUZZER,
  LOAD,
  POP,
  DUP,
  DEC,
  INC,
  CALL,
  RET,
  PARAM,
  SRET,
  LRET,
  LT,
  LTE,
  COLUMN,
  ROW
};

constexpr const char* kOpcodeNames[] = {
    "HALT",    "LINE",         "LEFT",       "WORLDWALLS", "ORIENTATION",
    "ROTL",    "ROTR",         "MASK",       "NOT",        "AND",
    "OR",      "EQ",           "EZ",         "JZ",         "JMP",
    "FORWARD", "WORLDBUZZERS", "BAGBUZZERS", "PICKBUZZER", "LEAVEBUZZER",
    "LOAD",    "POP",          "DUP",        "DEC",        "INC",
    "CALL",    "RET",          "PARAM",      "SRET",       "LRET",
    "LT",      "LTE"
  };

struct Instruction {
  Opcode opcode = Opcode::HALT;
  int32_t arg = 0;
  int32_t arg2 = 0;
};

enum class RunResult : uint32_t {
  OK,
  WALL = 16,
  WORLDUNDERFLOW,
  BAGUNDERFLOW,
  STACK,
  STACKMEMORY,
  CALLSIZE,
  INTEGEROVERFLOW,
  INTEGERUNDERFLOW,
  WORLDOVERFLOW,
  BAGOVERFLOW,
  INSTRUCTION = 48,
  INSTRUCTION_LEFT,
  INSTRUCTION_FORWARD ,
  INSTRUCTION_PICK,
  INSTRUCTION_LEAVE
};

struct Runtime {
  size_t orientation = 1;
  size_t x = 0;
  size_t y = 0;
  size_t bag = 0;
  size_t line = 0;
  size_t column = 0;
  size_t instruction_limit = 10000000;
  size_t stack_limit = 65000;
  size_t stack_memory_limit = 65000;
  size_t call_param_limit = 5;
  size_t forward_limit = std::numeric_limits<size_t>::max();
  size_t left_limit = std::numeric_limits<size_t>::max();
  size_t pickbuzzer_limit = std::numeric_limits<size_t>::max();
  size_t leavebuzzer_limit = std::numeric_limits<size_t>::max();
  size_t forward_count = 0;
  size_t left_count = 0;
  size_t leavebuzzer_count = 0;
  size_t pickbuzzer_count = 0;
  size_t stack_memory = 0;

  size_t width = 100;
  size_t height = 100;
  int32_t ret = 0;
  uint32_t* buzzers = nullptr;
  uint8_t* walls = nullptr;

  size_t coordinates(size_t x, size_t y) const { return y * width + x; }

  void inc_buzzers(int32_t count) {
    if (buzzers[coordinates(x, y)] == kInfinity)
      return;
    buzzers[coordinates(x, y)] += count;
  }

  uint32_t get_buzzers() const { return buzzers[coordinates(x, y)]; }

  uint8_t get_walls() const { return walls[coordinates(x, y)]; }
};

std::optional<std::vector<Instruction>> ParseInstructions(
    std::string_view program);

RunResult Run(const std::vector<Instruction>& program, Runtime* runtime);

}  // namespace karel

#endif // KAREL_H
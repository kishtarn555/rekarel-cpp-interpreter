#include <gtest/gtest.h>
#include "../karel.h"
#include<vector>

struct TestKarel : public ::testing::Test {

  karel::Runtime * runtime;
  virtual void SetUp() override {
    runtime = new karel::Runtime();
    //Start world with zeros
    runtime->buzzers = new uint32_t[runtime->width * runtime->height]();
    runtime->walls  = new uint8_t[runtime->width * runtime->height]();
  }
  
  virtual void TearDown() override {
    delete [] (runtime->buzzers);
    delete [] (runtime->walls);
    delete runtime;
  }
};


TEST_F(TestKarel, HALT) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::HALT}
  };
  auto result = karel::Run(program,runtime);
  ASSERT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
}

TEST_F(TestKarel, LINE) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::LINE, 2, 5}
  };
  auto result = karel::Run(program,runtime);
  ASSERT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
  ASSERT_EQ(2, runtime->line) << "Line was not set correctly";
  ASSERT_EQ(5, runtime->column) << "Column was not set correclty";
}

TEST_F(TestKarel, SRET) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::LOAD, 12},
    {karel::Opcode::SRET}
  };
  auto result = karel::Run(program,runtime);
  ASSERT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
  ASSERT_EQ(12, runtime->ret) << "RET was not set correctly";
}

TEST_F(TestKarel, LRET) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::LRET},
    {karel::Opcode::INC, 1},
    {karel::Opcode::SRET},
  };
  runtime->ret = 10;
  auto result = karel::Run(program,runtime);
  ASSERT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
  ASSERT_EQ(runtime->ret, 11) << "RET not set correclty";
}

TEST_F(TestKarel, INC) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::LOAD, 10},
    {karel::Opcode::INC, 4},
    {karel::Opcode::SRET},
  };
  auto result = karel::Run(program,runtime);
  ASSERT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
  ASSERT_EQ(14, runtime->ret) << "RET was not set correctly";
}

TEST_F(TestKarel, DEC) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::LOAD, 10},
    {karel::Opcode::DEC, 4},
    {karel::Opcode::SRET},
  };
  auto result = karel::Run(program,runtime);
  ASSERT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
  ASSERT_EQ(6, runtime->ret) << "RET was not set correctly";
}

TEST_F(TestKarel, LT_3_5) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::LOAD, 3},
    {karel::Opcode::LOAD, 5},
    {karel::Opcode::LT},
    {karel::Opcode::SRET},
  };
  auto result = karel::Run(program,runtime);
  ASSERT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
  ASSERT_EQ(1, runtime->ret) << "RET was not set correctly";
}

TEST_F(TestKarel, LT_5_3) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::LOAD, 5},
    {karel::Opcode::LOAD, 3},
    {karel::Opcode::LT},
    {karel::Opcode::SRET},
  };
  auto result = karel::Run(program,runtime);
  ASSERT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
  ASSERT_EQ(0, runtime->ret) << "RET was not set correctly";
}

TEST_F(TestKarel, LT_5_5) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::LOAD, 5},
    {karel::Opcode::LOAD, 5},
    {karel::Opcode::LT},
    {karel::Opcode::SRET},
  };
  auto result = karel::Run(program,runtime);
  ASSERT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
  ASSERT_EQ(0, runtime->ret) << "RET was not set correctly";
}



TEST_F(TestKarel, LTE_3_5) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::LOAD, 3},
    {karel::Opcode::LOAD, 5},
    {karel::Opcode::LTE},
    {karel::Opcode::SRET},
  };
  auto result = karel::Run(program,runtime);
  ASSERT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
  ASSERT_EQ(1, runtime->ret) << "RET was not set correctly";
}

TEST_F(TestKarel, LTE_5_3) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::LOAD, 5},
    {karel::Opcode::LOAD, 3},
    {karel::Opcode::LTE},
    {karel::Opcode::SRET},
  };
  auto result = karel::Run(program,runtime);
  ASSERT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
  ASSERT_EQ(0, runtime->ret) << "RET was not set correctly";
}

TEST_F(TestKarel, LTE_5_5) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::LOAD, 5},
    {karel::Opcode::LOAD, 5},
    {karel::Opcode::LTE},
    {karel::Opcode::SRET},
  };
  auto result = karel::Run(program,runtime);
  ASSERT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
  ASSERT_EQ(1, runtime->ret) << "RET was not set correctly";
}

TEST_F(TestKarel, SIMPLE_CALL) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::LOAD, 0},
    {karel::Opcode::CALL, 3},
    {karel::Opcode::HALT},
    {karel::Opcode::FORWARD},
    {karel::Opcode::RET}
  };
  auto result = karel::Run(program,runtime);
  ASSERT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
  ASSERT_EQ(runtime->x, 0) << "X is wrong";
  ASSERT_EQ(runtime->y, 1) << "Y is wrong";

}

TEST_F(TestKarel, PARAMS) {
  for (int i =0; i < 3; i++) {
    std::vector<karel::Instruction> program = {
      {karel::Opcode::LOAD, 0},
      {karel::Opcode::LOAD, 1},
      {karel::Opcode::LOAD, 2},
      {karel::Opcode::LOAD, 3},
      {karel::Opcode::CALL, 6},
      {karel::Opcode::HALT},
      {karel::Opcode::PARAM, i},
      {karel::Opcode::SRET},
      {karel::Opcode::RET}
    };
    auto result = karel::Run(program,runtime);
    EXPECT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status at " << i;
    EXPECT_EQ(runtime->ret, 2-i) << "RET was not set correctly: " << i;
  }
}

TEST_F(TestKarel, STACK_RESTORATION_AFTER_RETURN) {
  std::vector<karel::Instruction> program = {
    {karel::Opcode::LOAD, 10},
    {karel::Opcode::LOAD, 0},
    {karel::Opcode::LOAD, 1},
    {karel::Opcode::LOAD, 2},
    {karel::Opcode::LOAD, 3},
    {karel::Opcode::CALL, 8},
    {karel::Opcode::SRET},
    {karel::Opcode::HALT},
    {karel::Opcode::PARAM, 0},
    {karel::Opcode::PARAM, 1},
    {karel::Opcode::PARAM, 1},
    {karel::Opcode::PARAM, 2},
    {karel::Opcode::RET}
  };
  auto result = karel::Run(program,runtime);
  EXPECT_EQ(result, karel::RunResult::OK) << "Run did not end in OK status";
  EXPECT_EQ(runtime->ret, 10) << "RET was not set correctly";

}


TEST_F(TestKarel, CODE_PARSE) {
  auto program = karel::ParseInstructions(
  "["
    "[\"HALT\"],"
    "[\"LINE\", 1, 2],"
    "[\"LEFT\"],"
    "[\"WORLDWALLS\"],"
    "[\"ORIENTATION\"],"
    "[\"ROTL\"],"
    "[\"ROTL\"],"
    "[\"ROTR\"],"
    "[\"MASK\"],"
    "[\"NOT\"],"
    "[\"AND\"],"
    "[\"OR\"],"
    "[\"EQ\"],"
    "[\"EZ\", \"WALL\"],"
    "[\"EZ\", \"WORLDUNDERFLOW\"],"
    "[\"EZ\", \"BAGUNDERFLOW\"],"
    "[\"EZ\", \"INSTRUCTION\"],"
    "[\"JZ\", 5],"
    "[\"JMP\", 9],"
    "[\"FORWARD\"],"
    "[\"WORLDBUZZERS\"],"
    "[\"BAGBUZZERS\"],"
    "[\"PICKBUZZER\"],"
    "[\"LEAVEBUZZER\"],"
    "[\"LOAD\", 5],"
    "[\"POP\"],"
    "[\"DUP\"],"
    "[\"DEC\", 3],"
    "[\"INC\", 5],"
    "[\"CALL\", 8, \"FUNC_NAME\"],"
    "[\"RET\"],"
    "[\"PARAM\", 3],"
    "[\"SRET\"],"
    "[\"LRET\"],"
    "[\"LT\"],"
    "[\"LTE\"]"
  "]\0");
  if (!program) {
    FAIL() << "Program failed to parse";
  }
  std::vector<karel::Instruction> expected = {
    {karel::Opcode::HALT},
    {karel::Opcode::LINE, 1, 2},
    {karel::Opcode::LEFT},
    {karel::Opcode::WORLDWALLS},
    {karel::Opcode::ORIENTATION},
    {karel::Opcode::ROTL},
    {karel::Opcode::ROTR},
    {karel::Opcode::MASK},
    {karel::Opcode::NOT},
    {karel::Opcode::AND},
    {karel::Opcode::OR},
    {karel::Opcode::EQ},
    {karel::Opcode::EZ, (int32_t)karel::RunResult::WALL},
    {karel::Opcode::EZ, (int32_t)karel::RunResult::WORLDUNDERFLOW},
    {karel::Opcode::EZ, (int32_t)karel::RunResult::BAGUNDERFLOW},
    {karel::Opcode::EZ, (int32_t)karel::RunResult::INSTRUCTION},
    {karel::Opcode::JZ, 5},
    {karel::Opcode::JMP, 9},
    {karel::Opcode::FORWARD},
    {karel::Opcode::WORLDBUZZERS},
    {karel::Opcode::PICKBUZZER},
    {karel::Opcode::LEAVEBUZZER},
    {karel::Opcode::LOAD, 5},
    {karel::Opcode::POP},
    {karel::Opcode::DUP},
    {karel::Opcode::CALL, 8},
    {karel::Opcode::RET},
    {karel::Opcode::PARAM},
    {karel::Opcode::SRET},
    {karel::Opcode::LRET},
    {karel::Opcode::LT},
    {karel::Opcode::LTE}
  };
  // ASSERT_EQ(program.value(), expected)<< "Program output is not what it was expected";

}
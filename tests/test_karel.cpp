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
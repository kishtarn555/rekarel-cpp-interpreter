#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "karel.h"
#include "logging.h"
#include "world.h"
#include "util.h"

namespace {

constexpr const std::string_view kFlagPrefix("--");
constexpr const std::string_view kDumpFlagPrefix("dump=");

std::vector<uint8_t> ReadFully(int fd) {
  constexpr size_t kChunkSize = 4096;
  std::vector<std::unique_ptr<uint8_t[]>> chunks;
  size_t total_bytes = 0;
  while (true) {
    chunks.emplace_back(std::make_unique<uint8_t[]>(kChunkSize));
    ssize_t bytes_read = read(fd, chunks.back().get(), kChunkSize);
    if (bytes_read == -1) {
      PLOG(ERROR) << "Failed to read file";
      return {};
    }
    if (bytes_read == 0)
      break;
    total_bytes += bytes_read;
  }
  std::vector<uint8_t> result(total_bytes + 1);
  uint8_t* ptr = result.data();
  for (const auto& chunk : chunks) {
    size_t chunk_bytes = std::min(kChunkSize, total_bytes);
    memcpy(ptr, chunk.get(), chunk_bytes);
    total_bytes -= chunk_bytes;
    ptr += chunk_bytes;
  }
  result.pop_back();
  return result;
}


[[noreturn]] void Usage(const std::string_view program_name) {
  LOG(ERROR) << "Usage: " << program_name
             << " [--dump={world,result}] program.kx < world.in > world.out";
  exit(1);
}

}  // namespace

int main(int argc, char* argv[]) {
  bool dump_result = true;

  for (int i = 1; i < argc; ++i) {
    std::string_view arg = argv[i];
    if (arg.find(kFlagPrefix) != 0)
      continue;
    arg.remove_prefix(kFlagPrefix.size());

    if (arg.find(kDumpFlagPrefix) == 0) {
      arg.remove_prefix(kDumpFlagPrefix.size());
      if (arg == "world")
        dump_result = false;
      else if (arg == "result")
        dump_result = true;
      else
        Usage(argv[0]);
    } else {
      Usage(argv[0]);
    }

    // Shift all arguments by one.
    --argc;
    for (int j = i; j < argc; ++j)
      argv[j] = argv[j + 1];
    --i;
  }

  if (argc < 2)
    Usage(argv[0]);

  ScopedFD program_fd(open(argv[1], O_RDONLY));
  if (!program_fd) {
    PLOG(ERROR) << "Failed to open " << argv[1];
    return -1;
  }
  auto program_str = ReadFully(program_fd.get());
  auto program = karel::ParseInstructions(std::string_view(
      reinterpret_cast<const char*>(program_str.data()), program_str.size()));
  if (!program)
    return -1;

  auto world = karel::World::Parse(STDIN_FILENO);
  if (!world)
    return -1;

  auto result = karel::Run(program.value(), world->runtime());
  switch (result) {
    case karel::RunResult::OK:
      // No STDERR
      break;
    case karel::RunResult::WALL:
      WriteFileDescriptor(STDERR_FILENO, "MOVIMIENTO INVALIDO");
      break;
    case karel::RunResult::WORLDUNDERFLOW:
      WriteFileDescriptor(STDERR_FILENO, "ZUMBADOR INVALIDO MUNDO");
      break;
    case karel::RunResult::BAGUNDERFLOW:
      WriteFileDescriptor(STDERR_FILENO, "ZUMBADOR INVALIDO MOCHILA");
      break;
    case karel::RunResult::STACK:
      WriteFileDescriptor(STDERR_FILENO, "STACK OVERFLOW");
      break;
    case karel::RunResult::STACKMEMORY:
      WriteFileDescriptor(STDERR_FILENO, "LIMITE DE MEMORIA DEL STACK");
      break;
    case karel::RunResult::CALLSIZE:
      WriteFileDescriptor(STDERR_FILENO, "LIMITE DE LONGITUD DE LLAMADA");
      break;
    case karel::RunResult::INSTRUCTION:
      WriteFileDescriptor(STDERR_FILENO, "LIMITE DE INSTRUCCIONES GENERAL");
      break;
    case karel::RunResult::INSTRUCTION_LEFT:
      WriteFileDescriptor(STDERR_FILENO, "LIMITE DE INSTRUCCIONES IZQUIERDA");
      break;
    case karel::RunResult::INSTRUCTION_FORWARD:
      WriteFileDescriptor(STDERR_FILENO, "LIMITE DE INSTRUCCIONES AVANZA");
      break;
    case karel::RunResult::INSTRUCTION_PICK:
      WriteFileDescriptor(STDERR_FILENO, "LIMITE DE INSTRUCCIONES COGE_ZUMBADOR");
      break;
    case karel::RunResult::INSTRUCTION_LEAVE:
      WriteFileDescriptor(STDERR_FILENO, "LIMITE DE INSTRUCCIONES DEJA_ZUMBADOR");
      break;
  }
  if (dump_result)
    world->DumpResult(result);
  else
    world->Dump();

  return static_cast<int32_t>(result);
}

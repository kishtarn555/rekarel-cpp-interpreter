#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

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
  LOG(ERROR) 
    << "Usage: " <<program_name << "<bytecode-file> [options]\n"
    << "Arguments:\n"
    << "  <bytecode-file>             The Karel bytecode file to execute. This is a required argument.\n"
    << "Options:\n"
    << "  -i, --input <input-path>    Specify a file to read the world input from. If not provided, the program reads from stdin.\n"
    << "  -o, --output <output-path>  Specify a file to write the world output to. If not provided, the program writes to stdout.\n"
    << "  -d, --dump {world|result}   Set the output type:\n"
    << "    - result:   (default) Outputs the program's result.\n"
    << "    - world:    Outputs the world input.\n"
    << "  -e, --expect-version <major.minor>\n"
    << "    Specify the required version of the program (major.minor).\n"
    << "    If the version does not match, the program exits with an error.\n"
    << "\n"
    << "Example:\n"
    << "  " << program_name << " mycode.kx -i world.in -o world.out -d world -e 3.2\n"
    << "  " << program_name << " mycode.kx -d result\n"
    << "  " << program_name << " --version \n"
  ;
  exit(1);
}

}  // namespace


constexpr const char* PROGRAM_VERSION = "2.3.0";

bool CheckVersion(const std::string& expected_version) {
    std::istringstream prog_stream(PROGRAM_VERSION);
    std::istringstream expect_stream(expected_version);
    std::string prog_major, prog_minor, expect_major, expect_minor;

    std::getline(prog_stream, prog_major, '.');
    std::getline(prog_stream, prog_minor, '.');
    std::getline(expect_stream, expect_major, '.');
    std::getline(expect_stream, expect_minor, '.');

    return prog_major == expect_major && prog_minor == expect_minor;
}

int main(int argc, char* argv[]) {
  bool dump_result = true;  
  struct option long_options[] = {
      {"help", no_argument, nullptr, 'h'},
      {"version", no_argument, nullptr, 'v'},
      {"dump", required_argument, nullptr, 'd'},
      {"input", required_argument, nullptr, 'i'},
      {"output", required_argument, nullptr, 'o'},
      {"expect-version", required_argument, nullptr, 'e'},
      {nullptr, 0, nullptr, 0} // End of options
  };
  std::string expected_version = "";
  std::optional<std::string> output_file;
  std::optional<std::string> input_file;
  int opt;
  while ((opt = getopt_long(argc, argv, "hvd:i:o:e:", long_options, nullptr)) != -1) {
      switch (opt) {
          case 'v':
              WriteFileDescriptor(STDOUT_FILENO, std::string(PROGRAM_VERSION) + "\n");
              return 0;
          case 'd':
              if (std::string_view(optarg) != "world" && std::string_view(optarg) != "result") {
                  LOG(ERROR) << "Error: Invalid dump option. Use 'world' or 'result'.\n";
                  Usage(argv[0]);
                  return 1;
              }
              dump_result = std::string_view(optarg) == "result";
              break;
          case 'i':
              input_file = optarg;
              break;
          case 'o':
              output_file = optarg;
              break;
          case 'e':
              expected_version = optarg;
              if (!CheckVersion(expected_version)) {
                  LOG(ERROR) << "Error: Version mismatch. Expected: " << expected_version
                            << ", Found: " << PROGRAM_VERSION << "\n";
                  return 2;
              }
              break;
          case 'h':
              Usage(argv[0]);
              break;
          default:
              Usage(argv[0]);
      }
  }

  if (optind >= argc || argc < 2) {
    Usage(argv[0]);
  }
  ScopedFD program_fd(open(argv[optind], O_RDONLY));
  if (!program_fd) {
    PLOG(ERROR) << "Failed to open " << argv[optind];
    return -1;
  }
  auto program_str = ReadFully(program_fd.get());
  auto program = karel::ParseInstructions(std::string_view(
      reinterpret_cast<const char*>(program_str.data()), program_str.size()));
  if (!program)
    return -1;
  int input_fd = STDIN_FILENO;
  if (input_file) {
        input_fd = open(input_file->c_str(), O_RDONLY);
        if (input_fd == -1) {
            perror("Error opening file");
            return 1;
        }
    }

  auto world = karel::World::Parse(input_fd);
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
    case karel::RunResult::INTEGEROVERFLOW:
      WriteFileDescriptor(STDERR_FILENO, "INTEGER OVERFLOW");
      break;
    case karel::RunResult::INTEGERUNDERFLOW:
      WriteFileDescriptor(STDERR_FILENO, "INTEGER UNDERFLOW");
      break;
    case karel::RunResult::WORLDOVERFLOW:
      WriteFileDescriptor(STDERR_FILENO, "DEMASIADOS ZUMBADORES (MUNDO)");
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
  int output_fd = STDOUT_FILENO;
  if (output_file) {
      output_fd = open(output_file->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (output_fd == -1) {
          perror("Error opening output file");
          return 1;
      }
  }

  if (dump_result)
    world->DumpResult(result, output_fd);
  else
    world->Dump(output_fd);

  if (output_fd != STDOUT_FILENO) {
    close(output_fd);
  }
  if (input_fd != STDIN_FILENO) {
    close(input_fd);
  }

  return static_cast<int32_t>(result);
}

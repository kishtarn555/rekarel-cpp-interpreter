#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "world.h"
#include "logging.h"
#include "xml.h"
#include "util.h"

namespace karel {
    

World::World(World&& other)
      : width_(other.width_),
        height_(other.height_),
        name_(std::move(other.name_)),
        program_name_(std::move(other.program_name_)),
        target_version(std::move(other.target_version)),
        buzzers_(std::move(other.buzzers_)),
        walls_(std::move(other.walls_)),
        buzzer_dump_(std::move(other.buzzer_dump_)),
        dump_world_(other.dump_world_),
        dump_universe_(other.dump_universe_),
        dump_position_(other.dump_position_),
        dump_orientation_(other.dump_orientation_),
        dump_bag_(other.dump_bag_),
        dump_forward_(other.dump_forward_),
        dump_left_(other.dump_left_),
        dump_leavebuzzer_(other.dump_leavebuzzer_),
        dump_pickbuzzer_(other.dump_pickbuzzer_) {
    runtime_ = other.runtime_;
    runtime_.buzzers = buzzers_.get();
    runtime_.walls = walls_.get();
}

size_t World::coordinates(size_t x, size_t y) const { return y * width_ + x; }

void World::set_buzzers(size_t x, size_t y, uint32_t count) {
    buzzers_[coordinates(x, y)] = count;
}

uint32_t World::get_buzzers(size_t x, size_t y) const {
    return buzzers_[coordinates(x, y)];
}

uint8_t World::get_walls(size_t x, size_t y) const {
    return walls_[coordinates(x, y)];
}

std::optional<World> World::Parse(int fd) {
    World world;
    if (!xml::Reader().Parse(fd, [&world](xml::Reader::Element node) -> bool {
          const std::string_view name = node.GetName();
          if (name == "ejecucion") {
              auto version = node.GetAttribute("version");
              world.target_version = std::string(version.value_or("1.0"));
          } else if (name == "mundo") {
            auto width = ParseString<uint32_t>(node.GetAttribute("ancho")),
                 height = ParseString<uint32_t>(node.GetAttribute("alto"));
            if (!width || !height)
              return false;

            world.Init(width.value(), height.value(),
                       node.GetAttribute("nombre").value_or("mundo_0"));
          } else if (name == "condiciones") {
            auto instruction_limit = ParseString<size_t>(
                     node.GetAttribute("instruccionesMaximasAEjecutar")),
                 stack_limit =
                     ParseString<size_t>(node.GetAttribute("longitudStack")),
                 call_param_limit = 
                     ParseString<size_t>(node.GetAttribute("llamadaMaxima")),
                 stack_memory_limit = 
                     ParseString<size_t>(node.GetAttribute("memoriaStack"));
            if (instruction_limit)
              world.runtime_.instruction_limit = instruction_limit.value();
            if (stack_limit)
              world.runtime_.stack_limit = stack_limit.value();
            if (call_param_limit)
              world.runtime_.call_param_limit = call_param_limit.value();
            if (stack_memory_limit)
              world.runtime_.stack_memory_limit = stack_memory_limit.value();
          } else if (name == "comando") {
            auto nombre = node.GetAttribute("nombre");
            auto maximoNumeroDeEjecuciones = ParseString<size_t>(
                node.GetAttribute("maximoNumeroDeEjecuciones"));
            if (!maximoNumeroDeEjecuciones)
              return false;
            if (nombre.value() == "AVANZA")
              world.runtime_.forward_limit = maximoNumeroDeEjecuciones.value();
            else if (nombre.value() == "GIRA_IZQUIERDA")
              world.runtime_.left_limit = maximoNumeroDeEjecuciones.value();
            else if (nombre.value() == "COGE_ZUMBADOR")
              world.runtime_.pickbuzzer_limit =
                  maximoNumeroDeEjecuciones.value();
            else if (nombre.value() == "DEJA_ZUMBADOR")
              world.runtime_.leavebuzzer_limit =
                  maximoNumeroDeEjecuciones.value();
            else {
              LOG(ERROR) << "Invalid limit name " << nombre.value();
              return false;
            }
          } else if (name == "monton") {
            auto x = ParseString<size_t>(node.GetAttribute("x")),
                 y = ParseString<size_t>(node.GetAttribute("y"));
            auto count = ParseString<uint32_t>(node.GetAttribute("zumbadores"));
            if (!x || !y || !count)
              return false;
            (*x)--;
            (*y)--;
            if (x.value() >= world.width_ || y.value() >= world.height_)
              return true;
            world.set_buzzers(*x, *y, *count);
          } else if (name == "pared") {
            auto x1 = ParseString<size_t>(node.GetAttribute("x1", false)),
                 y1 = ParseString<size_t>(node.GetAttribute("y1", false)),
                 x2 = ParseString<size_t>(node.GetAttribute("x2", false)),
                 y2 = ParseString<size_t>(node.GetAttribute("y2", false));
            if (x1 && x2 && y1 && !y2) {
              // Horizontal
              size_t x = std::min(*x1, *x2);
              size_t y = *y1;
              if (x >= world.width_ || y >= world.height_)
                return true;
              world.walls_[world.coordinates(x, y)] |= 1 << 3;
              if (y)
                world.walls_[world.coordinates(x, y - 1)] |= 1 << 1;
            } else if (y1 && y2 && x1 && !x2) {
              // Vertical
              size_t x = *x1;
              size_t y = std::min(*y1, *y2);
              if (x >= world.width_ || y >= world.height_)
                return true;
              world.walls_[world.coordinates(x, y)] |= 1 << 0;
              if (x)
                world.walls_[world.coordinates(x - 1, y)] |= 1 << 2;
            } else {
              LOG(ERROR) << "Invalid pared";
              return false;
            }
          } else if (name == "posicionDump") {
            auto x = ParseString<size_t>(node.GetAttribute("x")),
                 y = ParseString<size_t>(node.GetAttribute("y"));
            if (!x || !y)
              return false;
            (*x)--;
            (*y)--;
            if (x.value() >= world.width_ || y.value() >= world.height_)
              return true;
            world.buzzer_dump_[world.coordinates(x.value(), y.value())] = true;
          } else if (name == "programa") {
            auto karel_x = ParseString<size_t>(node.GetAttribute("xKarel")),
                 karel_y = ParseString<size_t>(node.GetAttribute("yKarel"));
            auto direccion_karel = node.GetAttribute("direccionKarel");
            auto karel_bag =
                ParseString<uint32_t>(node.GetAttribute("mochilaKarel"));
            auto nombre = node.GetAttribute("nombre");
            if (karel_x)
              world.runtime_.x = karel_x.value() - 1;
            if (karel_y)
              world.runtime_.y = karel_y.value() - 1;
            if (karel_bag)
              world.runtime_.bag = karel_bag.value();
            if (nombre)
              world.program_name_ = std::string(nombre.value());
            if (direccion_karel) {
              if (direccion_karel.value() == "OESTE")
                world.runtime_.orientation = 0;
              else if (direccion_karel.value() == "NORTE")
                world.runtime_.orientation = 1;
              else if (direccion_karel.value() == "ESTE")
                world.runtime_.orientation = 2;
              else if (direccion_karel.value() == "SUR")
                world.runtime_.orientation = 3;
              else {
                LOG(ERROR) << "Invalid orientation " << direccion_karel.value();
                return false;
              }
            }
          } else if (name == "despliega") {
            auto tipo = node.GetAttribute("tipo");
            if (!tipo) {
              LOG(ERROR) << "Invalid despliega";
              return false;
            }
            if (*tipo == "MUNDO") {
              world.dump_world_ = true;
            } else if (*tipo == "UNIVERSO") {
              world.dump_universe_ = true;
            } else if (*tipo == "ORIENTACION") {
              world.dump_orientation_ = true;
            } else if (*tipo == "POSICION") {
              world.dump_position_ = true;
            } else if (*tipo == "MOCHILA") {
              world.dump_bag_ = true;
            } else if (*tipo == "AVANZA") {
              world.dump_forward_ = true;
            } else if (*tipo == "GIRA_IZQUIERDA") {
              world.dump_left_ = true;
            } else if (*tipo == "DEJA_ZUMBADOR") {
              world.dump_leavebuzzer_ = true;
            } else if (*tipo == "COGE_ZUMBADOR") {
              world.dump_pickbuzzer_ = true;
            } else {
              LOG(ERROR) << "Invalid dump type " << *tipo;
              return false;
            }
          }

          return true;
        })) {
      return std::nullopt;
    }

    return std::make_optional<World>(std::move(world));
  }

  void World::Dump(int fd) const {
    xml::Writer writer(fd);

    auto ejecucion = writer.CreateElement("ejecucion");
    {
      auto condiciones = ejecucion.CreateElement("condiciones");
      condiciones.AddAttribute("instruccionesMaximasAEjecutar",
                               StringPrintf("%zd", runtime_.instruction_limit));
      condiciones.AddAttribute("longitudStack",
                               StringPrintf("%zd", runtime_.stack_limit));

      if (runtime_.forward_limit != std::numeric_limits<size_t>::max()) {
        auto comando = condiciones.CreateElement("comando");
        comando.AddAttribute("nombre", "AVANZA");
        comando.AddAttribute("maximoNumeroDeEjecuciones",
                             StringPrintf("%zu", runtime_.forward_limit));
      }
      if (runtime_.left_limit != std::numeric_limits<size_t>::max()) {
        auto comando = condiciones.CreateElement("comando");
        comando.AddAttribute("nombre", "GIRA_IZQUIERDA");
        comando.AddAttribute("maximoNumeroDeEjecuciones",
                             StringPrintf("%zu", runtime_.left_limit));
      }
      if (runtime_.pickbuzzer_limit != std::numeric_limits<size_t>::max()) {
        auto comando = condiciones.CreateElement("comando");
        comando.AddAttribute("nombre", "COGE_ZUMBADOR");
        comando.AddAttribute("maximoNumeroDeEjecuciones",
                             StringPrintf("%zu", runtime_.pickbuzzer_limit));
      }
      if (runtime_.leavebuzzer_limit != std::numeric_limits<size_t>::max()) {
        auto comando = condiciones.CreateElement("comando");
        comando.AddAttribute("nombre", "DEJA_ZUMBADOR");
        comando.AddAttribute("maximoNumeroDeEjecuciones",
                             StringPrintf("%zu", runtime_.leavebuzzer_limit));
      }
    }
    {
      auto mundos = ejecucion.CreateElement("mundos");
      auto mundo = mundos.CreateElement("mundo");
      mundo.AddAttribute("nombre", "mundo_0");
      mundo.AddAttribute("ancho", StringPrintf("%zd", runtime_.width));
      mundo.AddAttribute("alto", StringPrintf("%zd", runtime_.height));

      for (size_t x = 0; x < width_; ++x) {
        for (size_t y = 0; y < height_; ++y) {
          if (!runtime_.buzzers[coordinates(x, y)])
            continue;
          auto monton = mundo.CreateElement("monton");
          monton.AddAttribute("x", StringPrintf("%zd", x + 1));
          monton.AddAttribute("y", StringPrintf("%zd", y + 1));
          if (runtime_.buzzers[coordinates(x, y)] == karel::kInfinity) {
            monton.AddAttribute("zumbadores", "INFINITO");
          } else {
            monton.AddAttribute(
                "zumbadores",
                StringPrintf("%u", runtime_.buzzers[coordinates(x, y)]));
          }
        }
      }

      for (size_t x = 0; x < width_; ++x) {
        for (size_t y = 0; y < height_; ++y) {
          if (y + 1 < height_ && walls_[coordinates(x, y)] & (1 << 1)) {
            auto pared = mundo.CreateElement("pared");
            pared.AddAttribute("x1", StringPrintf("%zu", x));
            pared.AddAttribute("y1", StringPrintf("%zu", y + 1));
            pared.AddAttribute("x2", StringPrintf("%zu", x + 1));
          }
          if (x + 1 < width_ && walls_[coordinates(x, y)] & (1 << 2)) {
            auto pared = mundo.CreateElement("pared");
            pared.AddAttribute("x1", StringPrintf("%zu", x + 1));
            pared.AddAttribute("y1", StringPrintf("%zu", y));
            pared.AddAttribute("y2", StringPrintf("%zu", y + 1));
          }
        }
      }

      for (size_t x = 0; x < width_; ++x) {
        for (size_t y = 0; y < height_; ++y) {
          if (!buzzer_dump_[coordinates(x, y)])
            continue;
          auto posicionDump = mundo.CreateElement("posicionDump");
          posicionDump.AddAttribute("x", StringPrintf("%zd", x + 1));
          posicionDump.AddAttribute("y", StringPrintf("%zd", y + 1));
        }
      }
    }
    {
      auto programas = ejecucion.CreateElement("programas");
      programas.AddAttribute("tipoEjecucion", "CONTINUA");
      programas.AddAttribute("intruccionesCambioContexto", "1");
      programas.AddAttribute("milisegundosParaPasoAutomatico", "0");

      auto programa = programas.CreateElement("programa");
      programa.AddAttribute("nombre", "p1");
      programa.AddAttribute("ruta", "{$2$}");
      programa.AddAttribute("mundoDeEjecucion", "mundo_0");
      programa.AddAttribute("xKarel", StringPrintf("%zd", runtime_.x + 1));
      programa.AddAttribute("yKarel", StringPrintf("%zd", runtime_.y + 1));
      switch (runtime_.orientation) {
        case 0:
          programa.AddAttribute("direccionKarel", "OESTE");
          break;
        case 1:
          programa.AddAttribute("direccionKarel", "NORTE");
          break;
        case 2:
          programa.AddAttribute("direccionKarel", "ESTE");
          break;
        case 3:
          programa.AddAttribute("direccionKarel", "SUR");
          break;
      }
      if (runtime_.bag == karel::kInfinity)
        programa.AddAttribute("mochilaKarel", "INFINITO");
      else
        programa.AddAttribute("mochilaKarel",
                              StringPrintf("%zu", runtime_.bag));

      if (dump_world_) {
        auto despliega = programa.CreateElement("despliega");
        despliega.AddAttribute("tipo", "MUNDO");
      }
      if (dump_universe_) {
        auto despliega = programa.CreateElement("despliega");
        despliega.AddAttribute("tipo", "UNIVERSO");
      }
      if (dump_orientation_) {
        auto despliega = programa.CreateElement("despliega");
        despliega.AddAttribute("tipo", "ORIENTACION");
      }
      if (dump_position_) {
        auto despliega = programa.CreateElement("despliega");
        despliega.AddAttribute("tipo", "POSICION");
      }
      if (dump_bag_) {
        auto despliega = programa.CreateElement("despliega");
        despliega.AddAttribute("tipo", "MOCHILA");
      }
      if (dump_forward_) {
        auto despliega = programa.CreateElement("despliega");
        despliega.AddAttribute("tipo", "AVANZA");
      }
      if (dump_left_) {
        auto despliega = programa.CreateElement("despliega");
        despliega.AddAttribute("tipo", "GIRA_IZQUIERDA");
      }
      if (dump_leavebuzzer_) {
        auto despliega = programa.CreateElement("despliega");
        despliega.AddAttribute("tipo", "DEJA_ZUMBADOR");
      }
      if (dump_pickbuzzer_) {
        auto despliega = programa.CreateElement("despliega");
        despliega.AddAttribute("tipo", "COGE_ZUMBADOR");
      }
    }
  }

  void World::DumpResult(karel::RunResult result, int fd) const {
    {
      xml::Writer writer(fd);

      auto resultados = writer.CreateElement("resultados");

      if (dump_world_ || dump_universe_) {
        auto mundos = resultados.CreateElement("mundos");
        auto mundo = mundos.CreateElement("mundo");
        mundo.AddAttribute("nombre", name_);
        for (ssize_t y = static_cast<ssize_t>(height_) - 1; y >= 0; y--) {
          bool printCoordinate = true;
          std::ostringstream line;
          for (size_t x = 0; x < width_; x++) {
            if (!dump_universe_ && !buzzer_dump_[coordinates(x, y)])
              continue;
            if (get_buzzers(x, y) != 0) {
              if (printCoordinate) {
                line << '(' << (x + 1) << ") ";
              }
              uint32_t dump_buzzers= get_buzzers(x, y);
              if (dump_buzzers == karel::kInfinity) {
                dump_buzzers = 0xFFFF; // Handle infinite as 2^16-1
              }
              if (target_version == "1.0") {
                dump_buzzers = dump_buzzers & 0xFFFF; //Version 1.0 has a 16-bit output precision on beepers
              }
              line << (dump_buzzers) << ' ';
            }
            printCoordinate = get_buzzers(x, y) == 0;
          }

          if (line.tellp() == 0)
            continue;

          auto linea =
              mundo.CreateElement("linea", std::string_view(line.str()));
          linea.AddAttribute("fila", StringPrintf("%zd", y + 1));
          linea.AddAttribute("compresionDeCeros", "true");
        }
      }

      auto programas = resultados.CreateElement("programas");
      auto programa = programas.CreateElement("programa");
      programa.AddAttribute("nombre", program_name_);
      switch (result) {
        case karel::RunResult::OK:
          programa.AddAttribute("resultadoEjecucion", "FIN PROGRAMA");
          break;
        case karel::RunResult::WALL:
          programa.AddAttribute("resultadoEjecucion", "MOVIMIENTO INVALIDO");
          break;
        case karel::RunResult::WORLDUNDERFLOW:
          programa.AddAttribute("resultadoEjecucion", "ZUMBADOR INVALIDO MUNDO");
          break;
        case karel::RunResult::BAGUNDERFLOW:
          programa.AddAttribute("resultadoEjecucion", "ZUMBADOR INVALIDO MOCHILA");
          break;
        case karel::RunResult::INTEGEROVERFLOW:
          programa.AddAttribute("resultadoEjecucion", "INTEGER OVERFLOW");
          break;
        case karel::RunResult::INTEGERUNDERFLOW:
          programa.AddAttribute("resultadoEjecucion", "INTEGER UNDERFLOW");
          break;
        case karel::RunResult::WORLDOVERFLOW:
          programa.AddAttribute("resultadoEjecucion", "DEMASIADOS ZUMBADORES (MUNDO)");
          break;
        case karel::RunResult::BAGOVERFLOW:
          programa.AddAttribute("resultadoEjecucion", "DEMASIADOS ZUMBADORES (MOCHILA)");
          break;
        case karel::RunResult::INSTRUCTION:
          programa.AddAttribute("resultadoEjecucion",
                                "LIMITE DE INSTRUCCIONES GENERAL");
          break;
        case karel::RunResult::INSTRUCTION_FORWARD:
          programa.AddAttribute("resultadoEjecucion",
                                "LIMITE DE INSTRUCCIONES AVANZA");
          break;
        case karel::RunResult::INSTRUCTION_LEFT:
          programa.AddAttribute("resultadoEjecucion",
                                "LIMITE DE INSTRUCCIONES IZQUIERDA");
          break;
        case karel::RunResult::INSTRUCTION_PICK:
          programa.AddAttribute("resultadoEjecucion",
                                "LIMITE DE INSTRUCCIONES COGE_ZUMBADOR");
          break;
        case karel::RunResult::INSTRUCTION_LEAVE:
          programa.AddAttribute("resultadoEjecucion",
                                "LIMITE DE INSTRUCCIONES DEJA_ZUMBADOR");
          break;
        case karel::RunResult::STACK:
          programa.AddAttribute("resultadoEjecucion", "STACK OVERFLOW");
          break;
        case karel::RunResult::STACKMEMORY:
          programa.AddAttribute("resultadoEjecucion", "LIMITE DE MEMORIA DEL STACK");
          break;
        case karel::RunResult::CALLSIZE:
          programa.AddAttribute("resultadoEjecucion", "LIMITE DE LONGITUD DE LLAMADA");
          break;
      }
      if (dump_position_ || dump_orientation_ || dump_bag_) {
        auto karel = programa.CreateElement("karel");
        if (dump_position_) {
          karel.AddAttribute("x", StringPrintf("%zu", runtime_.x + 1));
          karel.AddAttribute("y", StringPrintf("%zu", runtime_.y + 1));
        }
        if (dump_orientation_) {
          switch (runtime_.orientation) {
            case 0:
              karel.AddAttribute("direccion", "OESTE");
              break;
            case 1:
              karel.AddAttribute("direccion", "NORTE");
              break;
            case 2:
              karel.AddAttribute("direccion", "ESTE");
              break;
            case 3:
              karel.AddAttribute("direccion", "SUR");
              break;
          }
        }
        if (dump_bag_) {
          if (runtime_.bag == karel::kInfinity)
            karel.AddAttribute("mochila", "INFINITO");
          else
            karel.AddAttribute("mochila", StringPrintf("%zu", runtime_.bag));
        }
      }
      if (dump_forward_ || dump_left_ || dump_leavebuzzer_ ||
          dump_pickbuzzer_) {
        auto instrucciones = programa.CreateElement("instrucciones");
        if (dump_forward_) {
          instrucciones.AddAttribute(
              "avanza", StringPrintf("%zu", runtime_.forward_count));
        }
        if (dump_left_) {
          instrucciones.AddAttribute("gira_izquierda",
                                     StringPrintf("%zu", runtime_.left_count));
        }
        if (dump_pickbuzzer_) {
          instrucciones.AddAttribute(
              "coge_zumbador", StringPrintf("%zu", runtime_.pickbuzzer_count));
        }
        if (dump_leavebuzzer_) {
          instrucciones.AddAttribute(
              "deja_zumbador", StringPrintf("%zu", runtime_.leavebuzzer_count));
        }
      }
    }
    ignore_result(write(STDOUT_FILENO, "\n", 1));
  }

  karel::Runtime* World::runtime() { return &runtime_; }

  void World::Init(size_t width, size_t height, std::string_view name) {
    width_ = width;
    height_ = height;
    name_ = std::string(name);
    program_name_ = "p1";
    buzzers_ = std::make_unique<uint32_t[]>(width_ * height_);
    walls_ = std::make_unique<uint8_t[]>(width_ * height_);
    buzzer_dump_ = std::make_unique<bool[]>(width_ * height_);
    for (size_t x = 0; x < width_; x++) {
      walls_[coordinates(x, 0)] |= 1 << 0x3;
      walls_[coordinates(x, height_ - 1)] |= 1 << 0x1;
    }
    for (size_t y = 0; y < height_; y++) {
      walls_[coordinates(0, y)] |= 1 << 0x0;
      walls_[coordinates(width_ - 1, y)] |= 1 << 0x2;
    }
    runtime_.width = width_;
    runtime_.height = height_;
    runtime_.buzzers = buzzers_.get();
    runtime_.walls = walls_.get();
  }

}
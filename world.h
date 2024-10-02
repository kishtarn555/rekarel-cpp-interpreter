#ifndef WORLD_H
#define WORLD_H

#include<string_view>
#include<cstdint>

#include "karel.h"
#include "logging.h"
#include "util.h"
#include "xml.h"

namespace karel {
    
    class World {
        public:
            World(World&& other);

            size_t coordinates(size_t x, size_t y) const;

            void set_buzzers(size_t x, size_t y, uint32_t count);

            uint32_t get_buzzers(size_t x, size_t y) const;

            uint8_t get_walls(size_t x, size_t y) const ;

            static std::optional<World> Parse(int fd);

            void Dump() const;

            void DumpResult(karel::RunResult result) const;

            karel::Runtime* runtime();

        private:
            World() = default;

            void Init(size_t width, size_t height, std::string_view name);

            size_t width_;
            size_t height_;
            std::string name_;
            std::string program_name_;
            std::unique_ptr<uint32_t[]> buzzers_;
            std::unique_ptr<uint8_t[]> walls_;
            std::unique_ptr<bool[]> buzzer_dump_;
            bool dump_world_ = false;
            bool dump_universe_ = false;
            bool dump_position_ = false;
            bool dump_orientation_ = false;
            bool dump_bag_ = false;
            bool dump_forward_ = false;
            bool dump_left_ = false;
            bool dump_leavebuzzer_ = false;
            bool dump_pickbuzzer_ = false;

            karel::Runtime runtime_;

            DISALLOW_COPY_AND_ASSIGN(World);
    };
}

#endif // WORLD_H
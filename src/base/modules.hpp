#pragma once
#include <cstdint>
#include <string>
namespace trace {
struct Module {std::uintptr_t base{};std::string path,build_id;};
Module module(const char* basename);
bool executable(const Module& module,std::uintptr_t address);
}

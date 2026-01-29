#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

using namespace std::literals;

namespace logger = SKSE::log;

namespace util {
    using SKSE::stl::report_and_fail;
}

#define DLLEXPORT __declspec(dllexport)

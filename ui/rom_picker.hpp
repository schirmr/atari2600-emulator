#pragma once

#include <optional>
#include <string>

class RomPicker {
public:
    std::optional<std::string> pickRomFromTestsDir(const std::string& testsDir);
};

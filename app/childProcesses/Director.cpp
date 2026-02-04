#include "Director.hpp"

#include <string>

Director::Director(int flag)
    : Process(ProcessType::DIRECTOR, {std::to_string(flag)}) {
}
#pragma once

#include "types.h"

#include <string>

struct GLFWwindow;

namespace tde {

class engine;

class platform
{
public:
    platform(engine* engine, const std::string& name);
    ~platform();

    const bool run_loop();

    void get_required_extensions(u32* count, const char*** names);
    GLFWwindow* window() { return _window; }

private:
    engine* _engine;
    GLFWwindow* _window;
};

}
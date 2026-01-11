#pragma once

#include <string>
#include <glad/glad.h>

GLuint compileShader(GLenum type, const char* src);
// initializes text rendering system with given window dimensions
void initText(int window_w, int window_h);
// renders the given text at specified position with given color
void renderText(
    const std::string& text,
    float x, float y,
    float r, float g, float b, float a
);

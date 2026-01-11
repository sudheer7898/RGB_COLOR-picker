#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "text_render.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

// =======================================================
// Simple shaders for text
// =======================================================

const char* text_vs = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // x,y,u,v
out vec2 TexCoord;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoord = vertex.zw;
}
)";

const char* text_fs = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D fontTex;
uniform vec4 textColor;

void main()
{
    float alpha = texture(fontTex, TexCoord).r;
    FragColor = vec4(textColor.rgb, textColor.a * alpha);
}
)";

// =======================================================

GLuint compileShader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(s, 1024, nullptr, log);
        std::cout << log << std::endl;
    }
    return s;
}

// =======================================================
// Global font data
// =======================================================

stbtt_bakedchar cdata[96]; // ASCII 32..126
GLuint fontTex;
GLuint VAO, VBO;
GLuint shader;
GLuint projectionLoc, colorLoc;

// =======================================================

void initText(int window_w,int window_h)
{
    // ---- load font file
    std::ifstream file("Arial.ttf", std::ios::binary);
    if (!file)
    {
        std::cout << "Arial.ttf not found\n";
        exit(1);
    }

    std::vector<unsigned char> ttf((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    // ---- bake font
    const int TEX_W = 512;
    const int TEX_H = 512;
    std::vector<unsigned char> bitmap(TEX_W * TEX_H);

    stbtt_BakeFontBitmap(
        ttf.data(), 0,
        32.0f,
        bitmap.data(), TEX_W, TEX_H,
        32, 96, cdata
    );

    // ---- upload texture
    glGenTextures(1, &fontTex);
    glBindTexture(GL_TEXTURE_2D, fontTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TEX_W, TEX_H, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // ---- shader
    GLuint vs = compileShader(GL_VERTEX_SHADER, text_vs);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, text_fs);

    shader = glCreateProgram();
    glAttachShader(shader, vs);
    glAttachShader(shader, fs);
    glLinkProgram(shader);

    glDeleteShader(vs);
    glDeleteShader(fs);

    // ---- buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ---- projection
    float ortho[16] = {
        2.0f / window_w, 0, 0, 0,
        0, -2.0f / window_h, 0, 0,
        0, 0, -1, 0,
        -1, 1, 0, 1
    };

    glUseProgram(shader);
    projectionLoc = glGetUniformLocation(shader, "projection");
    colorLoc = glGetUniformLocation(shader, "textColor");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, ortho);
}

// =======================================================

void renderText(const std::string& text, float x, float y, float r, float g, float b, float a)
{
    glUseProgram(shader);
    glUniform4f(colorLoc, r, g, b, a);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTex);
    glBindVertexArray(VAO);

    for (char c : text)
    {
        if (c < 32 || c > 126) continue;

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(cdata, 512, 512, c - 32, &x, &y, &q, 1);

        float verts[6][4] = {
            { q.x0, q.y0, q.s0, q.t0 },
            { q.x1, q.y0, q.s1, q.t0 },
            { q.x1, q.y1, q.s1, q.t1 },

            { q.x0, q.y0, q.s0, q.t0 },
            { q.x1, q.y1, q.s1, q.t1 },
            { q.x0, q.y1, q.s0, q.t1 }
        };

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

// =======================================================

//int main()
//{
//    glfwInit();
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//    GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Text Rendering", nullptr, nullptr);
//    glfwMakeContextCurrent(window);
//    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
//
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//    initText();
//
//    while (!glfwWindowShouldClose(window))
//    {
//        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        renderText("R: 255", 20, 50, 1, 0, 0, 1);
//        renderText("G: 128", 20, 80, 0, 1, 0, 1);
//        renderText("B: 64", 20, 110, 0, 0, 1, 1);
//        renderText("A: 200", 20, 140, 1, 1, 1, 1);
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    glfwTerminate();
//    return 0;
//}

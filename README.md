# OpenGL RGB Color Picker with Alpha Control

## Overview
This project is a **custom RGB color picker built entirely using OpenGL** as the graphics library.  
The application allows the user to visually select colors from an RGB/Hue-based circular picker and adjust the **alpha (transparency) value**, with the complete UI rendered manually using OpenGL primitives and shaders.

No external GUI frameworks (like ImGui, Qt, etc.) are used — the interface is created purely through OpenGL rendering and shader logic.

---

## Features
- 🎨 **RGB / Hue-based circular color picker**
- 🔄 **Real-time color selection**
- 🌈 **HSV to RGB color conversion inside fragment shader**
- 🧊 **Alpha (transparency) adjustment bar**
- 🖥️ **Text rendering using OpenGL**
- 🧩 **Fully custom UI rendered using OpenGL only**

---

## Technologies Used
- **C++**
- **OpenGL 3.3 Core Profile**
- **GLFW** – Window and input handling
- **GLAD** – OpenGL function loader
- **stb_truetype.h** – Font rasterization and text rendering
- **GLSL** – Vertex and Fragment shaders

---

## Text Rendering
Text rendering is implemented manually using **stb_truetype.h**, without relying on any external text or UI libraries.  
Font glyphs are rasterized and uploaded as textures, then rendered using OpenGL quads.

---

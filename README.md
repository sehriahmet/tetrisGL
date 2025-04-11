# 🧩 tetrisGL

A simple 3D Tetris-like game developed using **OpenGL**, **GLSL shaders**, and **C/C++**  
📚 Project for CENG477 - *Introduction to Computer Graphics*, Fall 2024, METU

---

## 🎮 Game Overview

`tetrisGL` is a 3D adaptation of the classic Tetris game. It features a single 3×3×3 block that falls into a 9×9 grid-based board. With full camera rotation and step-by-step block motion, the game allows you to experience Tetris from a fresh 3D perspective.

Built using **OpenGL**, **GLFW**, **GLEW**, and **GLM**, this project was designed to reinforce real-time graphics programming principles and interactive rendering.

---

## 🚀 Features

- 3×3×3 falling cube block
- 9×9 board grid with visible border lines
- Step-by-step block falling (not continuous)
- Left/right movement with respect to current camera view
- Smooth camera rotation around the game space
- Ambient and point lighting
- Blinn-Phong shading (ambient + diffuse + specular)
- Score tracking
- "Game Over" display on collision
- Standard Tetris rules:
  - Complete levels collapse
  - New blocks spawn from top-center
  - Game ends when block can't be placed

---

## 🎮 Controls

| Key | Action |
|-----|--------|
| `A` | Move block left (in current view) |
| `D` | Move block right (in current view) |
| `S` | Speed up fall |
| `W` | Slow down fall |
| `H` | Rotate view left |
| `K` | Rotate view right |

> Note: Changing the camera view changes how directions are interpreted.

---

## 💡 Shading & Lighting

- **GLSL (v330)** shaders
- **Blinn-Phong** lighting model
- Ambient + point light source(s)
- Lighting moves or adjusts with camera view

---

## ⚙️ Build & Run

### Requirements

- C++ compiler
- OpenGL
- GLFW
- GLEW
- GLM

### Steps

```bash
cd tetrisGL
make
./tetrisGL

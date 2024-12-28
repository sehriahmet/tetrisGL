#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <deque>

// Constants
const int GRID_SIZE = 9;
const float BLOCK_SIZE = 1.0f;
const glm::vec3 INITIAL_BLOCK_POSITION(4, GRID_SIZE - 1, 4);

// Game State Variables
std::deque<std::vector<std::vector<int>>> background(GRID_SIZE, std::vector<std::vector<int>>(GRID_SIZE, std::vector<int>(GRID_SIZE, 0)));
glm::vec3 activeBlockPosition = INITIAL_BLOCK_POSITION;
int score = 0;
bool gameOver = false;
bool gameStarted = false;
float fallSpeed = 0.5f;
float lastFallTime = 0.0f;
float cameraAngle = 0.0f;

// OpenGL Uniform Locations
GLint modelingMatrixLoc, viewingMatrixLoc, projectionMatrixLoc;

// Matrices
glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;

// Function Prototypes
void drawGrid();
void drawBlocks();
void moveBlock(int dx, int dz);
void spawnNewBlock();
void checkAndClearRows();
void updateBlockFall(float currentTime);
void renderText(const std::string& text, float x, float y, float scale, glm::vec3 color);
void initializeOpenGL();
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void reshape(GLFWwindow* window, int width, int height);

void drawGrid() {
    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int z = 0; z < GRID_SIZE; ++z) {
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0, z) * BLOCK_SIZE);
            glUniformMatrix4fv(modelingMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            // Render grid cell (placeholder)
        }
    }
}

void drawBlocks() {
    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            for (int z = 0; z < GRID_SIZE; ++z) {
                if (background[y][x][z]) {
                    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z) * BLOCK_SIZE);
                    glUniformMatrix4fv(modelingMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
                    // Render block (placeholder)
                }
            }
        }
    }
}



#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library ft;
FT_Face face;

void renderText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
    // Activate corresponding render state
    glUseProgram(0); // Use appropriate shader program for text rendering
    glUniform3f(glGetUniformLocation(0, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);

    float startX = x;

    // Iterate through all characters
    for (const char& c : text) {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            continue;
        }

        FT_GlyphSlot g = face->glyph;

        float xpos = x + g->bitmap_left * scale;
        float ypos = y - (g->bitmap.rows - g->bitmap_top) * scale;

        float w = g->bitmap.width * scale;
        float h = g->bitmap.rows * scale;

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, 0); // Bind appropriate texture
        // Use your vertex buffer here for rendering the quad with the glyph texture

        // Advance to the next character
        x += (g->advance.x >> 6) * scale;
    }
}

void moveBlock(int dx, int dz) {
    glm::vec3 newPosition = activeBlockPosition + glm::vec3(dx, 0, dz);
    if (newPosition.x >= 0 && newPosition.x < GRID_SIZE && newPosition.z >= 0 && newPosition.z < GRID_SIZE) {
        activeBlockPosition = newPosition;
    }
}

void spawnNewBlock() {
    activeBlockPosition = INITIAL_BLOCK_POSITION;
    if (background[activeBlockPosition.y][activeBlockPosition.x][activeBlockPosition.z]) {
        gameOver = true;
    }
}

void checkAndClearRows() {
    for (int y = 0; y < GRID_SIZE; ++y) {
        bool fullRow = true;
        for (int x = 0; x < GRID_SIZE; ++x) {
            for (int z = 0; z < GRID_SIZE; ++z) {
                if (!background[y][x][z]) {
                    fullRow = false;
                    break;
                }
            }
            if (!fullRow) break;
        }
        if (fullRow) {
            score++;
            background.erase(background.begin() + y);
            background.push_back(std::vector<std::vector<int>>(GRID_SIZE, std::vector<int>(GRID_SIZE, 0)));
        }
    }
}

void updateBlockFall(float currentTime) {
    if (!gameStarted) return;

    if (currentTime - lastFallTime > fallSpeed) {
        glm::vec3 newPosition = activeBlockPosition + glm::vec3(0, -1, 0);
        if (newPosition.y < 0 || background[newPosition.y][newPosition.x][newPosition.z]) {
            background[activeBlockPosition.y][activeBlockPosition.x][activeBlockPosition.z] = 1;
            checkAndClearRows();
            spawnNewBlock();
        } else {
            activeBlockPosition = newPosition;
        }
        lastFallTime = currentTime;
    }
}

void display(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set camera and projection
    glUniformMatrix4fv(viewingMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Draw elements
    drawGrid();
    drawBlocks();

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), activeBlockPosition * BLOCK_SIZE);
    glUniformMatrix4fv(modelingMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    // Render active block (placeholder)

    // Render text (e.g., score and game over messages)
    if (gameOver) {
        renderText("Game Over", 10, 10, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    renderText("Score: " + std::to_string(score), 10, 570, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

    glfwSwapBuffers(window);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_A: moveBlock(-1, 0); break;
            case GLFW_KEY_D: moveBlock(1, 0); break;
            case GLFW_KEY_W: fallSpeed = std::min(fallSpeed + 0.1f, 1.0f); break;
            case GLFW_KEY_S: fallSpeed = std::max(fallSpeed - 0.1f, 0.1f); break;
            case GLFW_KEY_H: cameraAngle -= 5.0f; break;
            case GLFW_KEY_K: cameraAngle += 5.0f; break;
            case GLFW_KEY_SPACE: gameStarted = true; break;
            case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        }
    }
}

void reshape(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
}

void initializeOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Load shaders, setup buffers, etc. (placeholders)
    modelingMatrixLoc = glGetUniformLocation(0, "modelingMatrix");
    viewingMatrixLoc = glGetUniformLocation(0, "viewingMatrix");
    projectionMatrixLoc = glGetUniformLocation(0, "projectionMatrix");

    viewingMatrix = glm::lookAt(glm::vec3(0.0f, 10.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "tetrisGL", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

    initializeOpenGL();

    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, reshape);

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        updateBlockFall(currentTime);
        display(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// GLEW loads OpenGL function pointers from the system's graphics drivers.
// glew.h MUST be included before gl.h
// clang-format off
#include <GL/glew.h>
#include <GL/gl.h>

// Disables inclusion of the dev-environ header.
// Allows GLFW + extension loader headers to be included in any order.
// GLFW including OpenGL headers causes function definition ambiguity.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
// clang-format on

// Maths library includes
#include "glm/vec3.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// -------
// Globals
// -------

constexpr int windowSize = 1000; // window is always square.
constexpr int gridSize = 250;
constexpr float cellSize = static_cast<float>(windowSize) / static_cast<float>(gridSize);
const std::string shaderPath = "../shader.glsl";

// ----------------------
// Helper structs & enums
// ----------------------

struct Vertex {
    // Vertex attributes
    glm::vec2 position;
    glm::vec3 colour;
};

struct ShaderProgramSource {
    std::string vertexSource;
    std::string fragmentSource;
};

enum class State {
    DEAD = 0,
    ALIVE = 1
};

struct Cell {
    glm::vec2 position;
    State state;

    // Allow for initializer list usage
    Cell(glm::vec2 m_position, State m_state)
        : position(m_position)
        , state(m_state) {};
};

// ------------------
// Program Management
// ------------------

void Initialize(GLFWwindow*& window);
int Exit(GLFWwindow* window);
void ProcessKeyboardInput(GLFWwindow* window, std::vector<Vertex>& buffer);
void FramebufferSizeCallback(GLFWwindow* window, int width, int height); // Adjust size of viewport

// ---------------------
// OpenGL Code
// ---------------------

void APIENTRY GLDebugPrintMessage(GLenum source, GLenum type, unsigned int id, GLenum severity, int length, const char* message, const void* data);
uint32_t CreateVAO();
void CreateVBO();
std::vector<uint32_t> CreateIBO();
uint32_t CreateShader(const std::string_view shaderPath);
void SpecifyLayout();
void Render(GLFWwindow*& window, const uint32_t& VAO, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, uint32_t shader);

// ----------------
// Shader Functions
// ----------------

ShaderProgramSource ParseShader(const std::string_view filepath);
uint32_t CompileShader(uint32_t shaderType, const std::string_view shaderSource);
uint32_t CreateShader(ShaderProgramSource& shaderSource);

// ------------------
// Game of Life Functions
// ------------------

std::vector<Vertex> CreateCell(Cell cell);
State GetCellState(const std::vector<Vertex>& buffer, glm::vec2 position);
void SetCellState(std::vector<Vertex>& buffer, Cell cell);
void GenerateRandomCells(std::vector<Vertex>& buffer);
void GameOfLife(std::vector<Vertex>& buffer);
void RestartGame(std::vector<Vertex>& buffer);

int main()
{
    GLFWwindow* window = nullptr;
    Initialize(window);

    constexpr int nVerticesPerCell = 4;
    constexpr int nVertices = (gridSize * gridSize) * nVerticesPerCell;

    const uint32_t VAO = CreateVAO();
    CreateVBO();
    const std::vector<uint32_t> cellIndices = CreateIBO();
    SpecifyLayout();
    const uint32_t shader = CreateShader(shaderPath);

    std::vector<Vertex> cellVertices;
    cellVertices.reserve(nVertices);

    GenerateRandomCells(cellVertices);

    while (!glfwWindowShouldClose(window)) {
        Render(window, VAO, cellVertices, cellIndices, shader);

        // Update Game of Life every frame.
        GameOfLife(cellVertices);

        // Restart game if space key is pressed
        ProcessKeyboardInput(window, cellVertices);
    }

    Exit(window);
}

// ------------------
// Program Management
// ------------------

void Initialize(GLFWwindow*& window)
{
    // GLFW Setup
    if (!glfwInit()) {
        std::cout << "GLFW Initialization failed!\n"
                  << "Exiting...\n";

        exit(EXIT_FAILURE);
    }

    // Use OpenGL 4.6 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // For OpenGL Debugging
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    // GLFW Options
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create window
    window = glfwCreateWindow(windowSize, windowSize, "Glautomata - John Conway's Game of Life", nullptr, nullptr);

    if (window == nullptr) {
        std::cout << "GLFW window creation failed\n"
                  << "Exiting...\n ";
        exit(EXIT_FAILURE);
    }

    // Create OpenGL context
    glfwMakeContextCurrent(window);

    // A valid OpenGL context must be created before initializing GLEW.
    // Initialize OpenGL loader (GLEW in this project).
    bool error = glewInit();
    if (error != GLEW_OK) {
        std::cout << "Error: Failed to initialize OpenGL function pointer loader!\n";
    }

    // Enable debugging layer of OpenGL
    int glFlags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &glFlags);
    if (glFlags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        glDebugMessageCallback(GLDebugPrintMessage, nullptr);

        std::cout << ("OpenGL Debug Mode\n");
    } else {
        std::cout << "Debug for OpenGL not supported by the system!\n";
    }
}

int Exit(GLFWwindow* window)
{
    glfwDestroyWindow(window);
    glfwTerminate();

    std::exit(EXIT_SUCCESS);
}

void ProcessKeyboardInput(GLFWwindow* window, std::vector<Vertex>& buffer)
{
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        RestartGame(buffer);
    }
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Ensure viewport matches new window dimensions
    glViewport(0, 0, width, height);
}

// ---------------------
// OpenGL Code
// ---------------------

// Callback function for printing OpenGL debug statements.
// Note that OpenGL Debug Output must be enabled to utilize glDebugMessageCallback() and consequently this function.
void APIENTRY GLDebugPrintMessage(GLenum source, GLenum type, unsigned int id, GLenum severity, int length, const char* message, const void* data)
{
    // To enable the debugging layer of OpenGL:

    // glEnable(GL_DEBUG_OUTPUT); - This is a faster version but there are no debugger breakpoints.
    // glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); - Callback is synchronized w/ errors, and a breakpoint can be placed on the callback to get a stacktrace for the GL error.

    // Followed by the call:
    // glDebugMessageCallback(glDebugPrintMessage, nullptr);

    std::string sourceMessage = "";
    std::string typeMessage = "";
    std::string severityMessage = "";

    switch (source) {
    case GL_DEBUG_SOURCE_API: {
        sourceMessage = "API";
        break;
    }
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
        sourceMessage = "WINDOW SYSTEM";
        break;
    }
    case GL_DEBUG_SOURCE_SHADER_COMPILER: {
        sourceMessage = "SHADER COMPILER";
        break;
    }
    case GL_DEBUG_SOURCE_THIRD_PARTY: {
        sourceMessage = "THIRD PARTY";
        break;
    }
    case GL_DEBUG_SOURCE_APPLICATION: {
        sourceMessage = "APPLICATION";
        break;
    }
    case GL_DEBUG_SOURCE_OTHER: {
        sourceMessage = "UNKNOWN";
        break;
    }
    default: {
        sourceMessage = "UNKNOWN";
        break;
    }
    }

    switch (type) {
    case GL_DEBUG_TYPE_ERROR: {
        typeMessage = "ERROR";
        break;
    }
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
        typeMessage = "DEPRECATED BEHAVIOUR";
        break;
    }
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
        typeMessage = "UNDEFINED BEHAVIOUR";
        break;
    }
    case GL_DEBUG_TYPE_PORTABILITY: {
        typeMessage = "PORTABILITY";
        break;
    }
    case GL_DEBUG_TYPE_PERFORMANCE: {
        typeMessage = "PERFORMANCE";
        break;
    }
    case GL_DEBUG_TYPE_OTHER: {
        typeMessage = "OTHER";
        break;
    }
    case GL_DEBUG_TYPE_MARKER: {
        typeMessage = "MARKER";
        break;
    }
    default: {
        typeMessage = "UNKNOWN";
        break;
    }
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH: {
        severityMessage = "HIGH";
        break;
    }
    case GL_DEBUG_SEVERITY_MEDIUM: {
        severityMessage = "MEDIUM";
        break;
    }
    case GL_DEBUG_SEVERITY_LOW: {
        severityMessage = "LOW";
        break;
    }
    case GL_DEBUG_SEVERITY_NOTIFICATION: {
        severityMessage = "NOTIFICATION";
        break;
    }
    default: {
        severityMessage = "UNKNOWN";
        break;
    }
    }

    std::cout << id << ": " << typeMessage << " of " << severityMessage << ", raised from "
              << sourceMessage << ": " << message << std::endl;
}

uint32_t CreateVAO()
{
    constexpr int nBuffers = 1;

    // Create Vertex Array Object
    uint32_t VAO = 0;
    glGenVertexArrays(nBuffers, &VAO);
    glBindVertexArray(VAO);

    return VAO;
}

void CreateVBO()
{
    constexpr int nVerticesPerCell = 4;
    constexpr int nVertices = (gridSize * gridSize) * nVerticesPerCell;
    constexpr int nBuffers = 1;
    const int nVertexBytes = nVertices * sizeof(Vertex);

    // Create Vertex Buffer Object
    uint32_t VBO = 0;
    glGenBuffers(nBuffers, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertexBytes, nullptr, GL_DYNAMIC_DRAW); // passed in nullptr as data will be copied later.
}

std::vector<uint32_t> CreateIBO()
{
    constexpr int nBuffers = 1;
    constexpr int nVerticesPerCell = 4;

    std::vector<uint32_t> indices;
    for (int index = 0; index < std::pow(gridSize, 2); ++index) {
        // The final digit in the expression is the pattern of the indices for each cell
        indices.push_back((index * nVerticesPerCell) + 0);
        indices.push_back((index * nVerticesPerCell) + 1);
        indices.push_back((index * nVerticesPerCell) + 2);
        indices.push_back((index * nVerticesPerCell) + 0);
        indices.push_back((index * nVerticesPerCell) + 2);
        indices.push_back((index * nVerticesPerCell) + 3);
    }

    // Create Index Buffer Object
    uint32_t IBO = 0;
    glGenBuffers(nBuffers, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    return indices;
}

uint32_t CreateShader(const std::string_view shaderPath)
{
    ShaderProgramSource shaderSource = ParseShader(shaderPath);

    const uint32_t shader = CreateShader(shaderSource);
    glUseProgram(shader);

    return shader;
}

void SpecifyLayout()
{
    // Link currently bound VAO with the currently bound VBO, and define its layout.
    constexpr int positionAttribute = 0;
    constexpr int colourAttribute = 1;

    constexpr int nFloatsInAttribute = 3;
    constexpr int stride = sizeof(Vertex);
    const void* const positionOffset = (void*)offsetof(struct Vertex, position); // (void*) as the OpenGL API requires it.
    const void* const colourOffset = (void*)offsetof(struct Vertex, colour);

    glVertexAttribPointer(positionAttribute, nFloatsInAttribute, GL_FLOAT, GL_FALSE, stride, positionOffset);
    glEnableVertexAttribArray(positionAttribute);

    glVertexAttribPointer(colourAttribute, nFloatsInAttribute, GL_FLOAT, GL_FALSE, stride, colourOffset);
    glEnableVertexAttribArray(colourAttribute);
}

void Render(GLFWwindow*& window, const uint32_t& VAO, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, uint32_t shader)
{
    // Set dynamic buffer
    glBindBuffer(GL_ARRAY_BUFFER, VAO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

    // Clear screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set viewport size to window size
    static int currentWindowWidth = 0;
    static int currentWindowHeight = 0;
    glfwGetWindowSize(window, &currentWindowWidth, &currentWindowHeight);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    // Orthographic project matrix.
    const glm::mat4 projection = glm::ortho(0.0f, (float)currentWindowWidth, 0.0f, (float)currentWindowHeight, 0.0f, 100.0f);
    constexpr int nElements = 1;
    glUniformMatrix4fv(glGetUniformLocation(shader, "u_MVP"), nElements, GL_FALSE, &projection[0][0]);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

    // Update screen
    glfwSwapBuffers(window);
    glfwPollEvents();
}

// ----------------
// Shader Functions
// ----------------

ShaderProgramSource ParseShader(const std::string_view filepath)
{
    // For separating the two stringstreams.
    enum class ShaderType {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    std::ifstream stream(filepath.data());
    std::array<std::stringstream, 2> ss;

    std::string line = "";
    ShaderType type = ShaderType::NONE;

    // Read lines from the file while separating the two shader types into different stringstreams.
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::VERTEX;
            } else if (line.find("fragment") != std::string::npos) {
                type = ShaderType::FRAGMENT;
            }
        } else {
            ss[static_cast<int>(type)] << line << "\n";
        }
    }

    ShaderProgramSource shaders;
    shaders.vertexSource = ss[0].str();
    shaders.fragmentSource = ss[1].str();

    return shaders;
}

uint32_t CompileShader(uint32_t shaderType, const std::string_view shaderSource)
{
    uint32_t id = glCreateShader(shaderType);

    const char* src = shaderSource.data();

    constexpr int nShaderSources = 1;
    glShaderSource(id, nShaderSources, &src, nullptr);
    glCompileShader(id);

    // Error handling.
    int result = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int errorMessageLength = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &errorMessageLength);
        std::string message = "";
        glGetShaderInfoLog(id, errorMessageLength, &errorMessageLength, message.data());

        // Simple logging
        std::cout << "Failed to compile " << (shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!\n"
                  << message << std::endl;

        glDeleteShader(id);

        // id set to 0 as shader was not compiled
        id = 0;
    }

    return id;
}

uint32_t CreateShader(ShaderProgramSource& source)
{
    const uint32_t program = glCreateProgram();
    const uint32_t vs = CompileShader(GL_VERTEX_SHADER, source.vertexSource);
    const uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, source.fragmentSource);

    // These steps create an executable that is run on the programmable vertex/fragment shader processer on the GPU.
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    // Delete to shaders once they have been linked and compiled.
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

// ------------------
// Game of Life Functions
// ------------------

std::vector<Vertex> CreateCell(Cell cell)
{
    constexpr glm::vec3 colourBlack = { 0.0f, 0.0f, 0.0f };
    constexpr glm::vec3 colourWhite = { 1.0f, 1.0f, 1.0f };

    std::vector<Vertex> cellVertices(4);
    const glm::vec3 cellColour = static_cast<bool>(cell.state) ? colourWhite : colourBlack;

    // Adjust each position for the size of a cell
    cell.position *= cellSize;

    // Vertex Buffer data
    cellVertices[0].position = { cell.position.x, cell.position.y };
    cellVertices[0].colour = cellColour;
    cellVertices[1].position = { cell.position.x + cellSize, cell.position.y };
    cellVertices[1].colour = cellColour;
    cellVertices[2].position = { cell.position.x + cellSize, cell.position.y + cellSize };
    cellVertices[2].colour = cellColour;
    cellVertices[3].position = { cell.position.x, cell.position.y + cellSize };
    cellVertices[3].colour = cellColour;

    return cellVertices;
}

State GetCellState(const std::vector<Vertex>& buffer, glm::vec2 position)
{
    constexpr glm::vec3 colourWhite = { 1.0f, 1.0f, 1.0f };
    constexpr int nVertices = 4;

    State state = State::DEAD;

    if (position.x >= 0 && position.y >= 0) {
        // Convert 2D position to 1D array
        const uint32_t index = ((position.x * gridSize) + position.y) * nVertices;

        if (index < buffer.size()) {
            if (buffer.at(index).colour == colourWhite) {
                state = State::ALIVE;
            }
        }
    }
    return state;
}

void SetCellState(std::vector<Vertex>& buffer, Cell cell)
{
    constexpr glm::vec3 colourBlack = { 0.0f, 0.0f, 0.0f };
    constexpr glm::vec3 colourWhite = { 1.0f, 1.0f, 1.0f };
    constexpr int nVertices = 4;

    const uint32_t index = ((cell.position.x * gridSize) + cell.position.y) * nVertices;

    if (index < buffer.size()) {
        for (int x = 0; x < 4; ++x) {
            buffer.at(index + x).colour = static_cast<bool>(cell.state) ? colourWhite : colourBlack;
        }
    }
}

void GenerateRandomCells(std::vector<Vertex>& buffer)
{
    // Seed srand() with the current time.
    std::srand(std::time(nullptr));

    for (int x = 0; x < gridSize; ++x) {
        for (int y = 0; y < gridSize; ++y) {
            auto cell = CreateCell({ { x, y }, rand() % 2 ? State::ALIVE : State::DEAD });
            buffer.insert(buffer.end(), cell.begin(), cell.end());
        }
    }
}

void GameOfLife(std::vector<Vertex>& buffer)
{
    // Write to tempBuffer while reading "cells" in buffer
    std::vector<Vertex> tempBuffer;
    tempBuffer.reserve(std::pow(gridSize, 2) * 4);

    // Iterate over grid of cells.
    for (int cellPosX = 0; cellPosX < gridSize; ++cellPosX) {
        for (int cellPosY = 0; cellPosY < gridSize; ++cellPosY) {

            int nAliveNeighbours = 0;
            for (int neighbourIndex_X = -1; neighbourIndex_X <= 1; ++neighbourIndex_X) {
                for (int neighbourIndex_Y = -1; neighbourIndex_Y <= 1; ++neighbourIndex_Y) {

                    // Don't check {0, 0} as that's the current cell.
                    if (neighbourIndex_X != 0 || neighbourIndex_Y != 0) {

                        const int neighbourPox_X = cellPosX + neighbourIndex_X;
                        const int neighbourPos_Y = cellPosY + neighbourIndex_Y;

                        const State neighbourState = GetCellState(buffer, { neighbourPox_X, neighbourPos_Y });

                        if (neighbourState == State::ALIVE) {
                            ++nAliveNeighbours;
                        }
                    }
                }
            }

            const State currentCellState = GetCellState(buffer, { cellPosX, cellPosY });
            State newCellState = currentCellState;

            switch (currentCellState) {
            case (State::ALIVE): {
                if (nAliveNeighbours < 2 || 3 < nAliveNeighbours) {
                    newCellState = State::DEAD; // Cell dies via underpopulation or overpopulation.
                } else if (nAliveNeighbours == 2 || nAliveNeighbours == 3) {
                    newCellState = State::ALIVE; // Cell is happy and remains alive :)
                }
                break;
            }
            case (State::DEAD): {
                if (nAliveNeighbours == 3) {
                    newCellState = State::ALIVE; // Cells reproduce to create an alive cell.
                }
                break;
            }
            }

            const auto tempCell = CreateCell({ { cellPosX, cellPosY }, newCellState });
            tempBuffer.insert(tempBuffer.end(), tempCell.begin(), tempCell.end());
        }
    }

    // Update buffer with the updated cell states.
    buffer = std::move(tempBuffer);
}

void RestartGame(std::vector<Vertex>& buffer)
{
    buffer.erase(buffer.begin(), buffer.end());
    GenerateRandomCells(buffer);
}
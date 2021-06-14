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
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

constexpr int windowWidth = 1280;
constexpr int windowHeight = 720;
const std::string shaderPath = "../shader.glsl";
constexpr int gridSize = 10;

// ------------------
// Program Management
// ------------------

void Initialize(GLFWwindow*& window);
void Run();
int Exit(GLFWwindow* window);

void FramebufferSizeCallback(GLFWwindow* window, int width, int height); // Adjust size of viewport

// ---------------------
// OpenGL Error Handling
// ---------------------

void APIENTRY glDebugPrintMessage(GLenum source, GLenum type, unsigned int id, GLenum severity, int length, const char* message, const void* data);

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
    Cell(glm::vec2 position_, State state_)
        : position(position_)
        , state(state_) {};
};

// ----------------
// Shader Functions
// ----------------

ShaderProgramSource parseShader(const std::string& filepath);
uint32_t compileShader(uint32_t shaderType, std::string& shaderSource);
uint32_t createShader(ShaderProgramSource& shaderSource);

// ------------------
// Automata Functions
// ------------------

std::vector<Vertex> CreateCell(Cell cell);

int main()
{
    GLFWwindow* window = nullptr;
    Initialize(window);

    constexpr int nBuffers = 1;

    // Create Vertex Array Object
    uint32_t VAO = 0;
    glGenVertexArrays(nBuffers, &VAO);
    glBindVertexArray(VAO);

    const int nVertices = std::pow(gridSize, 2) * 4;
    const int nVertexBytes = nVertices * sizeof(Vertex);

    // Create Vertex Buffer Object
    uint32_t VBO = 0;
    glGenBuffers(nBuffers, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertexBytes, nullptr, GL_DYNAMIC_DRAW); // passed in nullptr as data will be copied later.

    // Index Buffer data
    std::vector<uint32_t> indices = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7
    };
    // Create Index Buffer Object
    uint32_t IBO = 0;
    glGenBuffers(nBuffers, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    // Link current VAO with the VBO, and define its layout
    constexpr int positionAttribute = 0;
    constexpr int colourAttribute = 1;

    constexpr int nFloatsInAttribute = 3;
    constexpr int stride = sizeof(Vertex);
    const void* positionOffset = (void*)offsetof(struct Vertex, position); // (void*) as the OpenGL API requires it.
    const void* colourOffset = (void*)offsetof(struct Vertex, colour);

    glVertexAttribPointer(positionAttribute, nFloatsInAttribute, GL_FLOAT, GL_FALSE, stride, positionOffset);
    glEnableVertexAttribArray(positionAttribute);

    glVertexAttribPointer(colourAttribute, nFloatsInAttribute, GL_FLOAT, GL_FALSE, stride, colourOffset);
    glEnableVertexAttribArray(colourAttribute);

    // Shader creation
    ShaderProgramSource shaderSource = parseShader(shaderPath);

    uint32_t shader = createShader(shaderSource);
    glUseProgram(shader);

    auto c0 = CreateCell({ { 0, 0 }, State::DEAD });
    auto c1 = CreateCell({ { 100, 100 }, State::ALIVE });

    std::vector<Vertex> vertices;
    vertices.reserve(nVertices);
    vertices.insert(vertices.end(), c0.begin(), c0.end());
    vertices.insert(vertices.end(), c1.begin(), c1.end());

    while (!glfwWindowShouldClose(window)) {
        // Set dynamic buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
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
        glm::mat4 projection = glm::ortho(0.0f, (float)currentWindowWidth, 0.0f, (float)currentWindowHeight, 0.0f, 100.0f);
        constexpr int nElements = 1;
        glUniformMatrix4fv(glGetUniformLocation(shader, "u_MVP"), nElements, GL_FALSE, &projection[0][0]);

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

        // Update screen
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shader);
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

    // Create window
    window = glfwCreateWindow(windowWidth, windowHeight, "Glautomata - John Conway's Game of Life", NULL, NULL);

    if (window == NULL) {
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

        glDebugMessageCallback(glDebugPrintMessage, nullptr);

        std::cout << ("OpenGL Debug Mode\n");
    } else {
        std::cout << "Debug for OpenGL not supported by the system!\n";
    }
}

void Run()
{
}

int Exit(GLFWwindow* window)
{
    glfwDestroyWindow(window);
    glfwTerminate();

    std::exit(EXIT_SUCCESS);
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Ensure viewport matches new window dimensions
    glViewport(0, 0, width, height);
}

// ---------------------
// OpenGL Error Handling
// ---------------------

// Callback function for printing OpenGL debug statements.
// Note that OpenGL Debug Output must be enabled to utilize glDebugMessageCallback() and consequently this function.
void APIENTRY glDebugPrintMessage(GLenum source, GLenum type, unsigned int id, GLenum severity, int length, const char* message, const void* data)
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

// ----------------
// Shader Functions
// ----------------

ShaderProgramSource parseShader(const std::string& filepath)
{
    // For separating the two stringstreams.
    enum class ShaderType {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    std::ifstream stream(filepath);
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

uint32_t compileShader(uint32_t shaderType, std::string& shaderSource)
{
    uint32_t id = glCreateShader(shaderType);

    const char* src = shaderSource.c_str();

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

uint32_t createShader(ShaderProgramSource& source)
{
    uint32_t program = glCreateProgram();
    uint32_t vs = compileShader(GL_VERTEX_SHADER, source.vertexSource);
    uint32_t fs = compileShader(GL_FRAGMENT_SHADER, source.fragmentSource);

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
// Automata Functions
// ------------------

std::vector<Vertex> CreateCell(Cell cell)
{
    constexpr float size = 100.0f;
    constexpr glm::vec3 colourBlack = { 0.0f, 0.0f, 0.0f };
    constexpr glm::vec3 colourWhite = { 1.0f, 1.0f, 1.0f };

    std::vector<Vertex> cellVertices(4);
    glm::vec3 cellColour = static_cast<bool>(cell.state) ? colourWhite : colourBlack;

    // Vertex Buffer data
    cellVertices[0].position = { cell.position.x, cell.position.y };
    cellVertices[0].colour = cellColour;
    cellVertices[1].position = { cell.position.x + size, cell.position.y };
    cellVertices[1].colour = cellColour;
    cellVertices[2].position = { cell.position.x + size, cell.position.y + size };
    cellVertices[2].colour = cellColour;
    cellVertices[3].position = { cell.position.x, cell.position.y + size };
    cellVertices[3].colour = cellColour;

    return cellVertices;
}
#include <stdio.h>
#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "assrt.h"
#include "shader.h"

bool verbose = true;

static void gl_debug_messenger([[maybe_unused]] GLenum source, GLenum type,
    [[maybe_unused]] GLuint id, GLenum severity,
    [[maybe_unused]] GLsizei length,
    const GLchar* message,
    [[maybe_unused]] const void* user_param) {

    const char* type_str = nullptr;
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        type_str = "Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        type_str = "Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        type_str = "Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        type_str = "Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        type_str = "Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        type_str = "Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        type_str = "Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        type_str = "Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        type_str = "Other";
        break;
    }

    const char* severity_str = nullptr;
    switch (severity) {

    case GL_DEBUG_SEVERITY_HIGH:
        severity_str = "High";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severity_str = "Medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        severity_str = "Low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        severity_str = "Notification";
        break;
    }

    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    printf("GL: [%s] [%s] %s\n", type_str, severity_str, message);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (verbose) printf("GLFW: Resized to : (%d, %d)\n", width, height); 
}

struct input_frame {
    int space;
    bool wireframe;
};

void process_input(GLFWwindow* window, input_frame* last_frame) {
    auto new_frame = (input_frame) {
        .space = glfwGetKey(window, GLFW_KEY_SPACE),
        .wireframe = last_frame->wireframe
    };

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || 
            glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    } 

    if (new_frame.space == GLFW_PRESS && 
            last_frame->space == GLFW_RELEASE) {
        new_frame.wireframe = !last_frame->wireframe;
        int key = new_frame.wireframe ? GL_LINE : GL_FILL;
        glPolygonMode(GL_FRONT_AND_BACK, key);
    }
    *last_frame = new_frame; 
}

void set_color(GLuint shader_program) {
    auto time = glfwGetTime();
    auto r = (sin(time * 2) / 2.f) + 0.5f;
    auto g = (sin(time) / 2.f) + 0.5f;
    auto b = (sin(time / 2) / 2.f) + 0.5f;
    auto vert_location = glGetUniformLocation(shader_program, "prog_color");
    glUniform4f(vert_location, r, g, b, 1.0f);
}

void render(GLFWwindow* window, Shader* shader, GLuint vao) {
    glClearColor(1.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    shader->use();
    // set_color(shader_program);
    // glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(window);
}

void render_init(GLFWwindow* window, Shader* shader, GLuint &vao) {
    float vertices[] = {
        // positions          // colors
        -0.5f, -0.5f, 0.0f,   1.f, 0.f, 0.f,
         0.5f, -0.5f, 0.0f,   0.f, 1.f, 0.f,
        -0.5f,  0.5f, 0.0f,   0.f, 0.f, 1.f,
         0.5f,  0.5f, 0.0f,   1.f, 1.f, 1.f
    };
    unsigned int indices[] = {
        0, 3, 2,
        1, 3, 0
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int ebo; // element buffer object
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    unsigned int vbo; // vertex buffer object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
   
    shader->configure("shaders/shader.vs", "shaders/shader.fs");
    if (verbose) printf("Using shader {%d}\n", shader->ID);
   
    // intepret the vertex data (per vertex attribute)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    if (verbose) {
        int nr_attributes;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nr_attributes);
        printf("Maximum # of vertex attributes supported: {%d}\n", nr_attributes);
    }
}
int main() {

    // init window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "LearnOpenGL", NULL, NULL);
    assrt(window != NULL, "Failed to create GLFW window");

    glfwMakeContextCurrent(window);
    assrt(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to initialize GLAD");

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (GLAD_GL_KHR_debug) {
        glDebugMessageCallback(gl_debug_messenger, nullptr);
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }

    Shader shader;
    unsigned int vao;
    render_init(window, &shader, vao);

    auto frame = (input_frame) {
        .space = 0,
        .wireframe = false
    };

    while (!glfwWindowShouldClose(window)) { // render loop
        process_input(window, &frame);
        render(window, &shader, vao);
        glfwPollEvents();
        auto error = glGetError();
        if (error) {
            //printf("%d\n", error);
        }
    }
    glfwTerminate();
    return 0;
}

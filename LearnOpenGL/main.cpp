#include <stdio.h>
#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "assrt.h"
#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

bool verbose = true;

const char* vert_path = "data/shaders/shader.vert";
const char* frag_path = "data/shaders/shader.frag";
const char* image_path = "data/images/container.jpeg";
const char* image2_path = "data/images/awesomeface.png";

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

void update_color(Shader* shader, float time) {
    auto r = (sin(time * 2) / 2.f) + 0.5f;
    auto g = (sin(time) / 2.f) + 0.5f;
    auto b = (sin(time / 2) / 2.f) + 0.5f;
    auto vert_location = glGetUniformLocation(shader->ID, "prog_color");
    glUniform4f(vert_location, r, g, b, 1.0f);
}

void update_transform(Shader* shader, float time) {
    glm::mat4 trs = glm::mat4(1.f);
    trs = glm::rotate(trs, glm::radians(time * 200), glm::vec3(0.f, 1.f, 0.f));
    auto scale = sin(time) / 3.f + 0.5f;
    trs = glm::scale(trs, glm::vec3(scale, scale, scale));
    
    unsigned int trs_location = glGetUniformLocation(shader->ID, "trs");
    glad_glUniformMatrix4fv(trs_location, 1, GL_FALSE, glm::value_ptr(trs));
}

void render(GLFWwindow* window, Shader* shader, GLuint vao) {
    glClearColor(1.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    shader->use();
    shader->seti("tex", 0);
    shader->seti("tex2", 1);
    
    auto time = glfwGetTime(); 
    update_transform(shader, time);
    update_color(shader, time);

    // glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(window);
}

void load_texture(const char* path, GLenum image_format, GLenum active_texture) {
    // texture wrapping config
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRROR_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRROR_CLAMP_TO_EDGE);

    // filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // same but mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // magnification wouldn't need (can't) mipmap interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, number_of_color_channels;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &number_of_color_channels, 0);

    assrt(data, "Failed to load texture {%s}", path);

    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(active_texture); 
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, image_format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    
    stbi_image_free(data);
}

void render_init(GLFWwindow* window, Shader* shader, GLuint &vao) {
    float vertices[] = {
        // positions          //colors          // texture coordinates
        -0.5f, -0.5f, 0.0f,   1.f, 0.f, 0.f,    0.f, 0.f,      
         0.5f, -0.5f, 0.0f,   0.f, 1.f, 0.f,    1.f, 0.f,
        -0.5f,  0.5f, 0.0f,   0.f, 0.f, 1.f,    0.f, 1.f,
         0.5f,  0.5f, 0.0f,   1.f, 1.f, 1.f,    1.f, 1.f
    };
    unsigned int indices[] = {
        0, 3, 2,
        1, 3, 0
    };

    load_texture(image_path, GL_RGB, GL_TEXTURE0);
    load_texture(image2_path, GL_RGBA, GL_TEXTURE1);

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
   
    shader->configure(vert_path, frag_path);
    if (verbose) printf("Using shader {%d}\n", shader->ID);
   
    // intepret the vertex data (per vertex attribute)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


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

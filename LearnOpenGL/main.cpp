#include <stdio.h>
#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "assrt.h"
#include "glm/fwd.hpp"
#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

bool verbose = true;

const char* vert_path = "data/shaders/shader.vert";
const char* frag_path = "data/shaders/shader.frag";
const char* image_path = "data/images/container.jpeg";
const char* image2_path = "data/images/awesomeface.png";

struct program_state {
    bool wireframe;
    bool perspective;

    float width;
    float height;
    glm::vec3 world_offset;
};

program_state state = (program_state) {
    .perspective = true,
    .wireframe = false,
    .width = 800,
    .height = 800,
    .world_offset = glm::vec3(0.f, 0.f, -0.3f),
};

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
    state.height = height;
    state.width = width;
    glViewport(0, 0, width, height);
    if (verbose) printf("GLFW: Resized to : (%d, %d)\n", width, height); 
}

struct input_frame {
    int space;
    int p;
    int w,a,s,d,q,e;
};


void verbose_toggle(const char* var_name, bool var_value) {
    if (verbose) printf("%s: {%s}\n", var_name, (var_value ? "ON" : "OFF"));
}

void process_input(GLFWwindow* window, input_frame* last_frame) {
    auto new_frame = (input_frame) {
        .space = glfwGetKey(window, GLFW_KEY_SPACE),
        .p = glfwGetKey(window, GLFW_KEY_P),
        .w = glfwGetKey(window, GLFW_KEY_W),
        .a = glfwGetKey(window, GLFW_KEY_A),
        .s = glfwGetKey(window, GLFW_KEY_S),
        .d = glfwGetKey(window, GLFW_KEY_D),
        .q = glfwGetKey(window, GLFW_KEY_Q),
        .e = glfwGetKey(window, GLFW_KEY_E),
    };

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { 
        glfwSetWindowShouldClose(window, true);
    } 

    if (new_frame.space == GLFW_PRESS && 
            last_frame->space == GLFW_RELEASE) {
        state.wireframe = !state.wireframe;
        int key = state.wireframe ? GL_LINE : GL_FILL;
        glPolygonMode(GL_FRONT_AND_BACK, key);
        verbose_toggle("Wireframe", state.wireframe);
    }

    if (new_frame.p == GLFW_PRESS &&
            last_frame->p == GLFW_RELEASE) {
        state.perspective = !state.perspective;
        // effect happens next frame in render_loop
        verbose_toggle("Perspective", state.perspective);
    }

    if (new_frame.w == GLFW_PRESS) {
        state.world_offset = state.world_offset + glm::vec3(0, 0, 0.1f);
    }
    if (new_frame.a == GLFW_PRESS) {
        state.world_offset = state.world_offset + glm::vec3(0.1f, 0, 0);
    }
    if (new_frame.s == GLFW_PRESS) {
        state.world_offset = state.world_offset + glm::vec3(0, 0, -0.1f);
    }
    if (new_frame.d == GLFW_PRESS) {
        state.world_offset = state.world_offset + glm::vec3(-0.1f, 0, 0);
    }
    if (new_frame.q == GLFW_PRESS) {
        state.world_offset = state.world_offset + glm::vec3(0, 0.1f, 0);
    }
    if (new_frame.e == GLFW_PRESS) {
        state.world_offset = state.world_offset + glm::vec3(0, -0.1f, 0);
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

void update_transform_fun(Shader* shader, float time) {
    glm::mat4 trs = glm::mat4(1.f);
    trs = glm::rotate(trs, glm::radians(time * 200), glm::vec3(0.f, 1.f, 0.f));
    auto scale = sin(time) / 3.f + 0.5f;
    trs = glm::scale(trs, glm::vec3(scale, scale, scale));
    
    unsigned int trs_location = glGetUniformLocation(shader->ID, "trs");
    glUniformMatrix4fv(trs_location, 1, GL_FALSE, glm::value_ptr(trs));
}

void update_draw_transform(Shader* shader, float time) {
    glm::vec3 cube_positions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f ), 
        glm::vec3( 2.0f,  5.0f, -15.0f ), 
        glm::vec3(-1.5f, -2.2f, -2.5f),  
        glm::vec3(-3.8f, -2.0f, -12.3f),  
        glm::vec3( 2.4f, -0.4f, -3.5f ),  
        glm::vec3(-1.7f,  3.0f, -7.5f),  
        glm::vec3( 1.3f, -2.0f, -2.5f ),  
        glm::vec3( 1.5f,  2.0f, -2.5f ), 
        glm::vec3( 1.5f,  0.2f, -1.5f ), 
        glm::vec3(-1.3f,  1.0f, -1.5f)  
    };

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, state.world_offset); 
    shader->setmat4("view", view);

    glm::mat4 projection;
    projection = state.perspective 
        ? glm::perspective(glm::radians(45.f), 1.f, 0.1f, 100.f)
        : glm::ortho(0.f, state.width, 0.f, state.height, 0.1f, 100.f); 
    shader->setmat4("projection", projection);
    
    auto len = sizeof(cube_positions)/sizeof(cube_positions[0]);
    for(auto i = 0; i < len; i++) {
        glm::mat4 model = glm::mat4(1.f);
        model = glm::translate(model, cube_positions[i]); 
        model = glm::rotate(model, 6 * sin(time * (i+1) /6) + i / 6.f, glm::vec3(0.5f, 1.f, 0.f));
        shader->setmat4("model", model);
        
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
}

void update(Shader* shader, float time) {
    // update_transform_fun(shader, time);
    update_color(shader, time);
    update_draw_transform(shader, time);
}

void render(GLFWwindow* window, Shader* shader, GLuint vao) {
    glClearColor(1.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    shader->use();
    shader->seti("tex", 0);
    shader->seti("tex2", 1);
    
    auto time = glfwGetTime(); 
    update(shader, time);
    
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
         0.5f,  0.5f, 0.0f,   1.f, 1.f, 1.f,    1.f, 1.f,
         0.0f,  0.0f, 0.5f,   0.f, 0.f, 0.f,    2.f, 2.f, 
    };
    unsigned int indices[] = {
        0, 3, 2,
        1, 3, 0,
        0, 1, 4,
        0, 2, 4,
        2, 4, 3,
        1, 3, 4
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

    glEnable(GL_DEPTH_TEST);

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

    GLFWwindow* window = glfwCreateWindow(state.width, state.height, "LearnOpenGL", NULL, NULL);
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

    auto frame = (input_frame) {  };

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

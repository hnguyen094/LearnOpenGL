#include <stdio.h>
#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

bool verbose = true;

const char* vertex_shader_source = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 pos;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);\n"
    "}\0";

const char* fragment_shader_source =
    "#version 330 core\n"
    "out vec4 frag_color;\n"
    "void main()\n"
    "{\n"
    "    frag_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n";

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

bool wireframe;
int last_state;
void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    } else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && last_state != GLFW_PRESS) {
        wireframe = !wireframe;
        int key = wireframe ? GL_LINE : GL_FILL;
        glPolygonMode(GL_FRONT_AND_BACK, key);
    }
    last_state = glfwGetKey(window, GLFW_KEY_SPACE);
}

void assrt(bool pass_condition, const char* fail_message) {
    if (pass_condition) return;
    printf("[LearnOpenGL] {%s}\n", fail_message);
}

GLuint compile_shader(GLenum shader_type, const char* source) {
    auto shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, log);
        std::cout << "Shader compilation failed\n" << log << std::endl;
    }
    return success ? shader : 0;
}

GLuint create_link_shader_program(GLuint vert_shader, GLuint frag_shader) {
    unsigned int shader_program;
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);

    int success;
    char log[512];
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, log);
    }
    return success ? shader_program : 0;
}

void render(GLFWwindow* window, GLuint shader_program, GLuint vao) {
    glClearColor(1.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);
    glBindVertexArray(vao);
    // glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(window);
}

void render_init(GLFWwindow* window, GLuint &shader_program, GLuint &vao) {
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
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
    
    auto vert_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    assrt(vert_shader, "Failed to compile vertex shader.");
    auto frag_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    assrt(frag_shader, "Failed to compile fragment shader.");
    shader_program = create_link_shader_program(vert_shader, frag_shader);
    assrt(shader_program, "Failed to link shaders to shader program.");
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    
    // intepret the vertex data (per vertex attribute)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

}

int main() {
	  printf("Hello, World!\n");

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

    unsigned int shader_program, vao;
    render_init(window, shader_program, vao);

    while (!glfwWindowShouldClose(window)) { // render loop
        process_input(window);
        render(window, shader_program, vao);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "assrt.h"

class Shader {
    private:
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

    public:
        unsigned int ID; // program id
       
        Shader() {} 

        void configure(const char* vert_path, const char* frag_path) {
            std::string vert_code;
            std::string frag_code;
            std::ifstream v_shader_file;
            std::ifstream f_shader_file;

            v_shader_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            f_shader_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            try {
                v_shader_file.open(vert_path);
                f_shader_file.open(frag_path);

                std::stringstream v_shader_stream, f_shader_stream;

                v_shader_stream << v_shader_file.rdbuf();
                f_shader_stream << f_shader_file.rdbuf();

                v_shader_file.close();
                f_shader_file.close();

                vert_code = v_shader_stream.str();
                frag_code = f_shader_stream.str();
            } catch (std::ifstream::failure e) {
                printf("[Shader] Error: failed to read shader files.\n");
            }
            const char* v_shader_code = vert_code.c_str();
            const char* f_shader_code = frag_code.c_str();

            auto vert_shader = compile_shader(GL_VERTEX_SHADER, v_shader_code);
            assrt(vert_shader, "Failed to compile vertex shader.");
            auto frag_shader = compile_shader(GL_FRAGMENT_SHADER, f_shader_code);
            assrt(frag_shader, "Failed to compile fragment shader.");
            glDeleteShader(vert_shader);
            glDeleteShader(frag_shader);

            ID = create_link_shader_program(vert_shader, frag_shader);
            assrt(ID, "Failed to link shaders to shader program.");
        }
        
        void use() {
            glUseProgram(ID);
        }

        void setb(const std::string &name, bool value) const {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), (int) value);
        }
        void seti(const std::string &name, int value) const {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        }
        void setf(const std::string &name, float value) const {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        }
};
#endif

add_executable(LearnOpenGL
  main.cpp
  glad.cpp)

target_compile_features(LearnOpenGL PRIVATE cxx_std_17)
target_link_libraries(LearnOpenGL PRIVATE glfw glm)
target_include_directories(LearnOpenGL PUBLIC include/)

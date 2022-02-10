add_executable(LearnOpenGL
  shader.h
  assrt.h
  main.cpp
  glad.cpp)

target_compile_features(LearnOpenGL PRIVATE cxx_std_17)
target_link_libraries(LearnOpenGL PRIVATE glfw glm)
target_include_directories(LearnOpenGL PUBLIC include/)

# Copy shaders to build output
add_custom_target(copy_shaders)
add_custom_command(TARGET copy_shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/" "${CMAKE_CURRENT_BINARY_DIR}/shaders/"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
  )

add_dependencies(LearnOpenGL copy_shaders)

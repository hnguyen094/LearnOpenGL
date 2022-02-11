add_executable(LearnOpenGL
  shader.h
  assrt.h
  stb_image.h
  main.cpp
  glad.cpp)

target_compile_features(LearnOpenGL PRIVATE cxx_std_17)
target_link_libraries(LearnOpenGL PRIVATE glfw glm)
target_include_directories(LearnOpenGL PUBLIC include/)

# Copy data to build output
add_custom_target(copy_data)
add_custom_command(TARGET copy_data
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/data/" "${CMAKE_CURRENT_BINARY_DIR}/data/"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/data/"
  )

add_dependencies(LearnOpenGL copy_data)

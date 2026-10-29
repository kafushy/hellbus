cmake_minimum_required(VERSION 3.16)
project(Hellbus CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)

include_directories(include src)

set_source_files_properties(include/glad/glad.c PROPERTIES LANGUAGE CXX)

file(GLOB_RECURSE GAME_SOURCES "src/*.cpp")

add_executable(hellbus ${GAME_SOURCES} include/glad/glad.c)
target_link_libraries(hellbus PRIVATE glfw OpenGL::GL assimp::assimp)
cmake_minimum_required(VERSION 3.10)

project(Test VERSION 1.0 LANGUAGES C)

find_package(Vulkan REQUIRED)
if (NOT WIN32)
    find_package(glfw3)
endif ()

add_subdirectory(dependencies)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

add_subdirectory(source)

include_directories(${SOURCE_DIRECTORIES})
add_executable(Test ${SOURCE_FILES} "${CMAKE_SOURCE_DIR}/source/main.c")

set(FLAGS
    -g
    -W
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wunused
    -Werror
    -Wno-comment
    -pedantic
    -pedantic-errors
    -D_GLIBCXX_DEBUG
    -D_GLIBCXX_DEBUG_PEDANTIC
    -D_UCRT
)

target_compile_options(Test PUBLIC ${FLAGS})

get_target_property(MAIN_CFLAGS Test COMPILE_OPTIONS)

add_custom_command(TARGET Test POST_BUILD
    COMMAND echo built with the flags: ${MAIN_CFLAGS}
)

target_include_directories(Test PRIVATE ${INCLUDE_DIRECTORIES})
target_link_libraries(Test ${LINK_LIBRARIES} -lm)
target_compile_features(Test PRIVATE c_std_23)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_options(Test PRIVATE -mwindows)
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

add_custom_target(shaders DEPENDS
    ${CMAKE_SOURCE_DIR}/shaders/shader2.frag
    ${CMAKE_SOURCE_DIR}/shaders/shader.vert
    ${CMAKE_SOURCE_DIR}/textures
    ${CMAKE_SOURCE_DIR}/models
)

add_custom_command(TARGET shaders POST_BUILD
    COMMAND glslc ${CMAKE_SOURCE_DIR}/shaders/shader.vert -o ${CMAKE_BINARY_DIR}/shaders/vert.spv
    COMMAND glslc ${CMAKE_SOURCE_DIR}/shaders/shader2.frag -o ${CMAKE_BINARY_DIR}/shaders/frag2.spv
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/textures ${CMAKE_BINARY_DIR}/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/models ${CMAKE_BINARY_DIR}/models
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/examples ${CMAKE_BINARY_DIR}/examples
)

add_dependencies(Test shaders)

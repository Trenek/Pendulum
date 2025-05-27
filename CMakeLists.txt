cmake_minimum_required(VERSION 3.10)

project(Pendulum VERSION 1.0 LANGUAGES C)

add_subdirectory(dependencies)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(source)
add_subdirectory(icon)

add_executable(Pendulum
    ${SOURCE_FILES} 
    ${ICON}
    "${CMAKE_SOURCE_DIR}/source/main.c"
)

target_link_libraries(Pendulum ${LINK_LIBRARIES})
target_include_directories(Pendulum PRIVATE 
    ${INCLUDE_DIRECTORIES}
    ${SOURCE_DIRECTORIES}
)
set(COMPILE_OPTIONS
    -W
    -g
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wunused
    -Wno-logical-op-parentheses
    -Wno-parentheses
    -Werror
    -Wno-comment
    -pedantic
    -pedantic-errors
    -D_GLIBCXX_DEBUG
    -D_GLIBCXX_DEBUG_PEDANTIC
    -D_UCRT
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(COMPILE_OPTIONS 
        ${COMPILE_OPTIONS}
        -O3
    )
    if (WIN32)
        target_link_options(Pendulum PRIVATE -mwindows)
    endif()
else()
    set(COMPILE_OPTIONS 
        ${COMPILE_OPTIONS}
        -g
    )
endif()

target_compile_options(Pendulum PUBLIC ${COMPILE_OPTIONS})

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

add_custom_target(shaders DEPENDS
    ${CMAKE_SOURCE_DIR}/shaders
    ${CMAKE_SOURCE_DIR}/textures
    ${CMAKE_SOURCE_DIR}/models
)

set(SHADER_SRC "${CMAKE_SOURCE_DIR}/shaders")
set(SHADER_DST "${CMAKE_BINARY_DIR}/shaders")

add_custom_command(TARGET shaders POST_BUILD
    COMMAND glslc ${CMAKE_SOURCE_DIR}/shaders/shader.vert -o ${CMAKE_BINARY_DIR}/shaders/vert.spv
    COMMAND glslc ${CMAKE_SOURCE_DIR}/shaders/shader2.frag -o ${CMAKE_BINARY_DIR}/shaders/frag2.spv
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/textures ${CMAKE_BINARY_DIR}/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/models ${CMAKE_BINARY_DIR}/models
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/fonts ${CMAKE_BINARY_DIR}/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/music ${CMAKE_BINARY_DIR}/music
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/examples ${CMAKE_BINARY_DIR}/examples
)

add_dependencies(Pendulum shaders)

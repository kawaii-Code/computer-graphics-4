cmake_minimum_required(VERSION 3.29.6)
project(computer-graphics-4 C)

set(CMAKE_C_STANDARD 17)

file(GLOB glad_files third_party/glad/*.c)

add_executable(${PROJECT_NAME}
    main.c
    linalg.c
    glfw_boilerplate.c
    obj_loader.c
    ${glad_files}
)

# Общие инклуды (glad, stb)
target_include_directories(${PROJECT_NAME} PUBLIC third_party)

if (WIN32)
    # Заголовки GLFW лежат в third_party/GLFW
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}/third_party
    )

    # Линкуем glfw.lib, который лежит прямо в third_party
    target_link_libraries(${PROJECT_NAME}
        ${CMAKE_SOURCE_DIR}/third_party/glfw.lib
        opengl32
        winmm
    )

elseif(APPLE)
    target_link_libraries(${PROJECT_NAME}
        "-framework IOKit"
        "-framework Cocoa"
        "-framework OpenGL"
    )
else()
    target_link_libraries(${PROJECT_NAME} m)
endif()

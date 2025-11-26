cmake_minimum_required(VERSION 3.29.6)
project(computer-graphics-4 C)

set(CMAKE_C_STANDARD 17)

file(GLOB source_files *.c)
file(GLOB glad_files third_party/glad/*.c)

add_executable(${PROJECT_NAME} ${source_files} ${glad_files})

target_include_directories(${PROJECT_NAME} PUBLIC third_party)

target_link_directories(${PROJECT_NAME} PUBLIC third_party)
target_link_libraries(${PROJECT_NAME} glfw)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} winmm)
else()
    target_link_libraries(${PROJECT_NAME} m)
endif()

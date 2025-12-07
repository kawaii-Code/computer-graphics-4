cmake_minimum_required(VERSION 3.29.6)
project(computer-graphics-4 C)

set(CMAKE_C_STANDARD 17)

file(GLOB glad_files third_party/glad/*.c)

# Основной проект (main.c)
add_executable(${PROJECT_NAME} main.c linalg.c glfw_boilerplate.c obj_loader.c ${glad_files})

target_include_directories(${PROJECT_NAME} PUBLIC third_party)

target_link_directories(${PROJECT_NAME} PUBLIC third_party)
find_package(glfw3 REQUIRED)
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

# Солнечная система
add_executable(solar_system solar_system.c linalg.c glfw_boilerplate.c obj_loader.c ${glad_files})

target_include_directories(solar_system PUBLIC third_party)

target_link_directories(solar_system PUBLIC third_party)
target_link_libraries(solar_system glfw)

if (APPLE)
    target_link_libraries(solar_system "-framework IOKit")
    target_link_libraries(solar_system "-framework Cocoa")
    target_link_libraries(solar_system "-framework OpenGL")
elseif(WIN32)
    target_link_libraries(solar_system winmm)
else()
    target_link_libraries(solar_system m)
endif()


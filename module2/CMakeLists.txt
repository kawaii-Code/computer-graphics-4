cmake_minimum_required(VERSION 3.29.6)
project(computer-graphics-4 C)

set(CMAKE_C_STANDARD 17)

add_executable(${PROJECT_NAME}
        main.c
        common.c
        clay_renderer_raylib.c
        vector.h
        task1.c
        task2.c
        task3.c
        llist.h
        stack.h
        Polyhedron.c
        Polyhedron.h
        scene.h
        scene.c
        camera.h
        camera.c
)

file(GLOB RAYLIB_FILES "../third_party/lib/*raylib.*")
list(GET RAYLIB_FILES 0 RAYLIB_FILE)

add_library(raylib STATIC IMPORTED)
set_target_properties(raylib PROPERTIES
        IMPORTED_LOCATION ${RAYLIB_FILE}
)

target_link_libraries(${PROJECT_NAME} raylib)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} winmm)
else()
    target_link_libraries(${PROJECT_NAME} m)
endif()

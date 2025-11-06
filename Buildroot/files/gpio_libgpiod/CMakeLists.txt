cmake_minimum_required(VERSION 3.5)

project(gpio_libgpiod LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGPIOD REQUIRED libgpiod)

add_executable(${PROJECT_NAME} gpio_libgpiod.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBGPIOD_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBGPIOD_LIBRARIES})

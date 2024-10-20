cmake_minimum_required(VERSION 3.5)

project(gpio_libgpiod LANGUAGES C)

add_executable(${PROJECT_NAME} gpio_libgpiod.c)
target_link_libraries(${PROJECT_NAME} gpiod)

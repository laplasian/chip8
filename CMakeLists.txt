cmake_minimum_required(VERSION 3.28)
project(chip8 C)

set(CMAKE_C_STANDARD 11)

# Установка каталога исходников
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# Установка каталога ресурсов
set(RESOURCES_DIR ${CMAKE_SOURCE_DIR}/resources)

# Добавление исполняемого файла
add_executable(chip8 main.c
        src/CPU.c
        src/CPU.h
        src/emulator.c
        src/emulator.h
        src/Sound.c
        src/Sound.h
        src/super_chip.c
        src/super_chip.h
)

# Это нужно для иконки приложения
target_sources(chip8 PRIVATE resources.rc)

# Убираем консольное окно
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")

# подключение библиотек SDL2 и SDL2_ttf
set(SDL2_DIR ${CMAKE_SOURCE_DIR}/src/SDL2)
include_directories(${SDL2_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src/SDL2_ttf/include)
target_link_libraries(chip8 ${SDL2_DIR}/lib/x64/SDL2.lib ${SDL2_DIR}/lib/x64/SDL2main.lib)
target_link_libraries(chip8 ${CMAKE_SOURCE_DIR}/src/SDL2_ttf/lib/x64/SDL2_ttf.lib)

# Установка выходной директории для программы
set_target_properties(chip8 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Копирование ресурсов в выходную директорию
add_custom_command(
    TARGET chip8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${RESOURCES_DIR} ${CMAKE_BINARY_DIR}/resources
)
# Копирование динамических библиотек в выходную директорию
add_custom_command(
    TARGET chip8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${SDL2_DIR}/lib/x64/SDL2.dll ${CMAKE_BINARY_DIR}
)
add_custom_command(
    TARGET chip8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/src/SDL2_ttf/lib/x64/SDL2_ttf.dll ${CMAKE_BINARY_DIR}
)

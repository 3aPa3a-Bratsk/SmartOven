cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(oven_controller)

# Добавляем компоненты проекта
set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/components)

# Настройка компиляции
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=format" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-format" APPEND)

# Добавляем библиотеки
# Библиотеки MAX6675, LiquidCrystal и LCDMenu должны быть установлены в components/
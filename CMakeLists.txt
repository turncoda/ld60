cmake_minimum_required(VERSION 3.30)
project(DigActionGame)

set(TARGET_NAME mygame)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

add_executable(${TARGET_NAME}
  src/main.c
  src/game.c
)

set_property(TARGET ${TARGET_NAME} PROPERTY C_STANDARD 23)
target_compile_options(${TARGET_NAME} PRIVATE -Wall -Werror -Wextra -Wpedantic)
target_link_libraries(${TARGET_NAME} PRIVATE SDL3::SDL3)
target_include_directories(${TARGET_NAME} PRIVATE
  ${CMAKE_BINARY_DIR}/gen
  ${CMAKE_SOURCE_DIR}/src
)

# Load all PNG assets into separate header files
file(GLOB spr_paths ${CMAKE_SOURCE_DIR}/assets/*.png)
foreach(path ${spr_paths})
  get_filename_component(name ${path} NAME_WE)
  set(var_name spr_${name})
  set(output gen/${var_name}.h)
  add_custom_command(
    OUTPUT ${output}
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/make_generic_header.bat ${path} ${var_name} ${output}
    DEPENDS scripts/make_generic_header.bat ${path}
    VERBATIM
  )
  add_custom_target(generate_${var_name} DEPENDS ${output})
  add_dependencies(${TARGET_NAME} generate_${var_name})
endforeach()

# Parse JSON spritesheet data and generate header
file(GLOB meta_paths ${CMAKE_SOURCE_DIR}/assets/*.json)
foreach(path ${meta_paths})
  get_filename_component(name ${path} NAME_WE)
  set(var_name meta_${name})
  set(output gen/${var_name}.h)
  add_custom_command(
    OUTPUT ${output}
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/make_spritesheet_header.bat ${path} ${output}
    DEPENDS scripts/make_spritesheet_header.bat scripts/spritesheet_json_filter_h.jq ${path}
    VERBATIM
  )
  add_custom_target(generate_${var_name} DEPENDS ${output})
  add_dependencies(${TARGET_NAME} generate_${var_name})
endforeach()

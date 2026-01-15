add_executable(main main/src/main.cpp)
target_link_libraries(main PRIVATE sorLib)

add_executable(doomProcess doomProcess/src/main.cpp)
target_link_libraries(doomProcess PRIVATE sorLib)


add_library(global_config INTERFACE)
target_compile_definitions(global_config INTERFACE
        DOOM_PATH="$<TARGET_FILE:doomProcess>"
)
target_link_libraries(main PRIVATE global_config)

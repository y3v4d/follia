function(add_example NAME)
    add_executable("${NAME}" "${NAME}.c")
    target_link_libraries("${NAME}" PRIVATE follia)
endfunction()

add_example(snake)
add_example(primitives)
add_example(space-invaders)
add_example(3d)
add_example(gui)

target_link_libraries(3d PRIVATE -lm)

function(add_test NAME)
    add_executable("${NAME}" "${NAME}.c")
    target_link_libraries("${NAME}" PRIVATE follia)
endfunction()

add_test(mouse)
add_test(text)
add_test(timer)

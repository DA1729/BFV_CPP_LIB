foreach(example_name basic_demo noise_growth)
    add_executable(${example_name} ${example_name}.cpp)
    target_link_libraries(${example_name} PRIVATE bfv)
endforeach()

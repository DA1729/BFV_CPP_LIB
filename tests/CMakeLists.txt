set(bfv_tests
    test_mod_arith
    test_poly
    test_params
    test_encoder
    test_scheme
)

foreach(test_name ${bfv_tests})
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE bfv)
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

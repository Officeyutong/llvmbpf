# Have to fetch Catch2
if(${BPFTIME_ENABLE_UNIT_TESTING} AND NOT TARGET Catch2)
    message(STATUS "Adding Catch2 by FetchContent at llvmbpf/test/unit-test")
    Include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)
endif()

set(TEST_SOURCES
    bpf_prog_test.cpp
    vm_test.cpp
)

add_executable(llvm_jit_tests ${TEST_SOURCES})
set_property(TARGET llvm_jit_tests PROPERTY CXX_STANDARD 20)
add_dependencies(llvm_jit_tests llvmbpf_vm)
target_link_libraries(llvm_jit_tests PRIVATE llvmbpf_vm Catch2::Catch2WithMain ${LIBBPF_LIBRARIES})
target_include_directories(llvm_jit_tests PRIVATE ${Catch2_INCLUDE} ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../src ${LIBBPF_INCLUDE_DIRS} ${LIBBPF_INCLUDE_DIRS}/uapi)
add_test(NAME llvm_jit_tests COMMAND llvm_jit_tests)

include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)

FetchContent_MakeAvailable(doctest)

add_executable(forge_tests
    test_entities.cpp
    test_components.cpp
    test_meta.cpp
    test_algorithms.cpp
    test_view.cpp
    main.cpp
)

target_link_libraries(forge_tests
    PRIVATE
        forge
        doctest::doctest
)
target_compile_options(forge_tests PRIVATE
    -fsanitize=address
    -fno-omit-frame-pointer
)

target_link_options(forge_tests PRIVATE
    -fsanitize=address
)
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

doctest_discover_tests(forge_tests)
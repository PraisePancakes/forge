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
    main.cpp
)

target_link_libraries(forge_tests
    PRIVATE
        forge
        doctest::doctest
)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

doctest_discover_tests(forge_tests)
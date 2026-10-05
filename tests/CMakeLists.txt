include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)

FetchContent_MakeAvailable(doctest)

add_executable(forge_tests
   entity_traits.cpp
   generator.cpp
   sparse_set.cpp
   pool_storage.cpp
   registry.cpp
   meta.cpp
   algorithms.cpp
   view_iterator.cpp
   sparse_set_iterator.cpp
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
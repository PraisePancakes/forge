include(FetchContent)

FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.16.0
)

FetchContent_MakeAvailable(entt)

add_executable(forge_benchmark
    main.cpp
)

target_link_libraries(forge_benchmark
    PRIVATE
        forge
        EnTT::EnTT
)

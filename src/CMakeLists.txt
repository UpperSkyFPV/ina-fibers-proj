add_executable(fibers
    main.cpp
)

target_include_directories(fibers PUBLIC .)

target_link_libraries(fibers
    fmt::fmt
    loguru::loguru
    tsl::robin_map
    lockfree
)

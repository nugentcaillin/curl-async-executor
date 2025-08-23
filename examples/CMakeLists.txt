project(examples)


add_executable(example)

target_sources(example
    PRIVATE
        example.cpp
)

target_link_libraries(example
    PRIVATE
        ProjectName
)


CONFIGURE_TARGET(example)

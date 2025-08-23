# About
# Installation

This library can be made available using FetchContent as follows in your CMakeLists.txt

```console
include(FetchContent)
FetchContent_Declare(
    [Library Name]
    GIT_REPOSITORY [Repository URL]
    GIT_TAG [Git tag]
)
FetchContent_MakeAvailable([Library Name])

target_link_libraries(${PROJECT_NAME} PRIVATE [Library Name])
```

# Usage
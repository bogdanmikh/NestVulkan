file(GLOB_RECURSE INCLUDE
        include/*.h,
        include/*.hpp
)

file(GLOB_RECURSE SOURCES
        src/*.c,
        src/*.cpp
)

add_library(Nest STATIC ${SOURCES} ${INCLUDE})

configureTarget(Nest)

target_include_directories(Nest PUBLIC include)
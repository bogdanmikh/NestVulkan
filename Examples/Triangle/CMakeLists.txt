file(GLOB_RECURSE INCLUDE
        *.h,
        *.hpp
)

file(GLOB_RECURSE SOURCES
        *.c,
        *.cpp
)

add_executable(
        Triangle
        ${SOURCES}
        ${INCLUDE}
)

target_link_libraries(Triangle Nest)

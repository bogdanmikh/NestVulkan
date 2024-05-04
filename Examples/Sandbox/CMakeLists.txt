file(GLOB_RECURSE INCLUDE
        *.h,
        *.hpp
)

file(GLOB_RECURSE SOURCES
        *.c,
        *.cpp
)

add_executable(
        Sandbox
        ${SOURCES}
        ${INCLUDE}
)

target_link_libraries(Sandbox Nest)
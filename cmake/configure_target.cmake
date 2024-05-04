function(configureTarget TARGET_NAME)
    # GLFW
    target_link_libraries(${TARGET_NAME} glfw)
    target_include_directories(${TARGET_NAME} PUBLIC Vendor/glfw/include)

    # GLM
    target_link_libraries(${TARGET_NAME} glm)
    target_include_directories(${TARGET_NAME} PUBLIC Vendor/glm)

    # STB_IMAGE
    target_link_libraries(${TARGET_NAME} stb_image)
    target_include_directories(${TARGET_NAME} PUBLIC Vendor/stb_image)

    # imgui
    target_link_libraries(${TARGET_NAME} imgui)
    target_include_directories(${TARGET_NAME} PUBLIC Vendor/imgui)

    # vulkan
    target_link_libraries(${TARGET_NAME} vulkan)
    target_include_directories(${TARGET_NAME} PUBLIC Vendor/vulkan/include)

    # spdlog
    target_link_libraries(${TARGET_NAME} spdlog)
    target_include_directories(${TARGET_NAME} PUBLIC Vendor/spdlog)

    # allocator
endfunction()
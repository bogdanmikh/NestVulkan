function(configureTarget TARGET_NAME)
    target_compile_definitions(${TARGET_NAME} PRIVATE VK_ENABLE_BETA_EXTENSIONS)
    # GLFW
    target_link_libraries(${TARGET_NAME} glfw)
    target_include_directories(${TARGET_NAME} PUBLIC ${VENDOR_DIRECTORY}/glfw/include)

    # GLM
    target_link_libraries(${TARGET_NAME} glm)
    target_include_directories(${TARGET_NAME} PUBLIC ${VENDOR_DIRECTORY}/glm)

    # STB_IMAGE
    target_link_libraries(${TARGET_NAME} stb_image)
    target_include_directories(${TARGET_NAME} PUBLIC ${VENDOR_DIRECTORY}/stb_image)

    # imgui
    target_link_libraries(${TARGET_NAME} imgui)
    target_include_directories(${TARGET_NAME} PUBLIC ${VENDOR_DIRECTORY}/imgui)

    # vulkan
    target_link_libraries(${TARGET_NAME} Vulkan::Vulkan)

    # spdlog
    target_link_libraries(${TARGET_NAME} spdlog)
    target_include_directories(${TARGET_NAME} PUBLIC ${VENDOR_DIRECTORY}/spdlog)

    # allocator
endfunction()
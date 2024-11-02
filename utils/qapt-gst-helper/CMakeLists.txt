set(qapt-gst-helper_SRCS
    main.cpp
    GstMatcher.cpp
    PluginFinder.cpp
    PluginHelper.cpp
    PluginInfo.cpp)

include_directories(${GSTREAMER_INCLUDE_DIR}
    ${GLIB2_INCLUDE_DIR})

add_executable(qapt-gst-helper ${qapt-gst-helper_SRCS})
target_compile_definitions(qapt-gst-helper PRIVATE -DGST_DISABLE_DEPRECATED)
target_compile_options(qapt-gst-helper PRIVATE -fexceptions)

target_link_libraries(qapt-gst-helper
    ${GSTREAMER_LIBRARIES}
    ${GLIB2_LIBRARIES}
    KF${KF_MAJOR_VERSION}::CoreAddons
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::KIOCore
    KF${KF_MAJOR_VERSION}::WidgetsAddons
    KF${KF_MAJOR_VERSION}::WindowSystem
    QApt::Main)

install(TARGETS qapt-gst-helper DESTINATION ${LIBEXEC_INSTALL_DIR})

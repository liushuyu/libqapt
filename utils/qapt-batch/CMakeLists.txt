set(qaptbatch_SRCS
    main.cpp
    qaptbatch.cpp
    detailswidget.cpp
)

add_executable(qapt-batch ${qaptbatch_SRCS})

target_link_libraries(qapt-batch
    KF${KF_MAJOR_VERSION}::CoreAddons
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::KIOCore
    KF${KF_MAJOR_VERSION}::WidgetsAddons
    KF${KF_MAJOR_VERSION}::WindowSystem
    QApt::Main)

install(TARGETS qapt-batch ${INSTALL_TARGETS_DEFAULT_ARGS})

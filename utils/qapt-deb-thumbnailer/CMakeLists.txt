kcoreaddons_add_plugin(debthumbnailer SOURCES debthumbnailer.cpp INSTALL_NAMESPACE "kf${KF_MAJOR_VERSION}/thumbcreator")

target_link_libraries(debthumbnailer
    KF${KF_MAJOR_VERSION}::KIOWidgets
    QApt::Main)

message(WARNING "make sure thumbnailer actually works")

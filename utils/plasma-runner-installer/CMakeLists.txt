kcoreaddons_add_plugin(krunner_installer SOURCES installerrunner.cpp INSTALL_NAMESPACE "kf${KF_MAJOR_VERSION}/krunner")
# KI18N Translation Domain for this library
target_compile_definitions(krunner_installer PRIVATE -DTRANSLATION_DOMAIN=\"plasma-runner-installer\")

target_link_libraries(krunner_installer
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Runner)

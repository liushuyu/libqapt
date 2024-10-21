include(ECMAddTests)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED Test)

# FIXME: the data doesn't need to be copied, the absolute path to the data should be passed through definitions to the target
configure_file(
    data/test1.list
    ${CMAKE_CURRENT_BINARY_DIR}/data/test1.list
    COPYONLY
)

configure_file(
    data/test2.list
    ${CMAKE_CURRENT_BINARY_DIR}/data/test2.list
    COPYONLY
)

ecm_add_test(dependencyinfotest.cpp
    LINK_LIBRARIES
        Qt${QT_MAJOR_VERSION}::Test
        QApt::Main)

ecm_add_test(sourceslisttest.cpp
    LINK_LIBRARIES
        Qt${QT_MAJOR_VERSION}::Test
        QApt::Main)

ecm_add_test(transactionerrorhandlingtest.cpp
    LINK_LIBRARIES
        Qt${QT_MAJOR_VERSION}::Test
        QApt::Main)

project(qapt)
cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION_MAJOR 3)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 5)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" )
set(PROJECT_SOVERSION ${PROJECT_VERSION_MAJOR})

set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.96.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.84")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH
    ${ECM_MODULE_PATH}
    ${ECM_KDE_MODULE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(ECMGenerateHeaders)
include(ECMPackageConfigHelpers)
include(ECMPoQmTools)
include(ECMSetupVersion)
include(FeatureSummary)
include(GenerateExportHeader)
include(KDEInstallDirs)
include(KDECMakeSettings NO_POLICY_SCOPE)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMGeneratePriFile) # This needs InstallDirs

if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.4.0")
    set(QT_MIN_VERSION "${QT_REQUIRED_VERSION}")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()

# Turn exceptions on
kde_enable_exceptions()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED Core DBus Widgets)

find_package(Xapian REQUIRED)
find_package(AptPkg REQUIRED)

find_package(PolkitQt${QT_MAJOR_VERSION}-1 0.112 REQUIRED)

# Find the required Libaries
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${POLKITQT-1_INCLUDE_DIR}
    ${XAPIAN_INCLUDE_DIR}
    ${APTPKG_INCLUDE_DIR})

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX QAPT
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/qaptversion.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/QAptConfigVersion.cmake"
                  SOVERSION ${PROJECT_SOVERSION})

set(QAPT_WORKER_VERSION ${PROJECT_SOVERSION})
set(QAPT_WORKER_RDN org.kubuntu.qaptworker)
set(QAPT_WORKER_RDN_VERSIONED ${QAPT_WORKER_RDN}${QAPT_WORKER_VERSION})

# For forwarding into C++ convert them into properly excaped strings.
set(QAPT_WORKER_VERSION_STRING \"${QAPT_WORKER_VERSION}\")
set(QAPT_WORKER_RDN_VERSIONED_STRING \"${QAPT_WORKER_RDN_VERSIONED}\")

add_definitions(-DQAPT_WORKER_VERSION_STRING=${QAPT_WORKER_VERSION_STRING})
add_definitions(-DQAPT_WORKER_RDN_VERSIONED_STRING=${QAPT_WORKER_RDN_VERSIONED_STRING})

# Also forward version for utils.
add_definitions(-DCMAKE_PROJECT_VERSION=\"${PROJECT_VERSION}\")

find_package(KF${KF_MAJOR_VERSION} ${KF5_MIN_VERSION} COMPONENTS Auth # Only used to install worker actions
    # Used for utils
    CoreAddons I18n KIO IconThemes TextWidgets Runner WidgetsAddons WindowSystem)

find_package(DebconfKDE 1.0)
find_package(GStreamer 1.0)
find_package(GLIB2 2.0)

if (KF5CoreAddons_FOUND
        AND KF5I18n_FOUND
        AND KF5KIO_FOUND
        AND KF5Runner_FOUND
        AND KF5TextWidgets_FOUND
        AND KF5WidgetsAddons_FOUND
        AND KF5WindowSystem_FOUND
        AND KF5IconThemes_FOUND
        AND DebconfKDE_FOUND)
    set(WITH_UTILS true)
endif()

if (WITH_UTILS AND GSTREAMER_FOUND AND GLIB2_FOUND)
    set(WITH_GSTREAMER true)
    add_definitions(${GSTREAMER_DEFINITIONS})
endif()

add_feature_info(qapt-utils WITH_UTILS "Runtime utilities using KDE frameworks")
add_feature_info(qapt-gst-helper WITH_GSTREAMER "GStreamer codec helper util")

message(WARNING "gettext and tr in the same source is insanely tricky, maybe we should give some ki18n to qapt...")
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ecm_install_po_files_as_qm(po)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/QApt")

ecm_configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/QAptConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/QAptConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/QAptConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/QAptConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)
install(EXPORT QAptTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE QAptTargets.cmake
        NAMESPACE QApt:: )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qaptversion.h
        DESTINATION include/qapt
        COMPONENT Devel)

add_subdirectory(autotests)
add_subdirectory(src)

if(WITH_UTILS)
    add_subdirectory(utils)

    #Do not remove or modify these.  The release script substitutes in for these
    #comments with appropriate doc and translation directories.
    #PO_SUBDIR
    #DOC_SUBDIR
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libqapt.pc.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libqapt.pc
    @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libqapt.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    COMPONENT Devel)

ecm_generate_pri_file(BASE_NAME QApt
    LIB_NAME QApt
    DEPS "core widgets dbus"
    FILENAME_VAR PRI_FILENAME
    INCLUDE_INSTALL_DIR include/QApt)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

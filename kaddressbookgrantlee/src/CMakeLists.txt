add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions(-DTRANSLATION_DOMAIN=\"libkaddressbookgrantlee\")

if(KF5Prison_FOUND)
  add_definitions(-DHAVE_PRISON)
endif()

set(kaddressbook_grantlee_LIB_SRCS
  widget/grantleecontactviewer.cpp
  formatter/grantleecontactformatter.cpp
  formatter/grantleecontactgroupformatter.cpp
  formatter/grantleecontactutils.cpp
)

set(kaddressbook_grantlee_printing_LIB_SRCS
   printing/contactgrantleeprintaddressobject.cpp
   printing/contactgrantleeprintcryptoobject.cpp
   printing/contactgrantleeprintgeoobject.cpp
   printing/contactgrantleeprintimobject.cpp
   printing/contactgrantleeprintobject.cpp
   printing/contactgrantleeprintphoneobject.cpp
   printing/grantleeprint.cpp
)




add_library(KF5KaddressbookGrantlee ${kaddressbook_grantlee_LIB_SRCS} ${kaddressbook_grantlee_printing_LIB_SRCS})
generate_export_header(KF5KaddressbookGrantlee BASE_NAME kaddressbook_grantlee)

add_library(KF5::KaddressbookGrantlee ALIAS KF5KaddressbookGrantlee)

target_include_directories(KF5KaddressbookGrantlee INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KaddessbookGrantlee/;${KDE_INSTALL_INCLUDEDIR_KF5}/kaddressbookgrantlee>")
target_include_directories(KF5KaddressbookGrantlee PUBLIC "$<BUILD_INTERFACE:${kaddressbookgrantlee_SOURCE_DIR}/src;${kaddressbookgrantlee_BINARY_DIR}/src;${Grantlee_INCLUDE_DIRS}>")


target_link_libraries(KF5KaddressbookGrantlee
PUBLIC
  Grantlee5::Templates
PRIVATE
  KF5::ConfigWidgets
  KF5::Service
  KF5::GrantleeTheme
  KF5::AkonadiCore
  KF5::Contacts
  KF5::AkonadiContact
  KF5::IconThemes
  kleo
  KF5::I18n
)

set_target_properties(KF5KaddressbookGrantlee PROPERTIES
    VERSION ${KADDRESSBOOKGRANTLEE_VERSION_STRING}
    SOVERSION ${KADDRESSBOOKGRANTLEE_SOVERSION}
    EXPORT_NAME KaddressbookGrantlee
)

install(TARGETS
    KF5KaddressbookGrantlee
    EXPORT KF5KaddressbookGrantleeTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(KaddressbookGrantlee_CamelCase_HEADERS
  HEADER_NAMES
  formatter/GrantleeContactFormatter
  REQUIRED_HEADERS KaddressbookGrantlee_HEADERS
  PREFIX KaddressbookGrantlee
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${kaddressbookgrantlee_SOURCE_DIR})
install(FILES
    ${KaddressbookGrantlee_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KaddressbookGrantlee
    COMPONENT Devel
)

install(FILES
    ${KaddressbookGrantlee_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kaddressbookgrantlee_export.h
    ${KaddressbookGrantlee_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/kaddressbookgrantlee
    COMPONENT Devel
)
endif()

if (BUILD_TESTING)
   add_subdirectory(printing/autotests)
endif()

project(kaddressbook)


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/xxport/shared
  ${CMAKE_CURRENT_SOURCE_DIR}/printing/
)


add_definitions(-DTRANSLATION_DOMAIN=\"kaddressbook\")
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


if(KF5Prison_FOUND)
  add_definitions(-DHAVE_PRISON)
endif()

add_subdirectory(icons)
add_subdirectory(printing)
add_subdirectory(kontactplugin)
add_subdirectory(kconf_update)

if(KF5Gpgmepp_FOUND)
  add_definitions(-DQGPGME_FOUND)
endif()

set(kaddressbook_sendmail_SRCS
  sendmail/mailsenderjob.cpp
)

set(kaddressbook_sendvcards_SRCS
  sendvcards/sendvcardsjob.cpp
)

set(kaddressbook_printing_SRCS
  printing/detailled/detailledstyle.cpp
  printing/mike/mikesstyle.cpp
  printing/printingwizard.cpp
  printing/printprogress.cpp
  printing/printstyle.cpp
  printing/ringbinder/ringbinderstyle.cpp
  printing/compact/compactstyle.cpp
  printing/grantlee/grantleeprintstyle.cpp
  printing/stylepage.cpp
)

ki18n_wrap_ui(kaddressbook_printing_SRCS
  printing/ringbinder/rbs_appearance.ui
  printing/detailled/ds_appearance.ui
  printing/compact/compactstyle.ui
)

set(kaddressbook_xxport_SRCS
  xxport/csv/csv_xxport.cpp
  xxport/csv/csvimportdialog.cpp
  xxport/csv/dateparser.cpp
  xxport/csv/qcsvmodel.cpp
  xxport/csv/qcsvreader.cpp
  xxport/csv/templateselectiondialog.cpp
  xxport/ldif/ldif_xxport.cpp
  xxport/ldap/ldap_xxport.cpp
  xxport/gmx/gmx_xxport.cpp
  xxport/shared/xxport.cpp
  xxport/vcard/vcard_xxport.cpp
  xxport/xxportfactory.cpp
  xxport/xxportmanager.cpp
  xxport/vcard/vcardviewerdialog.cpp
  xxport/vcard/vcardexportselectionwidget.cpp
  xxport/contactselectiondialog.cpp
)

set(kaddressbook_merge_SRCS
   merge/job/searchpotentialduplicatecontactjob.cpp
   merge/job/mergecontactsjob.cpp
   merge/job/mergecontacts.cpp

   merge/manualmerge/mergecontactsdialog.cpp
   merge/manualmerge/mergecontactwidget.cpp
   merge/manualmerge/mergecontactwidgetlist.cpp

   merge/searchduplicate/searchandmergecontactduplicatecontactdialog.cpp
   merge/searchduplicate/searchduplicateresultwidget.cpp
   merge/searchduplicate/resultduplicatetreewidget.cpp
   merge/searchduplicate/mergecontactselectinformationtabwidget.cpp

   merge/widgets/mergecontactinfowidget.cpp
   merge/widgets/mergecontactshowresulttabwidget.cpp
   merge/widgets/mergecontactselectinformationwidget.cpp
   merge/widgets/mergecontactselectlistwidget.cpp
   merge/widgets/mergecontactloseinformationwarning.cpp
   merge/widgets/mergecontactselectinformationscrollarea.cpp
   merge/widgets/mergecontacterrorlabel.cpp
)

set(kaddressbook_gravatar_SRCS
   gravatar/job/gravatarupdatejob.cpp
   gravatar/widgets/gravatarupdatedialog.cpp
   gravatar/widgets/gravatarupdatewidget.cpp
)

set(kaddressbook_LIB_SRCS
  contactlist.cpp
  aboutdata.cpp
  categoryfilterproxymodel.cpp
  categoryselectwidget.cpp
  contactfields.cpp
  contactselectionwidget.cpp
  contactsorter.cpp
  contactswitcher.cpp
  globalcontactmodel.cpp
  mainwidget.cpp
  modelcolumnmanager.cpp
  quicksearchwidget.cpp
  utils.cpp
  kaddressbookmigrateapplication.cpp
  ${kaddressbook_merge_SRCS}
  ${kaddressbook_printing_SRCS}
  ${kaddressbook_xxport_SRCS}
  ${kaddressbook_sendmail_SRCS}
  ${kaddressbook_sendvcards_SRCS}
  ${kaddressbook_gravatar_SRCS}
)

ecm_qt_declare_logging_category(kaddressbook_LIB_SRCS HEADER kaddressbook_debug.h IDENTIFIER KADDRESSBOOK_LOG CATEGORY_NAME log_kaddressbook)

qt5_add_dbus_adaptor( kaddressbook_LIB_SRCS org.kde.kaddressbook.xml mainwidget.h MainWidget)

kconfig_add_kcfg_files(kaddressbook_LIB_SRCS settings/settings.kcfgc)
add_library(kaddressbookprivate ${kaddressbook_LIB_SRCS})
generate_export_header(kaddressbookprivate BASE_NAME kaddressbook)
target_link_libraries(kaddressbookprivate
PUBLIC
  KF5::PimCommon
PRIVATE
  KF5::Libkdepim
  KF5::KaddressbookGrantlee
  KF5::AkonadiCore
  KF5::Contacts
  KF5::AkonadiContact
  KF5::KCMUtils
  KF5::Codecs
  KF5::GrantleeTheme
  KF5::KDELibs4Support
  KF5::IconThemes
  Qt5::PrintSupport
  KF5::Gravatar
  KF5::AkonadiSearchDebug
)
if (WIN32)
   target_link_libraries(kaddressbookprivate KF5::Gpgmepp)
else()
   target_link_libraries(kaddressbookprivate PRIVATE KF5::Gpgmepp-pthread)
endif()
if (BUILD_TESTING)
   add_subdirectory(merge/autotests)
   add_subdirectory(sendmail/autotests)
   add_subdirectory(sendvcards/autotests)
   add_subdirectory(gravatar/autotests)
   add_subdirectory(gravatar/tests)
   add_subdirectory(autotests)
endif()

if(KF5Gpgmepp_FOUND)
   target_link_libraries(kaddressbookprivate PRIVATE KF5::QGpgme)
endif()
if(KF5Prison_FOUND)
   target_link_libraries(kaddressbookprivate PRIVATE KF5::Prison)
endif()

target_include_directories(kaddressbookprivate PUBLIC "$<BUILD_INTERFACE:${kaddressbook_SOURCE_DIR};${kaddressbook_BINARY_DIR}>")

set_target_properties(kaddressbookprivate PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS kaddressbookprivate ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set(kaddressbook_SRCS
  kaddressbook_debug.cpp
  main.cpp
  mainwindow.cpp
)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-kaddressbook.png")
ecm_add_app_icon(kaddressbook_SRCS ICONS ${ICONS_SRCS})

add_executable(kaddressbook ${kaddressbook_SRCS})

target_link_libraries(kaddressbook
  KF5::KontactInterface
  kaddressbookprivate
  KF5::Parts
)

if(KF5Gpgmepp_FOUND)
  target_link_libraries(kaddressbook KF5::QGpgme)
endif()

install(TARGETS kaddressbook ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS data/org.kde.kaddressbook.desktop data/kaddressbook-importer.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES data/kaddressbook.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES kaddressbookui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kaddressbook)

set(kaddressbook_PART_SRCS kaddressbookpart.cpp kaddressbook_debug.cpp)
add_library(kaddressbookpart MODULE ${kaddressbook_PART_SRCS})
target_link_libraries(kaddressbookpart  kaddressbookprivate KF5::Parts KF5::IconThemes)
install(TARGETS kaddressbookpart DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES data/kaddressbookpart.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

install(FILES
  viewertemplates/contact.html
  viewertemplates/contact_embedded.html
  viewertemplates/contact_row.html
  viewertemplates/contactgroup.html
  viewertemplates/contactgroup_embedded.html
  viewertemplates/contactgroup_additionalfieldrow.html
  viewertemplates/contactgroup_memberrow.html
  viewertemplates/theme.desktop
  DESTINATION ${KDE_INSTALL_DATADIR}/kaddressbook/viewertemplates/default/
)

install(FILES data/kaddressbook_themes.knsrc DESTINATION ${KDE_INSTALL_CONFDIR} )

install(DIRECTORY printing/grantlee/themes/ DESTINATION ${KDE_INSTALL_DATADIR}/kaddressbook/printing/themes/ )

project(kalarm)

add_definitions(-DQT_STRICT_ITERATORS)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

set(CMAKE_AUTOMOC_RELAXED_MODE ON)




include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

add_subdirectory(appicons)
add_subdirectory(pixmaps)
add_subdirectory(autostart)
add_subdirectory(kconf_update)

set(libkalarm_common_SRCS)
ecm_qt_declare_logging_category(libkalarm_common_SRCS HEADER kalarm_debug.h IDENTIFIER KALARM_LOG CATEGORY_NAME log_kalarm)

########### next target ###############
set(libkalarm_SRCS
    lib/buttongroup.cpp
    lib/checkbox.cpp
    lib/colourbutton.cpp
    lib/combobox.cpp
    lib/desktop.cpp
    lib/filedialog.cpp
    lib/groupbox.cpp
    lib/itembox.cpp
    lib/kalocale.cpp
    lib/label.cpp
    lib/messagebox.cpp
    lib/packedlayout.cpp
    lib/pushbutton.cpp
    lib/radiobutton.cpp
    lib/timeedit.cpp
    lib/timespinbox.cpp
    lib/timeperiod.cpp
    lib/timezonecombo.cpp
    lib/shellprocess.cpp
    lib/slider.cpp
    lib/spinbox.cpp
    lib/spinbox2.cpp
    lib/stackedwidgets.cpp
    lib/lineedit.cpp
    lib/synchtimer.cpp
)

set(kalarm_bin_SRCS ${libkalarm_SRCS}
    ${libkalarm_common_SRCS}
    birthdaydlg.cpp
    birthdaymodel.cpp
    main.cpp
    editdlg.cpp
    editdlgtypes.cpp
    soundpicker.cpp
    sounddlg.cpp
    alarmcalendar.cpp
    undo.cpp
    kalarmapp.cpp
    mainwindowbase.cpp
    mainwindow.cpp
    messagewin.cpp
    preferences.cpp
    prefdlg.cpp
    traywindow.cpp
    dbushandler.cpp
    recurrenceedit.cpp
    deferdlg.cpp
    functions.cpp
    fontcolour.cpp
    fontcolourbutton.cpp
    alarmtime.cpp
    alarmtimewidget.cpp
    specialactions.cpp
    reminder.cpp
    startdaytimer.cpp
    eventlistview.cpp
    alarmlistdelegate.cpp
    alarmlistview.cpp
    templatelistview.cpp
    kamail.cpp
    timeselector.cpp
    latecancel.cpp
    repetitionbutton.cpp
    emailidcombo.cpp
    find.cpp
    pickfileradio.cpp
    newalarmaction.cpp
    commandoptions.cpp
    resourceselector.cpp
    templatepickdlg.cpp
    templatedlg.cpp
    templatemenuaction.cpp
    wakedlg.cpp
)
set(kalarm_bin_SRCS ${kalarm_bin_SRCS}
    kalarmmigrateapplication.cpp
    akonadimodel.cpp
    akonadiresourcecreator.cpp
    collectionmodel.cpp
    collectionsearch.cpp
    itemlistmodel.cpp
    calendarmigrator.cpp
    eventid.cpp
   )

ki18n_wrap_ui(kalarm_bin_SRCS
    wakedlg.ui
)

qt5_add_dbus_adaptor(kalarm_bin_SRCS org.kde.kalarm.kalarm.xml dbushandler.h DBusHandler)
qt5_add_dbus_interfaces(kalarm_bin_SRCS ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml)

kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/kalarmresource.kcfg org.kde.Akonadi.KAlarm.Settings)
qt5_add_dbus_interface(kalarm_bin_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.KAlarm.Settings.xml kalarmsettings KAlarmSettings)

kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/kalarmdirresource.kcfg org.kde.Akonadi.KAlarmDir.Settings)
qt5_add_dbus_interface(kalarm_bin_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.KAlarmDir.Settings.xml kalarmdirsettings KAlarmDirSettings)

qt5_add_dbus_interfaces(kalarm_bin_SRCS ${AKONADI_DBUS_INTERFACES_DIR}/org.freedesktop.Akonadi.Agent.Control.xml)
#qt5_add_dbus_adaptor(kalarm_bin_SRCS ${AKONADI_DBUS_INTERFACES_DIR}/org.freedesktop.Akonadi.Agent.Control.xml agentbase.h Akonadi::AgentBase controladaptor Akonadi__ControlAdaptor)

kconfig_add_kcfg_files(kalarm_bin_SRCS GENERATE_MOC kalarmconfig.kcfgc)

#if (UNIX)
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/appicons/*-apps-kalarm.png")
ecm_add_app_icon(kalarm_bin_SRCS ICONS ${ICONS_SRCS})
add_executable(kalarm_bin ${kalarm_bin_SRCS})
add_dependencies(kalarm_bin kmail_xml)

set_target_properties(kalarm_bin PROPERTIES OUTPUT_NAME kalarm)

target_link_libraries(kalarm_bin
KF5::AlarmCalendar
KF5::CalendarCore
KF5::CalendarUtils
KF5::Contacts
KF5::Mime
KF5::IdentityManagement
KF5::MailTransport
KF5::AkonadiCore
KF5::AkonadiMime
KF5::AkonadiContact
KF5::Holidays
KF5::Codecs
KF5::ConfigCore
KF5::Libkdepim
Phonon::phonon4qt5
KF5::Auth
KF5::DBusAddons
KF5::PimCommon
)

if (Qt5X11Extras_FOUND)
  target_link_libraries(kalarm_bin Qt5::X11Extras)
endif()


install(TARGETS kalarm_bin ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
#endif (UNIX)

########### install files ###############

install(PROGRAMS org.kde.kalarm.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install(PROGRAMS kalarm.autostart.desktop DESTINATION ${KDE_INSTALL_AUTOSTARTDIR})
install(FILES kalarm.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES kalarmconfig.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kalarmui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kalarm)
install(FILES org.kde.kalarm.kalarm.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})

########### KAuth helper ###############

add_executable(kalarm_helper rtcwakeaction.cpp ${libkalarm_common_SRCS})
target_link_libraries(kalarm_helper KF5::Auth KF5::KDELibs4Support)
install(TARGETS kalarm_helper DESTINATION ${KDE_INSTALL_LIBEXECDIR})
kauth_install_helper_files(kalarm_helper org.kde.kalarmrtcwake root)
kauth_install_actions(org.kde.kalarmrtcwake rtcwakeaction.actions)

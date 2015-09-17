add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
if(KDEPIM_ENTERPRISE_BUILD)
  set(USE_GRAPHICAL_SIEVE_EDITOR true)
else()
  set(USE_GRAPHICAL_SIEVE_EDITOR false)
endif()


configure_file(settings/sieve-editor.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/sieve-editor.kcfg)

if(BUILD_TESTING)
   add_subdirectory(tests)
   add_subdirectory(editor/tests)
   add_subdirectory(managescriptsjob/autotests)
   add_subdirectory(editor/autotests)
   add_subdirectory(autocreatescripts/tests)
   add_subdirectory(scriptsparsing/tests)
   add_subdirectory(vacation/tests)
   add_subdirectory(sievescriptdebugger/autotests)
   add_subdirectory(sievescriptdebugger/tests)
endif()

set(ksieveui_vacation_LIB_SRCS
  vacation/vacation.cpp
  vacation/vacationdialog.cpp
  vacation/vacationhelperjob.cpp
  vacation/vacationeditwidget.cpp
  vacation/multiimapvacationdialog.cpp
  vacation/vacationpagewidget.cpp
  vacation/vacationwarningwidget.cpp
  vacation/vacationscriptextractor.cpp
  vacation/vacationutils.cpp
  vacation/vacationcreatescriptjob.cpp
  vacation/vacationcheckjob.cpp
  vacation/multiimapvacationmanager.cpp
  vacation/vacationmanager.cpp
)

set(ksieveui_editor_LIB_SRCS
  editor/sieveeditorwidget.cpp
  editor/sievetextedit.cpp
  editor/sieveeditor.cpp
  editor/sievelinenumberarea.cpp
  editor/sieveinfowidget.cpp
  editor/sieveeditortextmodewidget.cpp
  editor/sieveeditorabstractwidget.cpp
  editor/warningwidget/sieveeditorwarning.cpp
  editor/warningwidget/sieveeditorparsingmissingfeaturewarning.cpp
  editor/sieveeditorhelphtmlwidget.cpp
  editor/sieveeditortabwidget.cpp
  editor/sieveeditorutil.cpp
  editor/sieveeditorloadprogressindicator.cpp
  editor/sieveeditorwebview.cpp
  editor/sieveeditormenubar.cpp
  editor/sievetexteditorspellcheckdecorator.cpp
  editor/sievesyntaxspellcheckinghighlighter.cpp
  editor/sievetexteditwidget.cpp
)

set(ksieveui_autocreatescripts_LIB_SRCS
  autocreatescripts/sievescriptparsingerrordialog.cpp
  autocreatescripts/sieveeditorgraphicalmodewidget.cpp 
  autocreatescripts/autocreatescriptdialog.cpp
  autocreatescripts/sieveconditionwidgetlister.cpp
  autocreatescripts/sievescriptlistbox.cpp
  autocreatescripts/sievescriptdescriptiondialog.cpp
  autocreatescripts/sieveactionwidgetlister.cpp
  autocreatescripts/sievescriptpage.cpp
  autocreatescripts/sievescriptblockwidget.cpp
  autocreatescripts/sievescripttabwidget.cpp
  autocreatescripts/autocreatescriptutil.cpp
  autocreatescripts/sieveincludewidget.cpp
  autocreatescripts/sieveforeverypartwidget.cpp
  autocreatescripts/sievewidgetpageabstract.cpp
  autocreatescripts/sieveglobalvariablewidget.cpp
  autocreatescripts/sieveactions/sieveactionlist.cpp
  autocreatescripts/sieveactions/sieveaction.cpp
  autocreatescripts/sieveactions/sieveactiondiscard.cpp
  autocreatescripts/sieveactions/sieveactionstop.cpp
  autocreatescripts/sieveactions/sieveactionsetflags.cpp
  autocreatescripts/sieveactions/sieveactionaddflags.cpp
  autocreatescripts/sieveactions/sieveactionfileinto.cpp
  autocreatescripts/sieveactions/sieveactionreject.cpp
  autocreatescripts/sieveactions/sieveactionkeep.cpp
  autocreatescripts/sieveactions/sieveactionredirect.cpp
  autocreatescripts/sieveactions/sieveactionabstractflags.cpp
  autocreatescripts/sieveactions/sieveactionremoveflags.cpp
  autocreatescripts/sieveactions/sieveactionnotify.cpp
  autocreatescripts/sieveactions/sieveactionabstracteditheader.cpp
  autocreatescripts/sieveactions/sieveactiondeleteheader.cpp
  autocreatescripts/sieveactions/sieveactionaddheader.cpp
  autocreatescripts/sieveactions/sieveactionvacation.cpp
  autocreatescripts/sieveactions/sieveactionenclose.cpp
  autocreatescripts/sieveactions/sieveactionreplace.cpp
  autocreatescripts/sieveactions/sieveactionextracttext.cpp
  autocreatescripts/sieveactions/sieveactionbreak.cpp
  autocreatescripts/sieveactions/sieveactionconvert.cpp
  autocreatescripts/sieveactions/sieveactionsetvariable.cpp
  autocreatescripts/sieveactions/sieveactionreturn.cpp
  autocreatescripts/sieveactions/sieveactionereject.cpp
  autocreatescripts/sieveactions/widgets/selectflagswidget.cpp
  autocreatescripts/sieveactions/widgets/selectfileintowidget.cpp
  autocreatescripts/sieveactions/widgets/addresslineedit.cpp
  autocreatescripts/sieveactions/widgets/selectaddheaderpositioncombobox.cpp
  autocreatescripts/sieveactions/widgets/selectimportancecombobox.cpp
  autocreatescripts/sieveactions/widgets/multilineedit.cpp
  autocreatescripts/sieveactions/widgets/selectvacationcombobox.cpp
  autocreatescripts/sieveactions/widgets/selectvariablemodifiercombobox.cpp
  autocreatescripts/commonwidgets/selectconvertparameterwidget.cpp
  autocreatescripts/commonwidgets/selectmatchtypecombobox.cpp
  autocreatescripts/commonwidgets/selectmimetypecombobox.cpp
  autocreatescripts/commonwidgets/sievehelpbutton.cpp
  autocreatescripts/sieveconditions/sievecondition.cpp
  autocreatescripts/sieveconditions/sieveconditionheader.cpp
  autocreatescripts/sieveconditions/sieveconditionlist.cpp
  autocreatescripts/sieveconditions/sieveconditionaddress.cpp
  autocreatescripts/sieveconditions/sieveconditionsize.cpp
  autocreatescripts/sieveconditions/sieveconditionenvelope.cpp
  autocreatescripts/sieveconditions/sieveconditionexists.cpp
  autocreatescripts/sieveconditions/sieveconditiontrue.cpp
  autocreatescripts/sieveconditions/sieveconditionbody.cpp
  autocreatescripts/sieveconditions/sieveconditiondate.cpp
  autocreatescripts/sieveconditions/sieveconditioncurrentdate.cpp
  autocreatescripts/sieveconditions/sieveconditionmailboxexists.cpp
  autocreatescripts/sieveconditions/sieveconditionspamtest.cpp
  autocreatescripts/sieveconditions/sieveconditionvirustest.cpp
  autocreatescripts/sieveconditions/sieveconditionihave.cpp
  autocreatescripts/sieveconditions/sieveconditionfalse.cpp
  autocreatescripts/sieveconditions/sieveconditionenvironment.cpp
  autocreatescripts/sieveconditions/sieveconditionhasflag.cpp
  autocreatescripts/sieveconditions/sieveconditionmetadata.cpp
  autocreatescripts/sieveconditions/sieveconditionconvert.cpp
  autocreatescripts/sieveconditions/sieveconditionmetadataexists.cpp
  autocreatescripts/sieveconditions/sieveconditionservermetadata.cpp
  autocreatescripts/sieveconditions/sieveconditionservermetadataexists.cpp
  autocreatescripts/sieveconditions/widgets/selectdatewidget.cpp
  autocreatescripts/sieveconditions/widgets/selectaddresspartcombobox.cpp
  autocreatescripts/sieveconditions/widgets/selectheadertypecombobox.cpp
  autocreatescripts/sieveconditions/widgets/selectbodytypewidget.cpp
  autocreatescripts/sieveconditions/widgets/selectrelationalmatchtype.cpp
  autocreatescripts/sieveconditions/widgets/selectcomparatorcombobox.cpp
  autocreatescripts/sieveconditions/widgets/selectmimecombobox.cpp
  autocreatescripts/sieveconditions/widgets/selectsizetypecombobox.cpp
  autocreatescripts/sieveconditions/widgets/selectsizewidget.cpp
)

set(ksieveui_scriptsparsing_LIB_SRCS
  scriptsparsing/xmlprintingscriptbuilder.cpp
  scriptsparsing/parsingresultdialog.cpp
  scriptsparsing/xmlprintingsyntaxhighlighter.cpp
  scriptsparsing/parsingutil.cpp
)

set(ksieveui_sievescriptdebugger_LIB_SRCS
  sievescriptdebugger/sievescriptdebuggerdialog.cpp
  sievescriptdebugger/sievescriptdebuggerwidget.cpp
  sievescriptdebugger/sievescriptdebuggerfrontendwidget.cpp
  sievescriptdebugger/sievescriptdebuggerwarning.cpp
  sievescriptdebugger/sievescriptdebuggertextedit.cpp
  sievescriptdebugger/sievescriptdebuggerresulteditor.cpp
)

set(ksieveui_LIB_SRCS
  managesievescriptsdialog.cpp
  widgets/custommanagesievewidget.cpp
  widgets/sievetreewidgetitem.cpp
  widgets/managesievetreeview.cpp
  widgets/managesievewidget.cpp
  debug/sievedebugdialog.cpp
  util/util.cpp
  managescriptsjob/generateglobalscriptjob.cpp
  managescriptsjob/parseuserscriptjob.cpp
  templates/sievetemplatewidget.cpp
  templates/sievedefaulttemplate.cpp 
  templates/sievetemplateeditdialog.cpp
  ${ksieveui_scriptsparsing_LIB_SRCS}
  ${ksieveui_vacation_LIB_SRCS}
  ${ksieveui_editor_LIB_SRCS}
  ${ksieveui_autocreatescripts_LIB_SRCS}
  ${ksieveui_sievescriptdebugger_LIB_SRCS}
)

ecm_qt_declare_logging_category(ksieveui_LIB_SRCS HEADER libksieve_debug.h IDENTIFIER LIBKSIEVE_LOG CATEGORY_NAME log_libksieve)

kconfig_add_kcfg_files(ksieveui_LIB_SRCS settings/sieve-vacation.kcfgc settings/sieve-editor.kcfgc)

add_library(ksieveui ${ksieveui_LIB_SRCS})
generate_export_header(ksieveui BASE_NAME ksieveui)

target_link_libraries(ksieveui
PUBLIC
  kmanagesieve
  ksieve
  KF5::PimCommon
  Qt5::Xml
PRIVATE
  KF5::Libkdepim
  KF5::Archive
  KF5::AkonadiCore
  KF5::Mime
  KF5::IconThemes
  KF5::IdentityManagement
  KF5::MailTransport
  KF5::KIOWidgets
  KF5::NewStuff
  KF5::WindowSystem
  Qt5::WebKitWidgets
  KF5::PimTextEdit
)

target_include_directories(ksieveui PUBLIC "$<BUILD_INTERFACE:${libksieve_SOURCE_DIR}/ksieveui;${libksieve_BINARY_DIR}/ksieveui>")

set_target_properties(
  ksieveui PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS ksieveui ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES data/ksieve_script.knsrc DESTINATION ${KDE_INSTALL_CONFDIR} )
install(DIRECTORY data/scripts/ DESTINATION ${KDE_INSTALL_DATADIR}/sieve/scripts/ )
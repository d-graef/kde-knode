add_executable( synchronizeresourcedialogtest synchronizeresourcedialogtest.cpp ../dialog/synchronizeresourcedialog.cpp)
add_test(synchronizeresourcedialogtest synchronizeresourcedialogtest)
ecm_mark_as_test(synchronizeresourcedialogtest)
target_link_libraries(synchronizeresourcedialogtest Qt5::Test Qt5::Widgets KF5::I18n KF5::ItemViews KF5::ConfigCore)


add_subdirectory(yousenditplugin)
add_subdirectory(dropboxplugin)
add_subdirectory(boxplugin)
add_subdirectory(hubicplugin)
add_subdirectory(webdavplugin)
if( KF5GAPI_FOUND )
    add_subdirectory(gdriveplugin)
endif()

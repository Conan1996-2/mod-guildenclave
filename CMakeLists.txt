AC_ADD_SCRIPT_LOADER("GuildEnclave" "${CMAKE_CURRENT_LIST_DIR}/src/loader.h")

AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/GuildEnclaveBroker.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/GuildEnclaveMgr.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/GuildEnclaveSpawner.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/GuildEnclaveConfig.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/GuildEnclaveCatalogMgr.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/GuildEnclaveInstanceMgr.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/GuildEnclaveCommands.cpp")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/guildenclave.conf.dist")


#
# local copy
#
if( WIN32 )
message( STATUS "Detecting local LUA" )
find_path( LUA_LOCAL_INCLUDE_DIRS "lua.h"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/include"
	NO_DEFAULT_PATH )
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
find_library( LUA_LOCAL_LIBRARIES
	NAMES lua54 lua5.4 lua
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/lib/Win32"
	NO_DEFAULT_PATH )
else()
find_library( LUA_LOCAL_LIBRARIES
	NAMES lua54 lua5.4 lua
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64"
	NO_DEFAULT_PATH )
endif()
mark_as_advanced( LUA_LOCAL_LIBRARIES )
mark_as_advanced( LUA_LOCAL_INCLUDE_DIRS )

if( LUA_LOCAL_LIBRARIES AND LUA_LOCAL_INCLUDE_DIRS )
	set( HAVE_LOCAL_LUA ON
		CACHE BOOL "lua is available as a local copy" )
	mark_as_advanced( HAVE_LOCAL_LUA )
else()
	foreach( _VAR LUA_LOCAL_LIBRARIES LUA_LOCAL_INCLUDE_DIRS )
		if( NOT "${_VAR}" )
			set( MISSING_VARS ${MISSING_VARS} ${_VAR} )
		endif()
	endforeach()
	message( STATUS "Could NOT find LUA (missing: ${MISSING_VARS})" )
	unset( HAVE_LOCAL_LUA CACHE )
endif()
message( STATUS "Detecting local LUA - done" )
endif( WIN32 )


#
# system
#
message( STATUS "Detecting system LUA" )
unset( LUA_LIBRARIES CACHE )
unset( LUA_INCLUDE_DIRS CACHE )
unset( LUA_INCLUDE_DIR CACHE )
find_package( Lua 5.4 QUIET )

set( LUA_SYSTEM_LIBRARIES ${LUA_LIBRARIES}
	CACHE PATH "system lua libraries" )
if( LUA_INCLUDE_DIRS )
	set( LUA_SYSTEM_INCLUDE_DIRS ${LUA_INCLUDE_DIRS}
		CACHE PATH "system lua include directories" )
else()
	set( LUA_SYSTEM_INCLUDE_DIRS ${LUA_INCLUDE_DIR}
		CACHE PATH "system lua include directories" )
endif()

mark_as_advanced( LUA_SYSTEM_LIBRARIES )
mark_as_advanced( LUA_SYSTEM_INCLUDE_DIRS )

if( LUA_SYSTEM_LIBRARIES AND LUA_SYSTEM_INCLUDE_DIRS )
	set( HAVE_SYSTEM_LUA ON
		CACHE BOOL "lua is available on the system" )
	mark_as_advanced( HAVE_SYSTEM_LUA )
else()
	unset( HAVE_SYSTEM_LUA CACHE )
endif()
message( STATUS "Detecting system LUA - done" )


#
# configure
#
CONFIGURE_WITH_LOCAL_OR_SYSTEM( LUA )

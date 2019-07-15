

# project architecture
if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	set(PLATFORM_TARGET "x64")
else()
	set(PLATFORM_TARGET "x86")
endif()

# Qt's modules
find_package(Qt5 COMPONENTS Core Concurrent REQUIRED)

# if CMAKE_INSTALL_PREFIX is set to default value I override it
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	 # prefix path for windows platform
	if (${CMAKE_SYSTEM_NAME} MATCHES "^Windows$")
		set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../deploy/libraries/${PROJECT_NAME} CACHE PATH "CMAKE_INSTALL_PREFIX path" FORCE)
	endif()
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Qt's moc files
qt5_wrap_cpp(MOC_files
	"include/IzModels/AbstractItemModel.h"
	"include/IzModels/ProxyModel.h"
	"include/IzModels/NullModel.h"
	"include/IzModels/StringListModel.h"
	"include/IzModels/VariantListModel.h"
)

# library definition
add_library(${PROJECT_NAME} SHARED
	"private/AbstractItemModel.cpp"
	"private/ProxyModel.cpp"
	"private/NullModel.cpp"
	"private/StringListModel.cpp"
	"private/VariantListModel.cpp"
	${MOC_files}
)

# public headers
set(PUBLIC_HEADERS
	"include/IzModels/AbstractItemModel.h"
	"include/IzModels/ProxyModel.h"
	"include/IzModels/NullModel.h"
	"include/IzModels/StringListModel.h"
	"include/IzModels/VariantListModel.h"
	"include/IzModels/IzModels_Global.h"
)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

# include directories for target IzLogger
target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/private
)

# target compile features
target_compile_features(${PROJECT_NAME}
	PRIVATE
		cxx_std_17
)

# target links
target_link_libraries(${PROJECT_NAME}
	PRIVATE
		Qt5::Core
		Qt5::Concurrent
)

# target compile definitions
target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		IZMODELS_LIBRARY
		QT_USE_QSTRINGBUILDER
)

# installs
include(GNUInstallDirs)
install (TARGETS ${PROJECT_NAME}
	EXPORT ${PROJECT_NAME}-export
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/${CMAKE_BUILD_TYPE}/${Qt5Core_VERSION}/${CMAKE_CXX_COMPILER_ID}/${PLATFORM_TARGET}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_BUILD_TYPE}/${Qt5Core_VERSION}/${CMAKE_CXX_COMPILER_ID}/${PLATFORM_TARGET}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_BUILD_TYPE}/${Qt5Core_VERSION}/${CMAKE_CXX_COMPILER_ID}/${PLATFORM_TARGET}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)
install(FILES ${PROJECT_NAME}Config.cmake
	DESTINATION ${CMAKE_INSTALL_PREFIX}
)

# export target
install(EXPORT ${PROJECT_NAME}-export
	FILE
		${PROJECT_NAME}Targets.cmake
	NAMESPACE
		IzModels::
	DESTINATION cmake/${PROJECT_NAME}
)

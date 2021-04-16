# cmake minumum version
cmake_minimum_required(VERSION 3.10)

# project name and version
project(IzModels VERSION 1.0.0 LANGUAGES CXX)

# project architecture
if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(PLATFORM_TARGET "x64")
else()
    set(PLATFORM_TARGET "x86")
endif()

# Qt's modules
find_package(Qt6 COMPONENTS Core Concurrent REQUIRED)

# enable autogeneration of moc files
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# library definition
add_library(
    IzModels
SHARED
    ""
)

# public headers
set(
PUBLIC_HEADERS
    "include/IzModels/AbstractItemModel.h"
    "include/IzModels/ProxyModel.h"
    "include/IzModels/NullModel.h"
    "include/IzModels/StringListModel.h"
    "include/IzModels/VariantListModel.h"
    "include/IzModels/IzModels_Global.h"
)

target_sources(
    IzModels
PRIVATE
    "private/AbstractItemModel.cpp"
    "private/ProxyModel.cpp"
    "private/NullModel.cpp"
    "private/StringListModel.cpp"
    "private/VariantListModel.cpp"
    ${PUBLIC_HEADERS}
)

set_target_properties(
    IzModels
PROPERTIES
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

# include directories for target
target_include_directories(
    IzModels
PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/private
)

# target compile features
target_compile_features(
    IzModels
PRIVATE
    cxx_std_17
)

# target links
target_link_libraries(
    IzModels
PRIVATE
    Qt6::Core
    Qt6::Concurrent
)

# target compile definitions
target_compile_definitions(
    IzModels
PRIVATE
    IZMODELS_LIBRARY
    QT_USE_QSTRINGBUILDER
)

# installs
include(GNUInstallDirs)
install(
    TARGETS IzModels
    EXPORT IzModels-export
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/${CMAKE_BUILD_TYPE}/${Qt5Core_VERSION}/${CMAKE_CXX_COMPILER_ID}/${PLATFORM_TARGET}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_BUILD_TYPE}/${Qt5Core_VERSION}/${CMAKE_CXX_COMPILER_ID}/${PLATFORM_TARGET}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_BUILD_TYPE}/${Qt5Core_VERSION}/${CMAKE_CXX_COMPILER_ID}/${PLATFORM_TARGET}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/IzModels
)

install(
    FILES IzModelsConfig.cmake
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)

# export target
install(
EXPORT IzModels-export
FILE
    IzModelsTargets.cmake
NAMESPACE
    IzModels::
DESTINATION cmake/IzModels
)

get_filename_component(IzModels_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

include(CMakeFindDependencyMacro)

find_dependency(Qt6 REQUIRED COMPONENTS Core Sql Qml Concurrent)

if(NOT TARGET IzModels::IzModels)
    include("${IzModels_CMAKE_DIR}/cmake/IzModels/IzModelsTargets.cmake")
endif()

cmake_minimum_required(VERSION 3.21)

project(GitDesk VERSION 0.2.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")
endif()

if (NOT DEFINED CMAKE_PREFIX_PATH OR CMAKE_PREFIX_PATH STREQUAL "")
    list(PREPEND CMAKE_PREFIX_PATH "D:/Qt/6.10.2/msvc2022_64")
endif()

find_package(Qt6 6.10 REQUIRED COMPONENTS
    Quick
    QuickControls2
    QuickEffects
    Widgets
    Concurrent
)
qt_standard_project_setup(REQUIRES 6.10)

# Pin packaged Md3 (v1.1.x). Prefer ./Md3 beside this project.
set(MD3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Md3" CACHE PATH "Packaged Md3 next to this project")
if (NOT EXISTS "${MD3_DIR}/lib/cmake/Md3/Md3Config.cmake")
    if (EXISTS "D:/QML_MD3/QML_MD3/dist/Md3/lib/cmake/Md3/Md3Config.cmake")
        set(MD3_DIR "D:/QML_MD3/QML_MD3/dist/Md3" CACHE PATH "" FORCE)
    elseif (EXISTS "$ENV{LOCALAPPDATA}/Md3/lib/cmake/Md3/Md3Config.cmake")
        set(MD3_DIR "$ENV{LOCALAPPDATA}/Md3" CACHE PATH "" FORCE)
    endif()
endif()

if (NOT EXISTS "${MD3_DIR}/lib/cmake/Md3/Md3Config.cmake")
    message(FATAL_ERROR
        "Md3 package not found at:\n  ${MD3_DIR}\n"
        "Junction/copy QML_MD3 dist/Md3 → ./Md3 (pin tag v1.1.2).")
endif()

list(APPEND CMAKE_PREFIX_PATH "${MD3_DIR}")
find_package(Md3 REQUIRED CONFIG)

qt_add_executable(appGitDesk
    main.cpp
)

set_source_files_properties(Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)

set(_gd_pages
    qml/pages/OverviewPage.qml
    qml/pages/GraphPage.qml
    qml/pages/ChangesPage.qml
    qml/pages/HistoryPage.qml
    qml/pages/SettingsPage.qml
)
foreach (_f IN LISTS _gd_pages)
    set_source_files_properties(${_f} PROPERTIES QT_RESOURCE_ALIAS ${_f})
endforeach()

set(_gd_components
    qml/components/TopToolbar.qml
    qml/components/RepoExplorer.qml
    qml/components/DetailPanel.qml
    qml/components/GitGraphCanvas.qml
    qml/components/DiffViewer.qml
    qml/components/CommitComposer.qml
    qml/components/StatCard.qml
    qml/components/WelcomeView.qml
    qml/components/AboutContent.qml
)
foreach (_f IN LISTS _gd_components)
    set_source_files_properties(${_f} PROPERTIES QT_RESOURCE_ALIAS ${_f})
endforeach()

qt_add_qml_module(appGitDesk
    URI GitDesk
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        Main.qml
        ${_gd_pages}
        ${_gd_components}
    SOURCES
        src/core/VersionInfo.h
        src/core/VersionInfo.cpp
        src/core/LocaleController.h
        src/core/LocaleController.cpp
        src/git/GitRunner.h
        src/git/GitRunner.cpp
        src/git/GitRepository.h
        src/git/GitRepository.cpp
        src/models/CommitListModel.h
        src/models/CommitListModel.cpp
        src/models/FileChangeModel.h
        src/models/FileChangeModel.cpp
        src/models/BranchListModel.h
        src/models/BranchListModel.cpp
        src/models/GraphNodeModel.h
        src/models/GraphNodeModel.cpp
        src/viewmodels/GitDeskApp.h
        src/viewmodels/GitDeskApp.cpp
)

qt_add_resources(appGitDesk "gitdesk_assets"
    PREFIX "/gitdesk"
    FILES
        resources/version.json
        i18n/gitdesk_en.qm
)

target_include_directories(appGitDesk PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/git"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/models"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/viewmodels"
)

target_compile_definitions(appGitDesk PRIVATE
    GITDESK_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(appGitDesk PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::Concurrent
    Md3::Md3
)

if (TARGET Md3::QmlPlugin)
    target_link_libraries(appGitDesk PRIVATE Md3::QmlPlugin)
endif()

if (Md3_SHARED)
    target_compile_definitions(appGitDesk PRIVATE MD3_SHARED)
endif()

set_target_properties(appGitDesk PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    OUTPUT_NAME GitDesk
)

qt_import_qml_plugins(appGitDesk)

if (EXISTS "${MD3_DIR}/lib/cmake/Md3/Md3DeployShared.cmake")
    include("${MD3_DIR}/lib/cmake/Md3/Md3DeployShared.cmake")
endif()
if (COMMAND md3_deploy_shared_runtime)
    md3_deploy_shared_runtime(appGitDesk "${MD3_DIR}")
elseif (Md3_SHARED AND WIN32 AND EXISTS "${MD3_DIR}/bin/Md3.dll")
    add_custom_command(TARGET appGitDesk POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MD3_DIR}/bin/Md3.dll" "$<TARGET_FILE_DIR:appGitDesk>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MD3_DIR}/lib/qml" "$<TARGET_FILE_DIR:appGitDesk>/qml"
        COMMAND ${CMAKE_COMMAND}
            -DMD3_QML_DIR=$<TARGET_FILE_DIR:appGitDesk>/qml
            -P "${MD3_DIR}/lib/cmake/Md3/Md3StripQmldirPrefer.cmake"
        COMMENT "Deploy shared Md3 runtime beside GitDesk"
    )
endif()

include(GNUInstallDirs)
install(TARGETS appGitDesk
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

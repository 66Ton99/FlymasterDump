#############################################################
# QT
#############################################################
if(APPLE AND EXISTS "/opt/homebrew/opt/qt@5")
    list(PREPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt@5")
endif()

find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)

if(WIN32)
    get_filename_component(QT_DIR ${QT_QMAKE_EXECUTABLE} DIRECTORY)
    if(EXISTS "${QT_DIR}/Qt5Core.dll")
        install( FILES ${QT_DIR}/Qt5Core.dll
                       ${QT_DIR}/Qt5Widgets.dll
                       ${QT_DIR}/Qt5Gui.dll DESTINATION . )
        install( FILES ${QT_DIR}/../plugins/platforms/qwindows.dll DESTINATION ./platforms )
        file( COPY ${QT_DIR}/Qt5Core.dll DESTINATION ${PROJECT_BINARY_DIR}/app )
        file( COPY ${QT_DIR}/Qt5Widgets.dll DESTINATION ${PROJECT_BINARY_DIR}/app )
        file( COPY ${QT_DIR}/Qt5Gui.dll DESTINATION ${PROJECT_BINARY_DIR}/app )
    endif()
endif()

#############################################################
# BOOST
#############################################################
if(APPLE AND EXISTS "/opt/homebrew/opt/boost")
    list(PREPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/boost")
endif()

find_package(Boost REQUIRED COMPONENTS system date_time)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

#############################################################
# GTest
#############################################################
if(APPLE AND EXISTS "/opt/homebrew/opt/googletest")
    list(PREPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/googletest")
endif()

find_package(GTest REQUIRED)
include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})
if(TARGET GTest::gtest)
    set(GTEST_LIBS GTest::gtest GTest::gtest_main)
else()
    set(GTEST_LIBS GTest::GTest GTest::Main)
endif()

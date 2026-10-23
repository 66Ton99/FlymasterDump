set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set( UIS
    MainWindow.ui
)
set_source_files_properties(${UIS} PROPERTIES HEADER_FILE_ONLY TRUE)

add_library( varioDumpUiLib
    CheckDeviceTask.h
    CheckDeviceTask.cpp
    DumpTask.h
    DumpTask.cpp
    FlightListTask.h
    FlightListTask.cpp
    MainWindow.h
    MainWindow.cpp
    Model.h
    Model.cpp
    Property.h
    QtCallback.h
    QtCallback.cpp
    TaskProcessor.h
    TaskProcessor.cpp
    TaskProgressDialog.h
    TaskProgressDialog.cpp
    ${UIS}
    )
target_link_libraries( varioDumpUiLib
    progressLib
    serialPortComLib
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
)
if(WIN32)
    target_compile_definitions( varioDumpUiLib PRIVATE BOOST_ALL_DYN_LINK _WIN32_WINNT=0x0501)
endif()

add_executable( variodump main.cpp )
target_link_libraries( variodump
    varioDumpUiLib
    progressLib
    serialPortComLib
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
)
if(MSVC)
    set_target_properties(variodump PROPERTIES WIN32_EXECUTABLE YES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()
if(WIN32)
    target_compile_definitions( variodump PRIVATE BOOST_ALL_DYN_LINK _WIN32_WINNT=0x0501)
endif()
install(TARGETS variodump RUNTIME DESTINATION .)

add_executable( variodumpTest
    TaskProcessor.test.cpp
    )
target_link_libraries( variodumpTest
    varioDumpUiLib
    progressLib
    serialPortComLib
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    ${GTEST_LIBS}
)
if(WIN32)
    target_compile_definitions( variodumpTest PRIVATE BOOST_ALL_DYN_LINK _WIN32_WINNT=0x0501)
endif()
add_test(NAME variodumpTest COMMAND variodumpTest )

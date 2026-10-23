add_library( serialPortComLib
    ascii/FlightMetaInfo.h
    ascii/FlightMetaInfo.cpp
    ascii/NmeaSentence.h
    ascii/NmeaSentence.cpp
    ascii/StringUtils.h
    ascii/StringUtils.cpp
    binary/BinaryBlock.h
    binary/BinaryBlock.cpp
    binary/Convert.h
    binary/Convert.cpp
    binary/FlightInformationRecord.h
    binary/FlightInformationRecord.cpp
    binary/KeyTrackPositionRecord.h
    binary/KeyTrackPositionRecord.cpp
    binary/TrackPositionRecordDeltas.h
    binary/TrackPositionRecordDeltas.cpp
    IgcFactory.h
    IgcFactory.cpp
    IgcFile.h
    IgcFile.cpp
    Request.h
    Request.cpp
    SerialPort.h
    SerialPort.cpp
    )
target_link_libraries( serialPortComLib
    progressLib
    Boost::system
    Boost::date_time
)
if(WIN32)
    target_compile_definitions( serialPortComLib PRIVATE BOOST_ALL_DYN_LINK _WIN32_WINNT=0x0501)
endif()

add_executable( serialPortComLibTest
    ascii/NmeaSentence.test.cpp
    ascii/FlightMetaInfo.test.cpp
    ascii/StringUtils.test.cpp
    binary/BinaryBlock.test.cpp
    binary/Convert.test.cpp
    Request.test.cpp
    )
target_link_libraries( serialPortComLibTest
    progressLib
    serialPortComLib
    ${GTEST_LIBS}
)
if(WIN32)
    target_compile_definitions( serialPortComLibTest PRIVATE BOOST_ALL_DYN_LINK _WIN32_WINNT=0x0501)
endif()
add_test(NAME serialPortComLibTest COMMAND serialPortComLibTest )

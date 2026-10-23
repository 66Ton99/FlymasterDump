add_library( progressLib
    Callback.h    
    Callback.cpp
    )
if(WIN32)
    target_compile_definitions( progressLib PRIVATE BOOST_ALL_DYN_LINK _WIN32_WINNT=0x0501)
endif()

add_executable( progressLibTest
    Callback.test.cpp
    )
target_link_libraries( progressLibTest
    progressLib
    ${GTEST_LIBS}
)
if(WIN32)
    target_compile_definitions( progressLibTest PRIVATE BOOST_ALL_DYN_LINK _WIN32_WINNT=0x0501)
endif()
add_test(NAME progressLibTest COMMAND progressLibTest)

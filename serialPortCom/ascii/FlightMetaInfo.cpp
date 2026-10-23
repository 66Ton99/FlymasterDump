#include "FlightMetaInfo.h"
#include <stdexcept>

FlightMetaInfo::FlightMetaInfo( const NmeaSentence& sentence ) {
    // Sample varioOutput; $PFMLST,056,055,27.06.16,12:14:58,00:23:49*3C
    // Interpretation:     $PFMLST, COUNT, INDEX, DATE, TIME, DURATION*??

    if( sentence.command != "PFMLST" ) {
        throw std::runtime_error( "Tried to parse invalid flight meta info" );
    }
    index = atoi( sentence.values[1].c_str() );
    date.year = sentence.values[2].substr( 6, 2 );
    date.month = sentence.values[2].substr( 3, 2 );
    date.day = sentence.values[2].substr( 0, 2 );
    time.hours = sentence.values[3].substr( 0, 2 );
    time.minutes = sentence.values[3].substr( 3, 2 );
    time.seconds = sentence.values[3].substr( 6, 2 );
    duration.hours = sentence.values[4].substr( 0, 2 );
    duration.minutes = sentence.values[4].substr( 3, 2 );
    duration.seconds = sentence.values[4].substr( 6, 2 );
}

std::string FlightMetaInfo::flightRequest() const {
    // example request $PFMDNL,160617120405,*18
    NmeaSentence s;
    s.command = "PFMDNL";
    s.values.push_back( date.year + date.month + date.day + time.hours + time.minutes + time.seconds );
    s.values.push_back( "" );
    return s.build();
}

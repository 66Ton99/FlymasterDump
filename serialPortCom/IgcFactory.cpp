#include "IgcFactory.h"

#include "binary/Convert.h"
#include "binary/FlightInformationRecord.h"
#include "binary/KeyTrackPositionRecord.h"
#include "binary/TrackPositionRecordDeltas.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <tuple>
#include <cmath>

namespace {

void setUtcTimeZone() {
#ifdef _WIN32
    _putenv( "TZ=UTC" );
#else
    setenv( "TZ", "UTC", 1 );
    tzset();
#endif
}

std::tuple< FixedSizeString< 2 >, FixedSizeString< 2 >, FixedSizeString< 2 > > convertTime( int seconds ) {
    struct tm timeinfo = {0};
    timeinfo.tm_year = 2000 - 1900;
    timeinfo.tm_mon = 0;
    timeinfo.tm_mday = 0;
    timeinfo.tm_hour = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = seconds;
    setUtcTimeZone();
    mktime( &timeinfo );
    FixedSizeString< 2 > hh;
    // length 3 is ok since fixedstr of length 2 is a char[3] with last entry \0
    // as crerated by strftime
    strftime( hh, 3, "%H", &timeinfo );
    FixedSizeString< 2 > mm;
    strftime( mm, 3, "%M", &timeinfo );
    FixedSizeString< 2 > ss;
    strftime( ss, 3, "%S", &timeinfo );
    return std::make_tuple( hh, mm, ss );
}

FixedSizeString< 6 > convertDate( int seconds ) {
    struct tm timeinfo = {0};
    timeinfo.tm_year = 2000 - 1900;
    timeinfo.tm_mon = 0;
    timeinfo.tm_mday = 0;
    timeinfo.tm_hour = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = seconds;
    setUtcTimeZone();
    mktime( &timeinfo );
    FixedSizeString< 6 > date;
    strftime( date, 7, "%d%m%y", &timeinfo );
    return date;
}

FixedSizeString< 8 > convertLatitude( int32_t l ) {
    double ddeg = l / 60000.0;
    auto deg = floor( ddeg );
    ddeg -= deg;
    auto min = floor( ddeg * 60.0 );
    ddeg -= min / 60.0;
    auto sec = round( ddeg * 60000.0 );
    FixedSizeString< 8 > buf;
    snprintf( buf, sizeof( buf ), "%02.lf%02.lf%03.lf", deg, min, sec );
    buf[7] = l > 0 ? 'N' : 'S';
    return buf;
}

FixedSizeString< 9 > convertLongiude( int32_t l ) {
    double ddeg = fabs( l / 60000.0 );
    auto deg = floor( ddeg );
    ddeg -= deg;
    auto min = floor( ddeg * 60.0 );
    ddeg -= min / 60.0;
    auto sec = round( ddeg * 60000.0 );
    FixedSizeString< 9 > buf;
    snprintf( buf, sizeof( buf ), "%03.lf%02.lf%03.lf", deg, min, sec );
    buf[8] = l > 0 ? 'W' : 'E';
    return buf;
}

FixedSizeString< 5 > convertAltitude( int16_t a ) {
    FixedSizeString< 5 > buf;
    snprintf( buf, sizeof( buf ), "%05d", a );
    return buf;
}

FixedSizeString< 5 > convertPressure( int16_t p ) {
    auto a = round( ( 1 - pow( fabs( ( p / 10.0 ) / 1013.25 ), 0.190284 ) ) * 44307.69 );
    return convertAltitude( static_cast<int16_t>( a ) );
}

BRecord convertPositionRecord( const KeyTrackPositionRecord& ktpr ) {
    BRecord b;
    std::tie( b.hour, b.minutes, b.seconds ) = convertTime( ktpr.time );
    b.latitude = convertLatitude( ktpr.latiude );
    b.longitude = convertLongiude( ktpr.longitude );
    b.validAltitude = ktpr.fixFlag & 255 ? "A" : "V";
    b.altitudeGPS = convertAltitude( ktpr.altiude );
    b.altitudePressure = convertPressure( ktpr.pressure );
    return b;
}

} // namespace

IgcFile buildIgc( const FlightMetaInfo& fmi, const BinaryFlightData& bfd ) {
    FlightInformationRecord fir( bfd );
    KeyTrackPositionRecord ktpr( bfd );
    TrackPositionRecordDeltas tprd( bfd );

    IgcFile file;
    file.aRecord.manufacturer = "XMP";
    file.aRecord.uid = "   ";
    file.aRecord.id = "Flymaster GpsSD SN " + std::to_string( fir.deviceSerial );

    file.hRecord.flightDate = convertDate( ktpr.time );

    file.bRecords.push_back( convertPositionRecord( ktpr ) );
    for( const auto& delta : tprd.deltas ) {
        ktpr.altiude += delta.altitude;
        ktpr.fixFlag = delta.fixFlag;
        ktpr.latiude += delta.latitude;
        ktpr.longitude += delta.longitude;
        ktpr.pressure += delta.pressure;
        ktpr.time += delta.time;
        file.bRecords.push_back( convertPositionRecord( ktpr ) );
    }

    return file;
}

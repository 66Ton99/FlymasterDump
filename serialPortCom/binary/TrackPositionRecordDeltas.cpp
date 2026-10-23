#include "TrackPositionRecordDeltas.h"

#include "Convert.h"
#include <stdexcept>

TrackPositionRecordDeltas::TrackPositionRecordDeltas( const BinaryFlightData& bfd ) {
    std::string allData;
    for( size_t i = 2; i < bfd.size(); ++i ) {
        if( bfd[i].packetId != PacketId::TRACK_POSITION_RECORD_DELTA ) {
            throw std::runtime_error( "Invalid packet id" );
        }
        allData += bfd[i].data;
    }

    if( allData.size() % sizeof( Delta ) != 0 ) {
        throw std::runtime_error( "Incomplete data" );
    }

    size_t start = 0;
    while( start < allData.size() ) {
        Delta d;
        d.fixFlag = fromBytes<uint8_t>( &allData[start] );
        d.latitude = fromBytes<int8_t>( &allData[start + 1] );
        d.longitude = fromBytes<int8_t>( &allData[start + 2] );
        d.altitude = fromBytes<int8_t>( &allData[start + 3] );
        d.pressure = fromBytes<int8_t>( &allData[start + 4] );
        d.time = fromBytes<int8_t>( &allData[start + 5] );
        deltas.emplace_back( std::move( d ) );
        start += sizeof( Delta );
    }
}

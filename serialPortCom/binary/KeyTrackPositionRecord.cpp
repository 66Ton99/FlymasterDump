#include "KeyTrackPositionRecord.h"

#include "Convert.h"
#include <stdexcept>

KeyTrackPositionRecord::KeyTrackPositionRecord( const BinaryFlightData& bfd ) {
    if( bfd[1].packetId != PacketId::KEY_TRACK_POSITION_RECORD ) {
        throw std::runtime_error( "Invalid packet id" );
    }
    if( bfd[1].length != 0x11 ) {
        throw std::runtime_error( "Invalid length" );
    }
    const std::string& data = bfd[1].data;

    fixFlag = data[0];
    latiude = fromBytes<int32_t>( &bfd[1].data[1] );
    longitude = fromBytes<int32_t>( &bfd[1].data[5] );
    altiude = fromBytes<int16_t>( &bfd[1].data[9] );
    pressure = fromBytes<int16_t>( &bfd[1].data[11] );
    time = fromBytes<uint32_t>( &bfd[1].data[13] );
}

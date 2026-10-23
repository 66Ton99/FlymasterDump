#include "BinaryBlock.h"

#include "Convert.h"
#include <stdexcept>

BinaryBlock::BinaryBlock( const std::string& bytes ) {
    if( bytes.size() < 2 ) {
        throw std::runtime_error( "No packet id" );
    }
    packetId = static_cast< PacketId >( fromBytes<uint16_t>( bytes.c_str() ) );

    if( packetId == PacketId::END_OF_DATA ) {
        return;
    }
    switch( packetId ) {
        case PacketId::FLIGHT_INFORMAION_RECORD:
        case PacketId::KEY_TRACK_POSITION_RECORD:
        case PacketId::TRACK_POSITION_RECORD_DELTA:
            break;
        default:
            throw std::runtime_error( "Unknown packet id" );
    }

    if( bytes.size() < 3 ) {
        throw std::runtime_error( "No length" );
    }
    length = fromBytes<uint8_t>( &bytes[2] );

    if( bytes.size() < 3 + length + 1 ) {
        throw std::runtime_error( "Received " + std::to_string( bytes.size() ) + " bytes but expected "
                                  + std::to_string( 3 + length + 1 ) + " bytes" );
    }
    data = bytes.substr( 3, length );

    if( crc( bytes.substr( 2, length + 1 ) ) != bytes[3 + length] ) {
        throw std::runtime_error( "Invalid CRC" );
    }
}

char BinaryBlock::crc( const std::string& str ) const {
    char crc = 0;
    for( auto& c : str ) {
        crc ^= c;
    }
    return crc;
}

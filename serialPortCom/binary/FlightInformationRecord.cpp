#include "FlightInformationRecord.h"

#include "Convert.h"
#include <stdexcept>

FlightInformationRecord::FlightInformationRecord( const BinaryFlightData& bfd ) {
    if( bfd[0].packetId != PacketId::FLIGHT_INFORMAION_RECORD ) {
        throw std::runtime_error( "Invalid packet id" );
    }
    if( bfd[0].length != 0x3f ) {
        throw std::runtime_error( "Invalid lengh" );
    }
    const std::string& data = bfd[0].data;
    firmwareVersion = fromBytes<int16_t>( &bfd[0].data[0] );
    hardwareVersion = fromBytes<int16_t>( &bfd[0].data[2] );
    deviceSerial = fromBytes<int32_t>( &bfd[0].data[4] );
    pilotCompetitionNumber = data.substr( 8, 8 );
    pilotName = data.substr( 16, 15 );
    gliderBrand = data.substr( 31, 15 );
    gliderModel = data.substr( 46, 15 );
}

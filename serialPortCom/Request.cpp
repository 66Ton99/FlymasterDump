#include "Request.h"

#include "ascii/StringUtils.h"
#include "binary/Convert.h"
#include <stdexcept>

namespace {

enum class Status : uint8_t { Ok = 0xb1, Error = 0xb2, Cancel = 0xb3 };

std::string toString( Status s ) {
    return toBytes( static_cast< uint8_t >( s ) );
}

} // namespace

DeviceInfo parseDeviceInfo( const std::string& sentence ) {
    DeviceInfo info;
    info.raw = sentence;

    std::string trimmed = sentence;
    while( !trimmed.empty() && ( trimmed.back() == '\r' || trimmed.back() == '\n' || trimmed.back() == ' ' ) ) {
        trimmed.pop_back();
    }
    while( !trimmed.empty() && ( trimmed.front() == '\r' || trimmed.front() == '\n' || trimmed.front() == ' ' ) ) {
        trimmed.erase( trimmed.begin() );
    }

    if( trimmed.empty() ) {
        throw std::runtime_error( "Empty response from device" );
    }

    NmeaSentence s( trimmed );
    if( s.command != "PFMSNP" ) {
        throw std::runtime_error( "Invalid response command: expected PFMSNP, got " + s.command );
    }

    if( s.values.empty() || s.values[0].empty() ) {
        throw std::runtime_error( "Missing model in PFMSNP response" );
    }
    info.model = s.values[0];

    if( s.values.size() <= 2 || s.values[2].empty() ) {
        throw std::runtime_error( "Missing serial number in PFMSNP response" );
    }
    info.serialNumber = s.values[2];

    if( s.values.size() > 3 ) {
        info.firmware = s.values[3];
    }
    if( s.values.size() > 4 ) {
        std::string p = s.values[4];
        while( !p.empty() && p.front() == ' ' )
            p.erase( p.begin() );
        info.pressure = p;
    }
    return info;
}

std::string requestSerial( SerialPort& port, Callback& cb ) {
    cb.notify( Severity::Debug, "Requesting serial" );
    auto serial = port.request( "$PFMSNP,*3A" );
    cb.notify( Severity::Debug, "Serial: " + serial );
    return serial;
}

DeviceInfo requestDeviceInfo( SerialPort& port, Callback& cb ) {
    cb.notify( Severity::Info, "Checking Flymaster device connection..." );
    std::string resp = requestSerial( port, cb );
    DeviceInfo info = parseDeviceInfo( resp );
    cb.notify( Severity::Info,
               "Device: " + info.model + ", Serial: " + info.serialNumber + ", Firmware: " + info.firmware );
    return info;
}

std::vector< FlightMetaInfo > requestFlightInfos( SerialPort& port, Callback& cb ) {
    cb.notify( Severity::Info, "Requesting flight infos" );
    auto list = split( port.request( "$PFMDNL,LST,*56" ), '\n' );
    std::vector< FlightMetaInfo > fmi;
    for( const auto& item : list ) {
        cb.notify( Severity::Debug, "Received: " + item );
        std::string trimmed = item;
        while( !trimmed.empty() && ( trimmed.back() == '\r' || trimmed.back() == '\n' || trimmed.back() == ' ' ) ) {
            trimmed.pop_back();
        }
        while( !trimmed.empty() && ( trimmed.front() == '\r' || trimmed.front() == '\n' || trimmed.front() == ' ' ) ) {
            trimmed.erase( trimmed.begin() );
        }
        if( trimmed.empty() ) {
            continue;
        }
        NmeaSentence sentence( trimmed );
        fmi.emplace_back( sentence );
    }
    cb.notify( Severity::Info, "#Flights: " + std::to_string( fmi.size() ) );
    return fmi;
}

BinaryFlightData requestFlight( SerialPort& port, FlightMetaInfo& info, Callback& cb ) {
    cb.notify( Severity::Info, "Requesting Flight #" + std::to_string( info.index ) );

    std::vector< BinaryBlock > blocks;
    size_t errorCount = 0;
    size_t bytes = 0;
    std::string request = info.flightRequest();
    while( true ) {
        try {
            BinaryBlock block = port.request( request );
            if( block.packetId == PacketId::END_OF_DATA ) {
                break;
            }
            blocks.push_back( block );
            request = toString( Status::Ok );
            errorCount = 0;
            cb.notify( Severity::Debug, "OK" );
            if( cb.isCancelled() ) {
                port.request( toString( Status::Cancel ) );
                break;
            }
            bytes += block.data.size();
            cb.notify( Severity::Progress, "Received " + std::to_string( bytes ) + " bytes" );
        } catch( std::exception& e ) {
            ++errorCount;
            if( errorCount == 5 ) {
                cb.notify( Severity::Critical, std::string( "Aborting: " ) + e.what() );
                port.request( toString( Status::Cancel ) );
                break;
            } else {
                cb.notify( Severity::Warning, std::string( "Retry: Received invalid block: " ) + e.what() );
                request = toString( Status::Error );
            }
        }
    }
    cb.notify( Severity::Info, "#Blocks: " + std::to_string( blocks.size() ) );
    return blocks;
}

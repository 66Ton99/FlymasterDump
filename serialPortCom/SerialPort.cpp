#include "SerialPort.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace boost::asio;

SerialPort::SerialPort( const std::string& port )
    : port_( ioContext_ ), timeout_( ioContext_ ), timeoutMs_( 1000 ), interByteTimeoutMs_( 100 ) {
    boost::system::error_code ec;
    port_.open( port, ec );
    if( ec || !port_.is_open() ) {
        throw std::runtime_error( "Failed to connect to " + port + ": " + ec.message() );
    }
    setSpeed( Speed::BAUD115200 );
}

SerialPort::~SerialPort() {
    boost::system::error_code ec;
    port_.close( ec );
}

void SerialPort::setSpeed( Speed s ) {
    port_.set_option( serial_port_base::baud_rate( static_cast< unsigned int >( s ) ) );
}

void SerialPort::setTimeout( unsigned int ms ) {
    timeoutMs_ = ms;
}

void SerialPort::setInterByteTimeout( unsigned int ms ) {
    interByteTimeoutMs_ = ms;
}

bool SerialPort::isFrameComplete( const std::string& resp ) {
    if( resp.size() < 2 ) {
        return false;
    }
    uint16_t pid
        = static_cast< uint8_t >( resp[0] ) | ( static_cast< uint16_t >( static_cast< uint8_t >( resp[1] ) ) << 8 );
    if( pid == 0xa3a3 ) {
        return resp.size() >= 2;
    }
    if( pid == 0xa0a0 || pid == 0xa1a1 || pid == 0xa2a2 ) {
        if( resp.size() < 3 ) {
            return false;
        }
        uint8_t payloadLen = static_cast< uint8_t >( resp[2] );
        return resp.size() >= static_cast< size_t >( payloadLen + 4 );
    }
    return false;
}

std::string SerialPort::request( const std::string& request ) {
    writeRequest( request );
    return readResponse();
}

void SerialPort::writeRequest( const std::string& request ) {
    streambuf requestBuffer;
    std::ostream requestStream( &requestBuffer );
    requestStream << request << "\n";
    write( port_, requestBuffer );
}

std::string SerialPort::readResponse() {
    response_ = "";
    dataAvailable_ = false;
    ioContext_.restart();
    port_.async_read_some( buffer( buffer_, 255 ),
                           [this]( const boost::system::error_code& error, std::size_t bytes_transferred ) {
                               read_callback( error, bytes_transferred );
                           } );
    timeout_.expires_after( std::chrono::milliseconds( timeoutMs_ ) );
    timeout_.async_wait( [this]( const boost::system::error_code& error ) { wait_callback( error ); } );

    ioContext_.run(); // will block until async callbacks are finished

    return response_;
}

void SerialPort::read_callback( const boost::system::error_code& error, size_t bytes_transferred ) {
    if( error || !bytes_transferred ) {
        dataAvailable_ = false;
        return;
    }

    timeout_.cancel();

    for( size_t i = 0; i < bytes_transferred; ++i ) {
        response_ += buffer_[i];
    }
    dataAvailable_ = true;

    if( isFrameComplete( response_ ) ) {
        return;
    }

    port_.async_read_some( buffer( buffer_, 255 ),
                           [this]( const boost::system::error_code& err, std::size_t transferred ) {
                               read_callback( err, transferred );
                           } );
    timeout_.expires_after( std::chrono::milliseconds( interByteTimeoutMs_ ) );
    timeout_.async_wait( [this]( const boost::system::error_code& err ) { wait_callback( err ); } );
}

void SerialPort::wait_callback( const boost::system::error_code& error ) {
    if( error ) {
        return;
    }
    boost::system::error_code ec;
    port_.cancel( ec );
}

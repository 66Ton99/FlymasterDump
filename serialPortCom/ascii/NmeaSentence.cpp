#include "NmeaSentence.h"

#include "StringUtils.h"

#include <sstream>
#include <stdexcept>

NmeaSentence::NmeaSentence( const std::string& sentence ) {
    auto strs = checkAndStrip( sentence );

    auto details = split( strs, ',' );
    command = details[0];
    values.reserve( details.size() - 1 );
    for( size_t i = 1; i < details.size(); ++i ) {
        values.push_back( details[i] );
    }
}

std::string NmeaSentence::build() const {
    return "$" + command + "," + join( values, ',' ) + "*"
           + printChecksum( calculateChecksum( command + "," + join( values, ',' ) ) );
}

int NmeaSentence::calculateChecksum( const std::string& str ) const {
    int checksum = 0;
    for( char c : str ) {
        checksum ^= c;
    }
    return checksum;
}

short NmeaSentence::parseChecksum( const std::string& cs ) const {
    short checksum;
    std::stringstream ss;
    ss << cs;
    ss >> std::hex >> checksum;
    return checksum;
}

std::string NmeaSentence::printChecksum( short s ) const {
    char buffer[8];
    snprintf( buffer, sizeof( buffer ), "%X", static_cast< unsigned short >( s ) );
    return std::string( buffer );
}

std::string NmeaSentence::checkAndStrip( const std::string& sentence ) {
    if( sentence.empty() || sentence.front() != '$' ) {
        throw std::runtime_error( "Invalid NmeaSentence: \"" + sentence + "\"" );
    }
    auto noStartEnd = sentence.substr( 1, sentence.size() - 1 );

    auto sc = split( noStartEnd, '*' );
    auto checksum = parseChecksum( sc[1] );
    auto expectedChecksum = calculateChecksum( sc[0] );
    if( checksum != expectedChecksum ) {
        throw std::runtime_error( "Invalid Checksum in NmeaSentence: \"" + sentence + "\", found "
                                  + std::to_string( checksum ) + ", expected " + std::to_string( expectedChecksum ) );
    }
    return sc[0];
}

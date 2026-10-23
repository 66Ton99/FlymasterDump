#include "NmeaSentence.h"

#include "StringUtils.h"

#include <cctype>
#include <sstream>
#include <stdexcept>

NmeaSentence::NmeaSentence( const std::string& sentence ) {
    auto strs = checkAndStrip( sentence );

    auto details = split( strs, ',' );
    if( details.empty() || details[0].empty() ) {
        throw std::runtime_error( "Invalid NmeaSentence (empty command): \"" + sentence + "\"" );
    }
    command = details[0];
    values.clear();
    if( details.size() > 1 ) {
        values.reserve( details.size() - 1 );
        for( size_t i = 1; i < details.size(); ++i ) {
            values.push_back( details[i] );
        }
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
    std::string clean = cs;
    while( !clean.empty() && ( clean.back() == '\r' || clean.back() == '\n' || clean.back() == ' ' ) ) {
        clean.pop_back();
    }
    while( !clean.empty() && ( clean.front() == '\r' || clean.front() == '\n' || clean.front() == ' ' ) ) {
        clean.erase( clean.begin() );
    }
    if( clean.empty() ) {
        throw std::runtime_error( "Empty checksum in NmeaSentence" );
    }
    for( char c : clean ) {
        if( !isxdigit( static_cast< unsigned char >( c ) ) ) {
            throw std::runtime_error( "Invalid hex character in checksum: \"" + clean + "\"" );
        }
    }
    short checksum = 0;
    std::stringstream ss;
    ss << clean;
    if( !( ss >> std::hex >> checksum ) ) {
        throw std::runtime_error( "Failed to parse hex checksum: \"" + clean + "\"" );
    }
    return checksum;
}

std::string NmeaSentence::printChecksum( short s ) const {
    char buffer[8];
    snprintf( buffer, sizeof( buffer ), "%X", static_cast< unsigned short >( s ) );
    return std::string( buffer );
}

std::string NmeaSentence::checkAndStrip( const std::string& sentence ) {
    std::string s = sentence;
    while( !s.empty() && ( s.back() == '\r' || s.back() == '\n' || s.back() == ' ' ) ) {
        s.pop_back();
    }
    while( !s.empty() && ( s.front() == '\r' || s.front() == '\n' || s.front() == ' ' ) ) {
        s.erase( s.begin() );
    }

    if( s.empty() || s.front() != '$' ) {
        throw std::runtime_error( "Invalid NmeaSentence: \"" + sentence + "\"" );
    }
    auto starPos = s.find( '*' );
    if( starPos == std::string::npos || starPos == 1 ) {
        throw std::runtime_error( "Invalid NmeaSentence (missing checksum delimiter '*'): \"" + sentence + "\"" );
    }

    auto noStartEnd = s.substr( 1 );
    auto sc = split( noStartEnd, '*' );
    if( sc.size() != 2 || sc[0].empty() || sc[1].empty() ) {
        throw std::runtime_error( "Invalid NmeaSentence format: \"" + sentence + "\"" );
    }

    auto checksum = parseChecksum( sc[1] );
    auto expectedChecksum = calculateChecksum( sc[0] );
    if( checksum != expectedChecksum ) {
        throw std::runtime_error( "Invalid Checksum in NmeaSentence: \"" + sentence + "\", found "
                                  + std::to_string( checksum ) + ", expected " + std::to_string( expectedChecksum ) );
    }
    return sc[0];
}

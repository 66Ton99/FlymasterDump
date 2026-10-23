#pragma once

#include "SerialPort.h"
#include "ascii/FlightMetaInfo.h"
#include "binary/BinaryBlock.h"

#include <progress/Callback.h>
#include <string>
#include <vector>

struct DeviceInfo {
    std::string model;
    std::string serialNumber;
    std::string firmware;
    std::string pressure;
    std::string raw;
};

DeviceInfo parseDeviceInfo( const std::string& sentence );

std::string requestSerial( SerialPort& port, Callback& cb );

DeviceInfo requestDeviceInfo( SerialPort& port, Callback& cb );

std::vector< FlightMetaInfo > requestFlightInfos( SerialPort& port, Callback& cb );

BinaryFlightData requestFlight( SerialPort& port, FlightMetaInfo& info, Callback& cb );

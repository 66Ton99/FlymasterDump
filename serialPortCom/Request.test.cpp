#include "Request.h"
#include <gtest/gtest.h>

TEST( RequestTest, parseDeviceInfoStandard ) {
    std::string sentence = "$PFMSNP,Gps,,01229,2.03g, 953.36,,b203*59\r\n";
    DeviceInfo info = parseDeviceInfo( sentence );

    EXPECT_EQ( "Gps", info.model );
    EXPECT_EQ( "01229", info.serialNumber );
    EXPECT_EQ( "2.03g", info.firmware );
    EXPECT_EQ( "953.36", info.pressure );
}

TEST( RequestTest, parseDeviceInfoSD ) {
    std::string sentence = "$PFMSNP,GpsSD,,00857,1.06p,1019.87,*3E";
    DeviceInfo info = parseDeviceInfo( sentence );

    EXPECT_EQ( "GpsSD", info.model );
    EXPECT_EQ( "00857", info.serialNumber );
    EXPECT_EQ( "1.06p", info.firmware );
    EXPECT_EQ( "1019.87", info.pressure );
}

TEST( RequestTest, parseDeviceInfoEmpty ) {
    DeviceInfo info = parseDeviceInfo( "" );
    EXPECT_TRUE( info.model.empty() );
    EXPECT_TRUE( info.serialNumber.empty() );
}

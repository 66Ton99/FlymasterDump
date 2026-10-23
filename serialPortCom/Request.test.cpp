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
    EXPECT_THROW( parseDeviceInfo( "" ), std::runtime_error );
    EXPECT_THROW( parseDeviceInfo( "   \r\n" ), std::runtime_error );
}

TEST( RequestTest, parseDeviceInfoTruncated ) {
    EXPECT_THROW( parseDeviceInfo( "$PFMSNP,Gps" ), std::runtime_error );
}

TEST( RequestTest, parseDeviceInfoInvalidResponses ) {
    EXPECT_THROW( parseDeviceInfo( "garbage data" ), std::runtime_error );
    // Echo of request without data fields:
    EXPECT_THROW( parseDeviceInfo( "$PFMSNP,*3A\r\n" ), std::runtime_error );
    // Non-PFMSNP sentence:
    EXPECT_THROW( parseDeviceInfo( "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47" ),
                  std::runtime_error );
    // Invalid checksum:
    EXPECT_THROW( parseDeviceInfo( "$PFMSNP,Gps,,01229,2.03g, 953.36,,b203*00\r\n" ), std::runtime_error );
    // Missing serial number:
    EXPECT_THROW( parseDeviceInfo( "$PFMSNP,Gps*33" ), std::runtime_error );
}

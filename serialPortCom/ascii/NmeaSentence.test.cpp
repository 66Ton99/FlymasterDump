#include "NmeaSentence.h"
#include <gtest/gtest.h>

TEST( NmeaSentenceTest, shouldFailIfWrongSyntax ) {
    ASSERT_ANY_THROW( NmeaSentence( "PFMSNP,*3A" ) );
}

TEST( NmeaSentenceTest, shouldFailIfWrongChecksum ) {
    ASSERT_ANY_THROW( NmeaSentence( "$PFMSNP,*3B" ) );
}

TEST( NmeaSentenceTest, shouldFailIfTruncatedOrMissingChecksum ) {
    ASSERT_THROW( NmeaSentence( "$PFMSNP,Gps" ), std::runtime_error );
    ASSERT_THROW( NmeaSentence( "$PFMSNP" ), std::runtime_error );
    ASSERT_THROW( NmeaSentence( "$PFMSNP,*" ), std::runtime_error );
    ASSERT_THROW( NmeaSentence( "$*" ), std::runtime_error );
    ASSERT_THROW( NmeaSentence( "$" ), std::runtime_error );
    ASSERT_THROW( NmeaSentence( "" ), std::runtime_error );
    ASSERT_THROW( NmeaSentence( "$PFMSNP,*GG" ), std::runtime_error );
    ASSERT_THROW( NmeaSentence( "$PFMSNP*3A*3A" ), std::runtime_error );
}

TEST( NmeaSentenceTest, shouldParseWithTrailingCarriageReturn ) {
    ASSERT_EQ( "$PFMDNL,LST,*56", NmeaSentence( "$PFMDNL,LST,*56\r" ).build() );
    ASSERT_EQ( "$PFMDNL,LST,*56", NmeaSentence( "$PFMDNL,LST,*56\r\n" ).build() );
    ASSERT_EQ( "$PFMSNP,*3A", NmeaSentence( "$PFMSNP,*3A\r" ).build() );
}

TEST( NmeaSentenceTest, shouldParse ) {
    ASSERT_EQ( "$PFMSNP,*3A", NmeaSentence( "$PFMSNP,*3A" ).build() );
    ASSERT_EQ( "$PFMDNL,LST,*56", NmeaSentence( "$PFMDNL,LST,*56" ).build() );
    ASSERT_EQ( "$PFMSNP,GpsSD,,00857,1.06p,1019.87,*3E",
               NmeaSentence( "$PFMSNP,GpsSD,,00857,1.06p,1019.87,*3E" ).build() );
}

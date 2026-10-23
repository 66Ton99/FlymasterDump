#include "Callback.h"
#include <gtest/gtest.h>

TEST( CallbackTest, BasicInterface ) {
    Callback cb;
    EXPECT_FALSE( cb.isCancelled() );
    cb.notify( Severity::Info, "test notification" );
}

TEST( CallbackTest, PrintCallback ) {
    PrintCallback pcb;
    EXPECT_FALSE( pcb.isCancelled() );
    pcb.notify( Severity::Debug, "debug message" );
    pcb.notify( Severity::Info, "info message" );
    pcb.notify( Severity::Warning, "warning message" );
    pcb.notify( Severity::Error, "error message" );
    pcb.notify( Severity::Critical, "critical message" );
    pcb.notify( Severity::Progress, "progress message" );
}

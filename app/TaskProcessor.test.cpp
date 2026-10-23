#include "TaskProcessor.h"

#include <QCoreApplication>
#include <gtest/gtest.h>

namespace {

class WaitTask : public Task{
public:
    WaitTask( int ms ): Task("Wait"), ms_( std::chrono::milliseconds(ms) ){
    }
    void run(){
        ++executeCount_;
        std::this_thread::sleep_for( ms_ );
    }
    std::chrono::milliseconds ms() const
    {
        return ms_;
    }

    size_t executeCount() const
    {
        return executeCount_;
    }

private:
    std::chrono::milliseconds ms_;
    size_t executeCount_ = 0;
};

}

TEST( TaskProcessorTest, BasicTest ){
    TaskProcessor p;
    ASSERT_TRUE( p.isRunning() );
    ASSERT_FALSE( p.isWorking() );
    ASSERT_EQ( nullptr, p.currentTask() );

    p.stop();
    ASSERT_FALSE( p.isRunning() );

    auto task = std::make_shared<WaitTask>(10);
    p.schedule( task );
    ASSERT_TRUE( p.isScheduled( task ) );
    ASSERT_FALSE( p.isRunning(task) );
    ASSERT_EQ( nullptr, p.currentTask() );
    ASSERT_EQ( 1, p.scheduledTasks().size() );
    ASSERT_EQ( 1, p.queueSize() );

    p.start();
    std::this_thread::sleep_for( 0.5*task->ms() );
    ASSERT_EQ( task, p.currentTask() );
    std::this_thread::sleep_for( task->ms() );
    ASSERT_EQ( nullptr, p.currentTask() );
}

TEST( TaskProcessorTest, ClearTest ){
    TaskProcessor p;
    p.stop();
    auto task = std::make_shared<WaitTask>(10);
    p.schedule( task );
    p.schedule( task );
    p.schedule( task );
    p.schedule( task );
    p.schedule( task );
    p.schedule( task );

    p.start();
    std::this_thread::sleep_for( 0.1*task->ms() );
    ASSERT_EQ( task, p.currentTask() );
    p.clear();
    std::this_thread::sleep_for( 2*task->ms() );
    ASSERT_EQ( nullptr, p.currentTask() );
    ASSERT_EQ( 0, p.queueSize() );
    ASSERT_EQ( 1, task->executeCount() );
}

int main( int argc, char** argv ) {
    QCoreApplication app( argc, argv );
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}



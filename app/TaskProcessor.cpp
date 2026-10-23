#include "TaskProcessor.h"


Task::Task(QString name): name_( name ) {}

QtCallback &Task::callback() {
    return callback_;
}

QString Task::name() const
{
    return name_;
}

TaskProcessor::TaskProcessor() : goOn_( false ) {
    qRegisterMetaType< std::shared_ptr< Task > >();
    start();
}

void TaskProcessor::schedule( std::shared_ptr< Task > task ) {
    std::unique_lock< std::mutex > lock( mutex_ );
    tasks_.push_back( task );

    emit scheduledTask( task );

    // wakeup thread
    if( tasks_.size() == 1 ) {
        blocker_.notify_one();
    }
}

void TaskProcessor::remove( std::shared_ptr< Task > task ) {
    std::unique_lock< std::mutex > lock( mutex_ );
    size_t oldSize = tasks_.size();
    tasks_.remove( task );
    if( oldSize != tasks_.size() ) {
        emit removedTask( task );
    }
}

void TaskProcessor::stop() {
    if( !isRunning() )
        return;

    {
        //changing goOn_ has to be mutex-locked due to the documentation of std::condition_variable
        std::unique_lock< std::mutex > lock( mutex_ );
        goOn_ = false;
    }
    blocker_.notify_all();
    thread_->join();
}

void TaskProcessor::start() {
    std::unique_lock< std::mutex > lock( mutex_ );
    if( isRunning() )
        return;

    goOn_ = true;
    thread_.reset( new std::thread( [this]() { this->threadLoop(); } ) );
}

void TaskProcessor::clear() {
    std::unique_lock< std::mutex > lock( mutex_ );

    while( !tasks_.empty() ) {
        auto back = tasks_.back();
        tasks_.pop_back();
        emit removedTask( back );
    }
    if( currentTask_ != nullptr ) {
        currentTask_->callback().cancel();
    }
}

bool TaskProcessor::isRunning() const {
    return goOn_;
}

bool TaskProcessor::isWorking() const {
    return currentTask_ != nullptr;
}

size_t TaskProcessor::queueSize() const {
    return tasks_.size();
}

const std::list< std::shared_ptr< Task > >& TaskProcessor::scheduledTasks() const {
    return tasks_;
}

const std::shared_ptr< Task >& TaskProcessor::currentTask() const {
    return currentTask_;
}

bool TaskProcessor::isScheduled( std::shared_ptr< Task > task ) const {
    std::unique_lock< std::mutex > lock( mutex_ );
    auto it = std::find( tasks_.begin(), tasks_.end(), task );
    return it != tasks_.end();
}

bool TaskProcessor::isRunning( std::shared_ptr< Task > task ) const {
    std::unique_lock< std::mutex > lock( mutex_ );
    return currentTask_ == task;
}

TaskProcessor::~TaskProcessor() {
    stop();
}

void TaskProcessor::threadLoop() {
    while( goOn_ ) {
        {
            std::unique_lock< std::mutex > lock( mutex_ );
            if( !tasks_.empty() ) {
                currentTask_ = tasks_.front();
                tasks_.pop_front();
            }
        }

        if( currentTask_ != nullptr ) {
            emit startingTask( currentTask_ );
            try {
                currentTask_->run();
            } catch( std::exception& e ) {
                currentTask_->callback().notify( Severity::Critical, e.what() );
            } catch( ... ) {
                currentTask_->callback().notify( Severity::Critical, "Unknown Error Occured!" );
            }
            emit finishedTask( currentTask_ );
            std::unique_lock< std::mutex > lock( mutex_ );
            currentTask_ = nullptr;
        }

        if( goOn_ ) {
            std::unique_lock< std::mutex > lock( mutex_ );
            if( tasks_.empty() ) {
                blocker_.wait( lock );
            }
        }
    }
}


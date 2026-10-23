#include "CheckDeviceTask.h"

#include <serialPortCom/SerialPort.h>

CheckDeviceTask::CheckDeviceTask( Model model )
    : Task( QObject::tr( "Checking Flymaster device..." ) ),
      model_( model ) {
}

void CheckDeviceTask::run() {
    SerialPort port( model_.serialPort().toStdString() );
    port.setSpeed( model_.serialPortSpeed() );
    deviceInfo_ = requestDeviceInfo( port, callback() );
}

const DeviceInfo& CheckDeviceTask::deviceInfo() const {
    return deviceInfo_;
}

#pragma once

#include "Model.h"
#include "TaskProcessor.h"

#include <serialPortCom/Request.h>

class CheckDeviceTask : public Task {
public:
    CheckDeviceTask( Model model );
    void run() override;

    const DeviceInfo& deviceInfo() const;

private:
    Model model_;
    DeviceInfo deviceInfo_;
};

#pragma once

#include "esphome/core/component.h"

#include<string>

// #include <net/openthread.h>
#include <openthread/srp_client.h>

namespace esphome {
namespace openthread {

class OpenThreadSRP : public Component {
    public:
        void setup() override;
        float get_setup_priority() const override {
            return setup_priority::HARDWARE;
        }
        void set_host_name(std::string host_name);
    protected:
    std::string host_name;
};

}  // namespace openthread
}  // namespace esphome
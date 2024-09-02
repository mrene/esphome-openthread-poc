#pragma once

#include "esphome/core/component.h"

extern "C" {
    void openthread_init(void);
}

namespace esphome {
namespace openthread {

class OpenThreadComponent : public Component {
    public:
        void setup() override;
        float get_setup_priority() const override {
            return setup_priority::WIFI;
        }
        void srp_setup();
        void set_host_name(std::string host_name);
        void set_mac(std::string mac);
    protected:
        std::string host_name;
        std::string mac;
};

}  // namespace openthread
}  // namespace esphome
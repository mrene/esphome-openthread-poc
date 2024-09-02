#include "ot_srp.h"
#include "esphome/core/log.h"

#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "esp_task_wdt.h"

#include <freertos/portmacro.h>

#include <openthread/thread.h>
#include <openthread/srp_client_buffers.h>
#include <openthread/netdata.h>

#include <cstring>

namespace esphome {
namespace openthread {

void OpenThreadSRP::initialize() {
    uint16_t len = host_name.size();
    otError error;
    int aborted = 1;
    uint16_t size;
    char *   existing_host_name;
    uint8_t       arrayLength;
    otIp6Address *hostAddressArray;
    std::string service = "_esphomelib._tcp";
    otSrpClientBuffersServiceEntry *entry = nullptr;
    char *                          string;
    char *                          label;
    const otIp6Address * localIp = nullptr;
    const otIp6Prefix * omrPrefix = nullptr;
    otBorderRouterConfig aConfig;
    otNetworkDataIterator iterator = OT_NETWORK_DATA_ITERATOR_INIT;
    const otNetifAddress *unicastAddrs = nullptr;
    bool locked = false;

    uint8_t ip_found = 0;
    char addressAsString[40];
    otInstance *instance = nullptr;

    ESP_LOGI("openthread", "Setting up SRP...");

    // Need to wait for the openthread component to have
    // completed its initialization
    while (!esp_openthread_lock_acquire(10)) {
        esp_task_wdt_reset();
    }
    locked = true;

    instance = esp_openthread_get_instance();

    // set the host name
    existing_host_name = otSrpClientBuffersGetHostNameString(instance, &size);

    if (len > size) { 
        ESP_LOGW("OT SRP", "Hostname is too long, choose a shorter project name");
        goto exit;
    }

    memcpy(existing_host_name, host_name.c_str(), len + 1);
    error = otSrpClientSetHostName(instance, existing_host_name);
    if (error != 0){
        ESP_LOGW("OT SRP", "Could not set host name with srp server");
        goto exit;
    }

    // set the ip address
    // get the link local ip address
    //localIp = otThreadGetLinkLocalIp6Address(context->instance);
    //localIp = otThreadGetMeshLocalEid(context->instance);
    while (otNetDataGetNextOnMeshPrefix(instance, &iterator, &aConfig) != OT_ERROR_NONE) {
        esp_openthread_lock_release();
        locked = false;
        vTaskDelay(100);
        if (!esp_openthread_lock_acquire(portMAX_DELAY)) {
            ESP_LOGW("OT SRP", "Could not acquire lock");
            goto exit;
        }
        locked = true;
    };
    if (error != 0){
        ESP_LOGW("OT SRP", "Could not get the OMR prefix");
        goto exit;
    }
    omrPrefix = &aConfig.mPrefix;
    otIp6PrefixToString(omrPrefix, addressAsString, 40);
    ESP_LOGW("OT SRP", "USING omr prefix %s", addressAsString);
    unicastAddrs = otIp6GetUnicastAddresses(instance);
    for (const otNetifAddress *addr = unicastAddrs; addr; addr = addr->mNext){
        localIp = &addr->mAddress;
        if (otIp6PrefixMatch(&omrPrefix->mPrefix, localIp)) {
            ip_found = 1;
            otIp6AddressToString(localIp, addressAsString, 40);
            ESP_LOGW("OT SRP", "USING %s for SRP address", addressAsString);
            break;
        }
    }
    if (ip_found == 0) {
        ESP_LOGW("OT SRP", "Could not find the OMR address");
        goto exit;
    }

    hostAddressArray = otSrpClientBuffersGetHostAddressesArray(instance, &arrayLength);

    memcpy(hostAddressArray, localIp, sizeof(*localIp));
    error = otSrpClientSetHostAddresses(instance, hostAddressArray, arrayLength);
    if (error != 0){
        ESP_LOGW("OT SRP", "Could not set ip address with srp server");
        goto exit;
    }

    // set the records
    entry = otSrpClientBuffersAllocateService(instance);
    entry->mService.mPort = 6053;

    string = otSrpClientBuffersGetServiceEntryInstanceNameString(entry, &size);
    memcpy(string, host_name.c_str(), strlen(host_name.c_str()));

    string = otSrpClientBuffersGetServiceEntryServiceNameString(entry, &size);
    memcpy(string, service.c_str(), strlen(service.c_str()));

    label = strchr(string, ',');
    entry->mService.mNumTxtEntries = 0;
    error = otSrpClientAddService(instance, &entry->mService);
    if (error != 0){
        ESP_LOGW("OT SRP", "Could not set service to advertise.");
        goto exit;
    }

    otSrpClientEnableAutoStartMode(instance, nullptr, nullptr);
    // unlock the mutex
    aborted = 0;
exit:
    if (aborted) {
        this->mark_failed();
        ESP_LOGW("OT SRP", "Setting SRP record failed, clear partial state");
        if (instance != nullptr) {
            otSrpClientClearHostAndServices(instance);
            otSrpClientBuffersFreeAllServices(instance);
        }
    }
    if (locked) {
       esp_openthread_lock_release();
    }
}

void OpenThreadSRP::set_host_name(std::string host_name){
    this->host_name = host_name;
}

}  // namespace openthread
}  // namespace esphome
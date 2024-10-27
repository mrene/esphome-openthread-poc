## ESPHome ESP32 OpenThread POC

Update:
[An official component has been submitted as a PR to esphome](https://github.com/esphome/esphome/pull/7506)

Here is example configuration for an ESP32-C6:

```yaml
esphome:
  name: openthread

esp32:
  board: esp32-c6-devkitm-1
  variant: esp32c6
  framework:
    type: esp-idf
    version: "5.3.1"
    platform_version: 6.8.1

network:
  enable_ipv6: true

openthread:
  channel: 13
  network_name: OpenThread-8f28
  network_key: dfd34f0f05cad978ec4e32b0413038ff
  panid: 0x8f28
  extpanid: d63e8e3e495ebbc3
  pskc: c23a76e98f1a6483639b1ac1271e2e27

# https://github.com/esphome/esphome/pull/7506
external_components:
  - source: github://pr#7506
    components: [ network, mdns, openthread ]
    refresh: 0s
```


## Old README
This is a proof of concept cobbling a few pieces together into a working openthread implementation for esphome:

- An implementation in [natelust/esphomeZephyr](https://github.com/natelust/esphomeZephyr/tree/zephyr/esphome/components/openthread) (mainly for the SRP component)
- The esp-idf example for [ot_cli](https://github.com/espressif/esp-idf/tree/master/examples/openthread/ot_cli)

Caveats:
- It was almost working with only external components, except that an `is_connected` method would return false and prevent any API connections from working. This was patched in order to return true for test purposes (see esphome-hack-is-connected.diff).
- A new task is created for the SRP client, a proper implementation would likely be to turn this into a state machine to avoid blocking for the openthread lock to be available.
- The SRP TXT records should be sourced from the mdns module, but they are hardcoded to only include the `mac=` record so it can be detected by HA.

Bugs:
- If the web server is active, the home assistant integration will sporadically disconnect, an error is seen HA's console where it's trying to parse the ipv6 address following the first : as a port number. (`ValueError: Port could not be cast to integer value as '4cc8:e352:1:3421:d788:5046:bc2d:80'`). Disabling the web server resolves this problem.

Note: This somehow makes the CLI available via the USB Serial JTAG interface, esphome won't pass any input but you can interact with it with picocom/mincom/idf.py.

## Tested MCUs
- [x] [ESP32C6](./test-ot.yaml)
- [x] [ESP32H2](./test-h2.yaml)


## Usage with nix
If you have nix installed, you can use the provided devshell to directly used the patched esphome.


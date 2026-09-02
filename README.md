# PlantMate Testing Firmware

Factory test firmware for the [PlantMate](https://github.com/sadrirammal/PlantMate-Open-Source) automatic plant watering device. This is **not** the production firmware. It is the first thing flashed onto a freshly assembled unit to check the build, after which the device fetches the real firmware over the air.

## What it does

1. **Connects to WiFi** using the credentials you put in `loop()` in `src/main.cpp`.
2. **Power-cycles each pump in turn** (5 pumps, 3 seconds each). Watching this during assembly immediately shows wrongly wired or dead pumps, swapped connectors, and bad solder joints.
3. **Pulls the latest production firmware** from `https://api.plantup.io/device/growmate/firmware` and flashes it via OTA, then reboots into it.

If WiFi is not available, the pump cycle still runs and the OTA step is skipped. The loop then repeats.

## Hardware

- ESP32-WROOM-32E on the PlantMate PCB V2.1 (see the open-source repo for schematic, gerbers and BOM)
- Pump MOSFET gates on GPIO 4, 16, 17, 18, 19 (pump 1 to 5, the fifth is the tank intake pump)
- Status LEDs on GPIO 2, 15, 13 (white, blue, red), active low

## Flashing

1. Install [PlatformIO](https://platformio.org/) (VS Code extension or CLI).
2. Open `src/main.cpp` and replace the placeholder WiFi credentials in `loop()`:
   ```cpp
   setupWiFi("wifi name", "passwort");
   ```
3. Connect the PCB over USB-C. Hold **BOOT** and tap **RST** if the board does not enter the bootloader on its own.
4. Build and upload:
   ```
   pio run -t upload
   pio device monitor
   ```
5. Watch the serial output at 115200 baud. You should see each pump run for 3 seconds, then either an OTA update or a message that WiFi was unavailable.

## Note on the OTA server

The OTA step downloads from the PlantUp server. If that server is no longer online the update will fail with an HTTP error and the test loop simply keeps cycling the pumps, which is still useful for assembly checks. For a self-built unit you will need your own firmware to flash afterwards.

## License

Released as part of the PlantMate open-source project. See the main repo for details.

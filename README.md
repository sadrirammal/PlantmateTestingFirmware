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

## Setting up PlatformIO

This project is built with [PlatformIO](https://platformio.org/), which handles the ESP32 toolchain, libraries and upload for you. Nothing else needs to be installed by hand.

- Install [Visual Studio Code](https://code.visualstudio.com/), then add the **PlatformIO IDE** extension from the Extensions panel. The first start downloads the ESP32 toolchain, which takes a few minutes.
- In VS Code use **File > Open Folder** on this repo. PlatformIO reads `platformio.ini` and pulls the dependencies automatically.
- The PlatformIO toolbar at the bottom of VS Code has **Build** (checkmark), **Upload** (arrow) and **Serial Monitor** (plug) buttons. Those are the only three you need.
- On Windows you may need the CH340 USB driver for the board to show up as a COM port. PlatformIO usually picks the port itself; if not, add `upload_port = COM5` (or whatever yours is) to `platformio.ini`.

If you have never used PlatformIO before, this video walks through install, first build and upload to an ESP32 in about 15 minutes:
[Perfect Combo for ESP32: VS Code & PlatformIO Guide](https://www.youtube.com/watch?v=WxELHnnlBmU). The official quick-start is at [docs.platformio.org](https://docs.platformio.org/en/latest/integration/ide/vscode.html#quick-start).

## Flashing

1. Set up PlatformIO as described above and open this folder in VS Code.
2. Open `src/main.cpp` and replace the placeholder WiFi credentials in `loop()`:
   ```cpp
   setupWiFi("wifi name", "passwort");
   ```
3. Connect the PCB over USB-C. Hold **BOOT** and tap **RST** if the board does not enter the bootloader on its own.
4. Click **Upload** in the PlatformIO toolbar, then **Serial Monitor**. From the command line the equivalent is:
   ```
   pio run -t upload
   pio device monitor
   ```
5. Watch the serial output at 115200 baud. You should see each pump run for 3 seconds, then either an OTA update or a message that WiFi was unavailable.

## Note on the OTA server

The OTA step downloads from the PlantUp server. If that server is no longer online the update will fail with an HTTP error and the test loop simply keeps cycling the pumps, which is still useful for assembly checks. For a self-built unit you will need your own firmware to flash afterwards.

## License

Released as part of the PlantMate open-source project. See the main repo for details.

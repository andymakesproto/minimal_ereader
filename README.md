# Minimal E-Reader

Minimal Reader is firmware for an ESP32-S3 e-reader with a 3.7-inch monochrome e-paper display. Browse and read plain-text books, save reading progress, and transfer books over Wi-Fi.

If you find Minimal E-reader useful, you can [support its development on Ko-fi](https://ko-fi.com/andymakesproto). Thank you!

## Features

- Read `.txt` books stored in the device's LittleFS flash filesystem.
- Resume from saved page position, navigate backward through page history, and track reading time and progress.
- Automatically save progress after a configurable number of page turns and when leaving a book or preparing for sleep.
- Upload and delete books through a browser-based Wi-Fi transfer page.
- Configure the inactivity sleep timeout and reading autosave interval.
- Enter light sleep while reading and deep sleep after the configured inactivity timeout.

## Roadmap

- Battery monitoring
- Screen orientation options
- Adjustable font size
- Browser-based firmware installer

## Hardware

The firmware targets the **Seeed XIAO ESP32S3** and the **GxEPD2_370_GDEY037T03** black-and-white e-paper panel (280 × 416 pixels). To build a fully functional device, you will also need a Li-po battery, 4 push buttons, a slide switch, a perboard, and 30 AWG silicone wires.

Recommended list of components:
- [XIAO ESP32-S3 by Seeed Studio](https://s.click.aliexpress.com/e/_c2IvrVyJ) 
- [We Act Studio 3.7" E-paper Module Black & White](https://www.aliexpress.us/item/3256809525686527.html?channel=twinner)
- [503048 800mAh Li-po Battery](https://s.click.aliexpress.com/e/_c4SsOrQF)
- [6x6x4.3 Push Buttons](https://s.click.aliexpress.com/e/_c3AJGSvd)
- [SS12F44 Slide Switch](https://s.click.aliexpress.com/e/_c2Ihgcin)
- [5x7 Perfboard](https://s.click.aliexpress.com/e/_c2Ihgcin)
- [30 AWG Silicone Wires](https://s.click.aliexpress.com/e/_c4Wn6jPz)

### Full build tutorial

For step-by-step instructions to build the device, please see my full tutorial on Ko-fi.

## How to use the firmware

To install the firmware, you will use Visual Studio Code and the PlatformIO extension. You do not need to edit any code.

1. Install [Visual Studio Code](https://code.visualstudio.com/).
2. In Visual Studio Code, open **Extensions** from the left-hand activity bar, search for **PlatformIO IDE**, and install the extension published by PlatformIO.
3. Download this project using the green **Code** button on GitHub and select **Download ZIP**. Extract the ZIP file to a location you can find again.
4. In Visual Studio Code, choose **File → Open Folder...** and select the extracted project folder that contains `platformio.ini`. Choose **Trust** if Visual Studio Code asks whether you trust the folder.
5. Wait for PlatformIO to finish its initial setup and install the project libraries. This can take a few minutes and requires an internet connection.
6. Connect the XIAO ESP32S3 to your computer using a USB data cable. Charge-only USB cables will not work.
7. In the PlatformIO icon in the left-hand activity bar, open **Project Tasks**, expand **seeed_xiao_esp32s3**, then **General**, and select **Upload**. PlatformIO will build the firmware and copy it to the connected board.
8. When the upload finishes, the reader will restart. If PlatformIO asks you to choose a serial port, select the port for the XIAO ESP32S3.

If the board is not detected, try another USB cable or port. The first setup and build require an internet connection. The project uses the Arduino framework and its libraries are listed in [`platformio.ini`](platformio.ini).

## Add books

### Recommended: use Wi-Fi Transfer

1. Choose **Wi-Fi Transfer** from the library.
2. Connect a phone or computer to the access point shown on the e-paper display.
3. Open `http://192.168.4.1` in a browser to upload or delete `.txt` files.
4. Choose **Back** on the device to stop the server and turn Wi-Fi off.

The default network name is `MinimalReader` and its initial password is `12345678`. Change this password from the transfer page before using the access point in a shared environment. The password can also be reset to the default from the Settings menu.

### Alternative: upload sample books with PlatformIO

The repository includes sample `.txt` books in [`data/`](data/). To copy them to the device, open **PlatformIO → Project Tasks → seeed_xiao_esp32s3 → Platform → Upload Filesystem Image** in Visual Studio Code.

**Warning:** Uploading a filesystem image replaces the existing filesystem contents, including books already on the device.

## Use the reader

The library opens on startup. The four buttons have these primary actions:

| Button | Library | Reader |
| --- | --- | --- |
| 1 (D0) | Open sleep screen | Return to library |
| 2 (D1) | Move selection up | Previous page |
| 3 (D2) | Move selection down | Next page |
| 4 (D3) | Select book or menu item | Full-refresh current page |

In the reader, long-press button 4 to open reading statistics and the progress-reset option. The library also contains **Settings** and **Wi-Fi Transfer** entries. The sleep screen can return to the library with any button; after inactivity, the device shows a warning and then enters deep sleep. Button 4 wakes it from deep sleep.

## Project layout

| Path | Purpose |
| --- | --- |
| `src/main.cpp` | Hardware initialization, app setup, and main loop |
| `src/Apps/` | Library, reader, settings, sleep, and transfer screens |
| `lib/Core/` | Shared app interface and screen / inactivity manager |
| `lib/HAL/` | E-paper display, button input, LittleFS settings, and book reading |
| `include/config.h` | Hardware pins, display settings, and default behavior |
| `data/` | Sample text files copied to LittleFS with `uploadfs` |
| `partitions.csv` | ESP32 flash partition layout |
| `test/` | PlatformIO test directory; no project test cases are currently defined |

## Dependencies

PlatformIO manages the project libraries declared in `platformio.ini`:

- [OneButton](https://github.com/mathertel/OneButton)
- [GxEPD2](https://github.com/ZinggJM/GxEPD2)
- Adafruit GFX and Adafruit BusIO, used by GxEPD2

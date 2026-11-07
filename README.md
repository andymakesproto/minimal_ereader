# Minimal E-Reader

Minimal E-Reader is firmware for an ESP32-S3 e-reader with a 3.7-inch monochrome e-paper display. It lets you read plain-text books, save your reading progress, and transfer books over Wi-Fi.

If you find Minimal E-Reader useful, you can [support its development on Ko-fi](https://ko-fi.com/andymakesproto). Thank you!

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/K2R3288SQP)

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

The firmware is designed for the **Seeed XIAO ESP32S3** and the **GxEPD2_370_GDEY037T03** black-and-white e-paper panel (280 × 416 pixels). To build the complete device, you will also need a LiPo battery, four push buttons, a slide switch, a perfboard, and 30 AWG silicone wires.

Recommended components:
- [XIAO ESP32-S3 by Seeed Studio](https://s.click.aliexpress.com/e/_c2IvrVyJ)
- [We Act Studio 3.7" E-paper Module Black & White](https://www.aliexpress.us/item/3256809525686527.html?channel=twinner)
- [503048 800mAh LiPo Battery](https://s.click.aliexpress.com/e/_c4SsOrQF)
- [6x6x4.3 Push Buttons](https://s.click.aliexpress.com/e/_c3AJGSvd)
- [SS12F44 Slide Switch](https://s.click.aliexpress.com/e/_c2Ihgcin)
- [5x7 Perfboard](https://s.click.aliexpress.com/e/_c2Ihgcin)
- [30 AWG Silicone Wires](https://s.click.aliexpress.com/e/_c4Wn6jPz)

### Full build tutorial

For step-by-step instructions on building the device, see my tutorial on Ko-fi.

## Install the firmware

Follow these steps to install the firmware using Visual Studio Code and the PlatformIO extension. You do not need to edit any code.

1. Download and install [Visual Studio Code](https://code.visualstudio.com/).
2. In Visual Studio Code, open **Extensions** from the left-hand sidebar. Search for **PlatformIO IDE** and install the extension published by PlatformIO.
3. On GitHub, click the green **Code** button and select **Download ZIP**. Extract the ZIP file somewhere you can find it again.
4. In Visual Studio Code, select **File → Open Folder...** and open the extracted project folder containing `platformio.ini`. If Visual Studio Code asks whether you trust the folder, select **Trust**.
5. Wait for PlatformIO to finish setting itself up and install the project libraries. This may take a few minutes and requires an internet connection.
6. Connect the XIAO ESP32S3 to your computer with a USB data cable. A charge-only cable will not work.
7. In the PlatformIO sidebar, open **Project Tasks**, expand **seeed_xiao_esp32s3**, then **General**, and select **Upload**. PlatformIO will build the firmware and upload it to the connected board.
8. When the upload finishes, the reader will restart. If PlatformIO asks you to select a serial port, choose the one for the XIAO ESP32S3.

If the board is not detected, try a different USB cable or port. The initial setup and firmware build require an internet connection. The project uses the Arduino framework, and its libraries are listed in [`platformio.ini`](platformio.ini).

## How to add books

### Recommended: use Wi-Fi Transfer

1. Choose **Wi-Fi Transfer** from the library menu.
2. Connect your phone or computer to the Wi-Fi access point shown on the e-paper display.
3. Open `http://192.168.4.1` in a browser to upload or delete `.txt` files. If your phone switches away from the reader's access point, temporarily turn off mobile data.
4. Choose **Back** on the device to stop the server and turn Wi-Fi off.

The default network name is `MinimalReader`, and the default password is `12345678`. You can change the password on the same browser page.

### Alternative: upload sample books with PlatformIO

The repository includes sample `.txt` books in [`data/`](data/). To copy them to the device, open **PlatformIO → Project Tasks → seeed_xiao_esp32s3 → Platform → Upload Filesystem Image** in Visual Studio Code.

**Warning:** Uploading a filesystem image replaces the files on the device, including any books and reading progress stored there.

## Using the reader

The library opens on startup. The four buttons have these primary actions:

| Button | Library | Reader |
| --- | --- | --- |
| 1 (D0) | Open sleep screen | Return to library |
| 2 (D1) | Move selection up | Previous page |
| 3 (D2) | Move selection down | Next page |
| 4 (D3) | Select book or menu item | Full-refresh current page |

In the reader, press and hold button 4 to open the reading statistics and progress-reset options. The library also contains **Settings** and **Wi-Fi Transfer** entries. After a period of inactivity, the device displays a warning and then enters deep sleep. To wake it from deep sleep, press and hold button 4 for about one second.

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

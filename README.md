# ESP32-3248S035C Touch MIDI Controller

Touchscreen controller that sends MIDI Control Change (CC) over Bluetooth Serial.  
Target board: **ESP32-3248S035C** (3.2" 320×480 TFT, GT911 capacitive touch).

---

## What you need in Arduino IDE

### 1. ESP32 board support (required)

The sketch uses **BluetoothSerial**, which is part of the ESP32 core. You must install the ESP32 boards in the IDE:

1. **File → Preferences**
2. In **Additional boards manager URLs** add:
   ```text
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
   (If you already have other URLs, add this one on a new line.)
3. **Tools → Board → Boards Manager**
4. Search for **“esp32”**
5. Install **“esp32” by Espressif Systems**

### 2. No extra libraries

The sketch does **not** use the MIDI library. It sends raw MIDI bytes over Bluetooth Serial, so you don’t need to install **MIDI** (or any other library). Everything else is from the ESP32 core (`SPI`, `Wire`, `BluetoothSerial`).

---

## Board selection and upload

1. **Tools → Board → ESP32 Arduino**  
   Choose a board that matches your module, for example:
   - **ESP32 Dev Module**, or  
   - **ESP32S3 Dev Module** (only if your board is actually an S3)
2. Set **Upload speed** and **Port** after plugging in the USB cable.  
   If upload fails with “chip stopped responding”, set **Upload speed** to **115200** (Tools → Upload Speed).
3. Compile and upload as usual.

If your board appears in the list with a name like “ESP32 3248S035” or “ESP32-3248S035C”, select that. Otherwise **ESP32 Dev Module** is usually correct for the standard ESP32-WROOM on this display board.

---

## Upload problems?

- **“Chip stopped responding” / upload fails at high speed**  
  **Tools → Upload Speed → 115200**, then upload again. Try **460800** if 115200 works but is slow.

- **Board doesn’t enter bootloader**  
  Hold **BOOT** (may be labeled IO0), press and release **EN**/RST, release BOOT, then start upload.

- Use a **data-capable USB cable** and avoid charge-only cables. If the display is on, try a powered USB port or hub.

---

## How it works

- **Display:** Kept off/black (SPI TFT init only to avoid garbage on screen).
- **Touch:** GT911 over I2C; X/Y and touch on/off are sent as MIDI CC.
- **Bluetooth:** Classic Bluetooth Serial (SPP), name **“ESP32-Touch-Guitar”**.
- **MIDI:** Raw CC bytes on channel 1:
  - CC 22 = X (0–127)
  - CC 23 = Y (0–127)
  - CC 24 = touch state (127 = touch, 0 = release)

To use it as MIDI on a computer you need a bridge that reads the Bluetooth Serial connection and forwards to a MIDI port (e.g. “Hairless MIDI Serial Bridge” or similar, configured for the Bluetooth serial port).

---

## Pin note (ESP32-3248S035C)

GPIO33 is used for display reset during `setup()`, then for I2C SDA (touch). That sequential use is correct for this board; no change needed.

# ESP32-3248S035C Touch MIDI Controller

Touchscreen controller that sends MIDI Control Change (CC) over **BLE MIDI**.  
Target board: **ESP32-3248S035C** (3.2" 320×480 TFT, GT911 capacitive touch).  
Shows up in **macOS Audio MIDI Setup** — add device, add session, no extra apps.

---

## What you need in Arduino IDE

### 1. ESP32 board support (required)

1. **File → Preferences**
2. In **Additional boards manager URLs** add:
   ```text
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
   (If you already have other URLs, add this one on a new line.)
3. **Tools → Board → Boards Manager**
4. Search for **“esp32”**
5. Install **“esp32” by Espressif Systems**

### 2. Libraries (Sketch → Include Library → Manage Libraries)

- **BLE-MIDI** (lathoub) — install this first; it will prompt for the **MIDI** library (FortySevenEffects), install that too.  
  The sketch uses the built-in ESP32 BLE backend (not NimBLE), so you do **not** need NimBLE-Arduino.

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
- **Bluetooth:** BLE MIDI, device name **“ESP32-Touch-Guitar”**.
- **MIDI:** CC on channel 10:
  - CC 22 = X (0–127)
  - CC 23 = Y (0–127)
  - CC 24 = touch state (127 = touch, 0 = release)

To use it as MIDI on a computer add the device in **Audio MIDI Setup** and create a session (see below).

---

## macOS: Audio MIDI Setup

1. Upload the sketch, then power the ESP32 (USB or external).
2. Open **Audio MIDI Setup** (Spotlight → "Audio MIDI Setup", or **Applications → Utilities**).
3. **Window → Show MIDI Studio**.
4. Click **"+"** at bottom-left (or **File → New External Device**).
5. Add the **Bluetooth MIDI** device: your Mac should see **"ESP32-Touch-Guitar"**. Select it and add it.
6. **Double-click the new device** (or select it and use the session/connection options) and **add a session** so the virtual MIDI port is created and available to your DAW or other apps.
7. In your DAW or app, choose the ESP32's MIDI port as an input. Touch the screen to send CC.

---

## Pin note (ESP32-3248S035C)

GPIO33 is used for display reset during `setup()`, then for I2C SDA (touch). That sequential use is correct for this board; no change needed.

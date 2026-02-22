// ESP32 Buttons + Knobs MIDI controller
// Same BLE MIDI stack as touch_screen — shows in macOS Audio MIDI Setup
// 6 buttons (5 with 3 modes, 1 = mode selector) + 5 knobs, all on channel 10
#include <Arduino.h>
#include <MIDI.h>
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32.h>

BLEMIDI_CREATE_INSTANCE("ESP32-Buttons-Knobs", MIDI)

#define MIDI_CHANNEL 10

// ——— Button pins (active LOW: press = LOW) ———
#define NUM_BUTTONS 6
const uint8_t BUTTON_PINS[NUM_BUTTONS] = { 4, 16, 17, 18, 19, 21 };

// ——— Knob pins (ADC1, 0–4095) ———
#define NUM_KNOBS 5
const uint8_t KNOB_PINS[NUM_KNOBS] = { 32, 33, 34, 35, 36 };

// ——— Modes for buttons 1–5 (button 6 cycles mode) ———
enum Mode { MODE_NOTES, MODE_MOMENTARY, MODE_SWITCH };
#define NUM_MODES 3

// ——— Button 1–5: note numbers (notes mode) ———
const uint8_t BUTTON_NOTE[NUM_BUTTONS - 1] = { 60, 61, 62, 63, 64 };

// ——— Button 1–5: CC numbers per mode (momentary = one CC, switch = another) ———
const uint8_t BUTTON_CC_MOMENTARY[NUM_BUTTONS - 1] = { 40, 41, 42, 43, 44 };
const uint8_t BUTTON_CC_SWITCH[NUM_BUTTONS - 1]    = { 50, 51, 52, 53, 54 };

// ——— Knob CC numbers (25–29: avoid 22,23,24 used by touch_screen) ———
const uint8_t KNOB_CC[NUM_KNOBS] = { 25, 26, 27, 28, 29 };

// ——— Timing / filtering ———
#define DEBOUNCE_MS    25
#define KNOB_THRESHOLD 2   // send CC only if value changed by more than this

bool isConnected = false;
void OnBLEConnected()    { isConnected = true; }
void OnBLEDisconnected() { isConnected = false; }

Mode currentMode = MODE_NOTES;
bool buttonState[NUM_BUTTONS];       // current debounced state (true = pressed)
bool lastButtonState[NUM_BUTTONS];
unsigned long lastDebounce[NUM_BUTTONS];
bool switchState[NUM_BUTTONS - 1];   // for MODE_SWITCH: on/off per button

uint8_t lastKnobValue[NUM_KNOBS];

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("ESP32 Buttons + Knobs MIDI");

  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    buttonState[i] = false;
    lastButtonState[i] = false;
    lastDebounce[i] = 0;
  }
  for (int i = 0; i < NUM_BUTTONS - 1; i++)
    switchState[i] = false;

  for (int i = 0; i < NUM_KNOBS; i++) {
    lastKnobValue[i] = 255; // force first read to send
  }

  BLEMIDI.setHandleConnected(OnBLEConnected);
  BLEMIDI.setHandleDisconnected(OnBLEDisconnected);
  MIDI.begin();
  Serial.println("BLE MIDI ready. Channel 10. Button 6 = mode.");
}

void loop() {
  MIDI.read();

  unsigned long now = millis();

  // ——— Buttons ———
  for (int i = 0; i < NUM_BUTTONS; i++) {
    bool raw = (digitalRead(BUTTON_PINS[i]) == LOW);
    if (raw != lastButtonState[i])
      lastDebounce[i] = now;
    lastButtonState[i] = raw;

    if ((now - lastDebounce[i]) > (unsigned long)DEBOUNCE_MS)
      buttonState[i] = raw;
  }

  // Button 6: mode cycle (on rising edge of press)
  static bool prevB6 = false;
  bool b6 = buttonState[5];
  if (b6 && !prevB6) {
    currentMode = (Mode)(((int)currentMode + 1) % NUM_MODES);
    if (isConnected) {
      // optional: send a mode CC so host knows (e.g. CC 19 = mode 0/1/2)
      MIDI.sendControlChange(19, (uint8_t)currentMode, MIDI_CHANNEL);
    }
    const char* names[] = { "Notes", "Momentary", "Switch" };
    Serial.printf("Mode: %s\n", names[(int)currentMode]);
  }
  prevB6 = b6;

  // Buttons 1–5: handle by mode
  for (int i = 0; i < NUM_BUTTONS - 1; i++) {
    bool pressed = buttonState[i];
    static bool wasPressed[NUM_BUTTONS - 1];
    bool justPressed  = pressed && !wasPressed[i];
    bool justReleased = !pressed && wasPressed[i];
    wasPressed[i] = pressed;

    if (!isConnected) continue;

    switch (currentMode) {
      case MODE_NOTES:
        if (justPressed)
          MIDI.sendNoteOn(BUTTON_NOTE[i], 127, MIDI_CHANNEL);
        if (justReleased)
          MIDI.sendNoteOff(BUTTON_NOTE[i], 0, MIDI_CHANNEL);
        break;

      case MODE_MOMENTARY:
        if (justPressed)
          MIDI.sendControlChange(BUTTON_CC_MOMENTARY[i], 127, MIDI_CHANNEL);
        if (justReleased)
          MIDI.sendControlChange(BUTTON_CC_MOMENTARY[i], 0, MIDI_CHANNEL);
        break;

      case MODE_SWITCH:
        if (justPressed) {
          switchState[i] = !switchState[i];
          uint8_t val = switchState[i] ? 127 : 0;
          MIDI.sendControlChange(BUTTON_CC_SWITCH[i], val, MIDI_CHANNEL);
        }
        break;
    }
  }

  // ——— Knobs ———
  for (int i = 0; i < NUM_KNOBS; i++) {
    int raw = analogRead(KNOB_PINS[i]);
    int val = map(raw, 0, 4095, 0, 127);
    if (val > 127) val = 127;
    uint8_t u = (uint8_t)val;
    if (lastKnobValue[i] == 255 || (int)abs((int)u - (int)lastKnobValue[i]) > KNOB_THRESHOLD) {
      lastKnobValue[i] = u;
      if (isConnected)
        MIDI.sendControlChange(KNOB_CC[i], u, MIDI_CHANNEL);
    }
  }

  delay(10);
}

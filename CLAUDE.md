 # M5Stack M5Paper Arduino Project

## Device Overview

Target hardware is the **M5Stack M5Paper** (ESP32-D0WDQ6-V3).

- **Display:** 4.7" e-ink, 960x540 resolution, 16-level grayscale, 235 ppi
- **Touch:** GT911 capacitive touchscreen, two-point touch, gesture support
- **MCU:** ESP32 dual-core, 240 MHz, 16 MB flash, 8 MB PSRAM
- **Sensors:** SHT30 temperature and humidity sensor (I2C, SDA=21, SCL=22)
- **RTC:** BM8563 real-time clock (used for timed wakeup from shutdown)
- **EEPROM:** FM24C02, 256 bytes, persistent across power cycles
- **Battery:** 1150 mAh LiPo
- **Storage:** TF card (microSD) slot
- **Expansion:** 3x HY2.0-4P Grove ports
- **Connectivity:** Wi-Fi and Bluetooth (ESP32 built-in)
- **USB:** USB-C for programming and charging (CP210x or CH9102 driver required)

## Arduino IDE Setup

**Board:** M5Paper (installed via M5Stack board manager)
Board manager URL: `https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json`

**Required libraries (install via Library Manager):**
- `M5Unified` >= 0.2.5
- `M5GFX` >= 0.2.7
- `M5Unit-ENV` >= 1.2.1 (for SHT30 sensor)

**Do NOT use the old `M5EPD` library.** It is no longer maintained. All new code must use `M5Unified` and `M5GFX`.

## Standard Includes and Initialization

```cpp
#include <M5Unified.h>
#include <M5GFX.h>

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);

    M5.Display.setRotation(1);           // 0=portrait, 1=landscape
    M5.Display.setEpdMode(epd_quality);  // see EPD modes below
    M5.Display.clear();
}

void loop() {
    M5.update();  // always call at the top of loop(); reads buttons and touch
}
```

For the SHT30 sensor:
```cpp
#include <M5UnitENV.h>
SHT3X sht3x;
// In setup():
sht3x.begin(&Wire, SHT3X_I2C_ADDR, 21, 22, 400000U);
// In loop():
sht3x.update();
float temp = sht3x.cTemp;
float humi = sht3x.humidity;
```

## E-Ink Display: Critical Rules

E-ink is NOT an LCD. Code must respect these constraints at all times.

**EPD modes** (set with `M5.Display.setEpdMode(...)`):
- `epd_quality` — full 16-level grayscale, slow refresh, use for final renders
- `epd_text` — optimized for black and white text
- `epd_fast` — faster refresh, reduced quality, good for interactive UI
- `epd_fastest` — fastest refresh, lowest quality, use for animations or touch feedback

**Batching draws:** Always wrap multiple draw calls in `startWrite()` / `endWrite()` to push them as a single update. This reduces flicker and improves speed.

```cpp
M5.Display.startWrite();
M5.Display.fillRect(0, 0, 960, 540, TFT_WHITE);
M5.Display.drawString("Hello", 100, 100);
M5.Display.endWrite();
```

**CRITICAL:** Do not access the SD card between `startWrite()` and `endWrite()`. The SPI bus is held exclusively during this window. SD access inside this block will crash or freeze the device.

**Minimize full refreshes.** Each full-screen refresh causes a visible flash. Prefer partial updates using sprites/canvases pushed to specific screen regions.

**Grayscale colors:** Use values 0 (black) to 15 (white) for grayscale. For compatibility, standard TFT color constants (TFT_BLACK, TFT_WHITE, TFT_DARKGREY, etc.) work and are mapped automatically by M5GFX.

**No backlight.** The e-ink display is always readable. Do not attempt to control brightness.

## Power Management

The M5Paper power architecture is different from other M5Stack devices. Read this carefully.

**Shutdown vs. deep sleep:**
- `M5.Power.timerSleep(seconds)` — full shutdown via RTC, wakes on timer. Use this for long intervals. This is the most battery-efficient option.
- `M5.Power.deepSleep(microseconds)` — ESP32 deep sleep, restarts on wake.
- Light sleep is generally not useful; the device consumes too much power in light sleep and touch wakeup is unreliable due to GT911 INT pin behavior.

**Cannot power off via USB.** `M5.Power.powerOff()` and `timerSleep()` only work when running on battery. When plugged into USB, the device stays on regardless.

**RTC wakeup:** Use `M5.Power.timerSleep()` with the `rtc_time_t` overload for reliable scheduled wakeup. The `int seconds` overload has known timing drift issues at certain intervals.

**Touch wakeup from deep sleep:** Use `GPIO_NUM_36` (TOUCH_INT) as the external wakeup pin. Note: the GT911 controller pulses this pin at ~24 Hz when idle, which can cause false wakeups. Test thoroughly before relying on touch wakeup.

```cpp
// Wakeup from touch during deep sleep
gpio_hold_en(GPIO_NUM_2);  // hold M5EPD_MAIN_PWR_PIN to keep display power
gpio_deep_sleep_hold_en();
esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, LOW);
esp_deep_sleep_start();
```

**Battery voltage detection:** There is no direct USB vs. battery detection. As a workaround, a voltage reading above ~4200 mV from `M5.Power.getBatteryVoltage()` generally indicates USB power.

## Hardware Notes

- **Power button** is the rotary dial knob; long-press 2s to power on/off. The reset button is on the back.
- **Do not expose the e-ink screen to UV light.** Extended UV exposure causes permanent damage.
- **Upload errors** ("Failed to write to target RAM", timeout): reinstall the USB serial driver. Both CP210x and CH9102 drivers may be needed depending on board revision.
- **Portrait orientation** is 540x960. **Landscape** is 960x540. Set rotation before drawing anything.

## Project Conventions

- Call `M5.update()` at the start of every `loop()` iteration.
- Always call `M5.Display.clear()` after changing EPD mode.
- Commit before asking Claude to make changes.
- Do not refactor unrelated code unless explicitly asked.
- Keep functions short and single-purpose.
- Use `Serial.begin(115200)` for debug output; remove before final build.

## Useful References

- M5Paper docs: https://docs.m5stack.com/en/core/m5paper_v1.1
- Arduino compile/flash guide: https://docs.m5stack.com/en/arduino/m5paper/program
- M5GFX quick start: https://docs.m5stack.com/en/quick_start/m5gfx/m5gfx
- M5Unified power API: https://docs.m5stack.com/en/arduino/m5unified/power_class
- M5EPD migration guide: https://docs.m5stack.com/en/arduino/m5paper/migrate_to_m5unified
- M5Stack community forum: https://community.m5stack.com

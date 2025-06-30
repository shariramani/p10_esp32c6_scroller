ESP32-C6+P10 RSS News scroller and Clock with Web Configuration and OTA
====================================================

Overview:
---------
This Arduino IDE based ESP32-C6 project functions as a smart RSS-driven news scroller and clock
that can fetch and display news headlines on P10 boards (red monochrome and color options), keep accurate time with or
without the Internet, and be configured entirely over a web interface.

Key Features:
-------------

1. Wi-Fi Manager (No Hardcoded SSID/PW)
   - Starts in AP mode (ESP32_Config) if no saved Wi-Fi
   - User selects SSID from drop down and enters password via captive portal
   - Credentials saved in non-volatile memory
   - Minimal or no dependency on external library.

2. Web-Based Configuration UI
   - Accessible via browser at device IP (e.g., http://192.168.4.1 or local IP)
   - Configure:
     - Fetch Interval (seconds)
     - Max News Age (hours)
     - Timezone select from dropdown(IANA zones, may be 20 most common if any performace issues while many)
     - Manual Time Set (YYYY-MM-DD HH:MM:SS)
   - Displays current time with timezone offset

3. RSS Feed Fetching
   - Loads list of feeds from 'feeds.json' in SPIFFS
   - Supports HTTPS with redirects
   - Uses TinyXML2 for XML parsing
   - Falls back to regex parsing on failure
   - Prints up to configurable number of headlines per feed to serial console. default 10
   - User can enable/disable specific feeds

4. Persistent Storage (SPIFFS)
   - 'feeds.json' holds feed names, URLs, enable flags
   - 'settings.json' holds fetch interval, timezone, max age, number of headlines
   - text files like fun facts, quote of the day, Option in config UI to select files and scroll line by line in random or sequence
   - All survive reboot and OTA updates

5. Timezone & Time Management
   - IANA-style timezones mapped to POSIX TZ strings (20 built-in if performace effect or many if no major impact)
   - Offset applied using setenv("TZ", ...) + tzset()
   - Time source:
     - NTP (if Internet)
     - DS3231 RTC module (optional)
     - Manual entry (fallback)

6. OTA Firmware Update
   - Accessible from web UI
   - Upload compiled .bin from Arduino IDE's "Export Compiled Binary"

7. Robust & Resilient
   - Handles bad URLs, malformed RSS, empty responses
   - Automatically skips to next feed if one fails
   - Logs all errors and steps to Serial

Planned or Optional:
--------------------
- Scroll headlines and time on P10 LED display
- Upload .txt files to scroll custom messages
- MQTT/Alexa/Google Assistant integration in later phase
- Mobile-friendly UI with theme support

Usage:
------
1. Flash the firmware via USB
2. Connect to ESP32_Config and visit 192.168.4.1/wifi
3. Set Wi-Fi, timezone, and fetch settings
4. Monitor output on Serial Monitor or P10 display
5. Visit http://<device-ip>/ to adjust settings or update firmware

------------------------------------------------------------
Developed using Arduino IDE, ESPAsyncWebServer, TinyXML2, and SPIFFS.
Built for reliability, simplicity, and modular extension.

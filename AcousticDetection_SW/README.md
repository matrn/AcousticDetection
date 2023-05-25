# Source code of the AcousticDetection device

The code is written in Arduino framework with the help of PlatformIO platform.


## Code setup
- copy `include/config.hpp.example` to `include/config.hpp` and set required values
- if you want to change distance between microphones, edit `lib/dsp/config.hpp`
- if you want Wi-Fi AP mode instead of station mode, uncomment `#define ENABLE_WIFI_AP` in the `src/main.cpp` file
- to disable OTA (Over the Air) update, comment out `#define ENABLE_OTA`
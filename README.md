# Smart alarm
## Description
It's a smart alarm working on ESP. A LED tape is connected to the ESP through a MOSFET. Because of this, there is an artificial sunrise effect (thanks to PWM).

![web UI](https://i.imgur.com/SxQ2D1F.png)! [web UI2](https://i.imgur.com/YA1SfWu.png)

## Usage

To repeat the project, you need:
1. Write down the required constants in 'main.cpp':
    - Wi-Fi ssid and password into variables'ssid' and `passwordWIFI`, respectively.
    - A password to web UI into `webPwd`. You will need to enter it only once, and it will be saved in cookies.
    - A pin to which the MOSFET is connected  into `ALARM_PIN`.
2. Write static into `main.cpp` by running `python simplify.py`.
3. Using PlatformIO, compile a project and flash it to ESP.
4. After flashing, find the ESP IP (nmap, router settings, or something else).
5. Open this URL in a browser to access the web UI.
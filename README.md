# The Last Meter Interface: Web Control for Your Homebrew Hardware
### [Turku <3 Frontend](https://www.meetabit.com/communities/turku-3-frontend) 25-Nov-2025 Demo projects 
For frontend developers, the world of DIY electronics is an exciting new frontier.

This session is about unlocking the potential to control the physical world using the web technologies.

[MCU binaries](binaries)

[Slides(PDF)](The-Last-Meter-Interface.pdf)

## Tools used:
* [CLion 2025.3](https://www.jetbrains.com/clion/embedded/) IDE
* [Junie](https://www.jetbrains.com/junie/) AI coding agent 
* [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html) Code Generator



## Web Bluetooth (BLE)
###  [ST NUCLEO-WB55RG](https://www.st.com/en/evaluation-tools/nucleo-wb55rg.html)  
### Extra LEDs:
| Color  | GPIO | Arduino Pin |
|--------|------|-------------|
| Red    | PA1  | A2          | 
| Yellow | PC1  | A1          | 
| Green  | PC0  | A0          | 
| *GND*  | PA0  | A3          | 

[HTML Control Page](https://www.elmot.xyz/speeches/2025-last-meter/ble-traffic-light.html)

[MDN Documentation](https://developer.mozilla.org/en-US/docs/Web/API/Web_Bluetooth_API)

[Source code](BLE_Custom)

## Wired Examples
### [STM32F3DISCOVERY](https://www.st.com/en/evaluation-tools/stm32f3discovery.html)

[Source code](BLE_Custom)

| Color  | GPIO |
|--------|------|
| Red    | PA10 | 
| Yellow | PA8  | 
| Green  | PC8  | 

### Web Serial Example

[HTML Control Page](https://www.elmot.xyz/speeches/2025-last-meter/serial-traffic-light.html)

[MDN Documentation](https://developer.mozilla.org/en-US/docs/Web/API/Web_Serial_API)

### WebUSB Example

[HTML Control Page](https://www.elmot.xyz/speeches/2025-last-meter/usb-traffic-light.html)

[MDN Documentation](https://developer.mozilla.org/en-US/docs/Web/API/WebUSB_API)

### WebHID Example

[Source code commit](https://github.com/elmot/2025-speech-last-mile/tree/webserial_webhid/f3-traffic-light)

[HTML Control Page](https://www.elmot.xyz/speeches/2025-last-meter/hid-traffic-light.html)

[MDN Documentation](https://developer.mozilla.org/en-US/docs/Web/API/WebHID_API)



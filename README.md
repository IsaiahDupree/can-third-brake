# CAN Signal Controller with Wi-Fi Interface

This project implements a CAN bus signal controller with Wi-Fi control capabilities, designed for automotive applications. It can monitor and control brake lights, turn signals, and hazard lights based on CAN bus messages while providing a web interface for remote control.

## Hardware Requirements

- ESP32 Development Board
- MCP2515 CAN Bus Module
- Relay Module (4 channels minimum)
- Power supply appropriate for your vehicle's electrical system
- Appropriate wiring and connectors

## Pin Connections

### CAN Module (MCP2515)
- CS Pin: GPIO 10
- INT Pin: GPIO 2
- SCK: SCK pin of ESP32
- MOSI: MOSI pin of ESP32
- MISO: MISO pin of ESP32

### Relay Pins
- Brake Light: GPIO 5
- Left Turn Signal: GPIO 16
- Right Turn Signal: GPIO 17
- Hazard Lights: GPIO 18

## Required Libraries

1. SPI.h (Built into Arduino IDE)
2. mcp_can.h (Install via Arduino Library Manager)
3. WiFi.h (Built into ESP32 core)

## Installation

1. Install the Arduino IDE
2. Install the ESP32 board support package
3. Install the required libraries
4. Upload the code to your ESP32

## Wi-Fi Configuration

The device creates a Wi-Fi Access Point with:
- SSID: "MyCarSignalAP"
- Password: "12345678"

Once connected, access the control interface at http://192.168.4.1

## Safety Notes

1. **Vehicle Integration**
   - Ensure proper isolation between vehicle systems
   - Use appropriate fuses and protection circuits
   - Verify CAN bus message IDs for your specific vehicle

2. **Security**
   - Change the default Wi-Fi password before deployment
   - Consider implementing additional security measures

3. **Legal Compliance**
   - Check local regulations regarding vehicle modifications
   - Ensure modifications comply with vehicle safety standards

## Customization

### CAN Message IDs
Current example uses placeholder IDs:
- Brake: 0x100
- Left Turn: 0x101
- Right Turn: 0x102
- Hazard: 0x103

Update these in the code to match your vehicle's actual CAN IDs.

### Pin Assignments
Modify the pin definitions in the code if using different GPIO pins.

## Troubleshooting

1. **CAN Bus Issues**
   - Verify CAN bus speed matches your vehicle
   - Check physical connections and termination resistors
   - Use Serial Monitor for debugging CAN messages

2. **Wi-Fi Connection**
   - Ensure proper power supply to ESP32
   - Check serial output for IP address assignment
   - Verify Wi-Fi credentials

## License

This project is provided as-is for educational purposes. Use at your own risk.

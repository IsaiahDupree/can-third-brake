/*
   ESP32 + MCP2515 CAN-bus + Relay Control + Wi-Fi AP
   ---------------------------------------------------------
   Listens on the CAN bus for certain signals (brakes, turn signals, hazards)
   Controls relay outputs accordingly.
   Also provides a basic web page to toggle or disable these relays (safety override).
*/

// Include necessary libraries
#include <SPI.h>
#include <mcp_can.h>
#include <WiFi.h>

// --------------------- CAN Settings ---------------------
#define MCP2515_CS_PIN    10   // SPI CS pin
#define MCP2515_INT_PIN   2    // Interrupt pin from MCP2515
#define CAN_SPEED         CAN_500KBPS  // Common speed is 500 kbps, adjust to match vehicle

// Example CAN IDs for demonstration:
#define BRAKE_CAN_ID      0x100
#define LEFT_TURN_CAN_ID  0x101
#define RIGHT_TURN_CAN_ID 0x102
#define HAZARD_CAN_ID     0x103

// --------------------- Relay Pins -----------------------
#define BRAKE_RELAY_PIN   5
#define LEFT_RELAY_PIN    16
#define RIGHT_RELAY_PIN   17
#define HAZARD_RELAY_PIN  18

// --------------------- WiFi Settings ---------------------
const char* AP_SSID     = "MyCarSignalAP";
const char* AP_PASSWORD = "12345678";

// Global flags to track whether each signal is automatically controlled via CAN
// or has been disabled manually over Wi-Fi
bool brakeEnabled   = true;
bool leftEnabled    = true;
bool rightEnabled   = true;
bool hazardEnabled  = true;

// Create CAN object
MCP_CAN CAN(MCP2515_CS_PIN);

// Create a server on port 80
WiFiServer server(80);

// Helper function to safely set a relay
void setRelay(int pin, bool on) {
  digitalWrite(pin, on ? HIGH : LOW);
}

// -------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  delay(1000);

  // ----------------- Set up Relay Pins -----------------
  pinMode(BRAKE_RELAY_PIN, OUTPUT);
  pinMode(LEFT_RELAY_PIN,  OUTPUT);
  pinMode(RIGHT_RELAY_PIN, OUTPUT);
  pinMode(HAZARD_RELAY_PIN,OUTPUT);

  // Initialize all relays off
  setRelay(BRAKE_RELAY_PIN,  false);
  setRelay(LEFT_RELAY_PIN,   false);
  setRelay(RIGHT_RELAY_PIN,  false);
  setRelay(HAZARD_RELAY_PIN, false);

  // ----------------- Initialize MCP2515 ---------------
  if (CAN.begin(CAN_SPEED) == CAN_OK) {
    Serial.println("CAN Bus Initialized Successfully!");
  } else {
    Serial.println("Error Initializing CAN Bus...");
    while (1) {
      delay(100);
    }
  }

  // Optionally set filters/masks here if you only want specific CAN IDs
  // e.g. CAN.init_Mask(0, 0, 0x7FF);  // Full mask
  //      CAN.init_Filt(0, 0, BRAKE_CAN_ID);

  // ------------------ Start Wi-Fi AP -------------------
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Access Point started. IP: ");
  Serial.println(myIP);

  // Start the server
  server.begin();
  Serial.println("Server started.");
}

// -------------------------------------------------------
void loop()
{
  // 1. Check for incoming CAN messages
  readCANMessages();

  // 2. Check for Wi-Fi clients & handle web requests
  handleWiFiClients();
}

// -------------------------------------------------------
// Reads CAN messages, detects signals, and sets relays
void readCANMessages()
{
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char rxBuf[8];

    // Read message
    CAN.readMsgBufID(&rxId, &len, rxBuf);

    // Print for debugging
    Serial.print("Received CAN ID: 0x");
    Serial.print(rxId, HEX);
    Serial.print(" Data: ");
    for (byte i = 0; i < len; i++) {
      Serial.print(rxBuf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Example: if the incoming ID matches one of our known signals:
    if (rxId == BRAKE_CAN_ID && brakeEnabled) {
      // Here we assume that if any non-zero data means "brake on"
      // Adjust logic based on your actual CAN data mapping
      bool brakeOn = (rxBuf[0] != 0); 
      setRelay(BRAKE_RELAY_PIN, brakeOn);

    } else if (rxId == LEFT_TURN_CAN_ID && leftEnabled) {
      bool leftOn = (rxBuf[0] != 0);
      setRelay(LEFT_RELAY_PIN, leftOn);

    } else if (rxId == RIGHT_TURN_CAN_ID && rightEnabled) {
      bool rightOn = (rxBuf[0] != 0);
      setRelay(RIGHT_RELAY_PIN, rightOn);

    } else if (rxId == HAZARD_CAN_ID && hazardEnabled) {
      bool hazardOn = (rxBuf[0] != 0);
      setRelay(HAZARD_RELAY_PIN, hazardOn);
    }
  }
}

// -------------------------------------------------------
// Serves a simple webpage that lets you enable/disable
// the 4 signals. This is a simplistic approach that
// reloads the page on each toggle click.
void handleWiFiClients()
{
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  while (!client.available()) {
    delay(1);
  }

  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Parse toggles in the request
  if (request.indexOf("/toggleBrake") != -1) {
    brakeEnabled = !brakeEnabled;
    if (!brakeEnabled) {
      setRelay(BRAKE_RELAY_PIN, false);
    }
  }
  if (request.indexOf("/toggleLeft") != -1) {
    leftEnabled = !leftEnabled;
    if (!leftEnabled) {
      setRelay(LEFT_RELAY_PIN, false);
    }
  }
  if (request.indexOf("/toggleRight") != -1) {
    rightEnabled = !rightEnabled;
    if (!rightEnabled) {
      setRelay(RIGHT_RELAY_PIN, false);
    }
  }
  if (request.indexOf("/toggleHazard") != -1) {
    hazardEnabled = !hazardEnabled;
    if (!hazardEnabled) {
      setRelay(HAZARD_RELAY_PIN, false);
    }
  }

  // Build the HTML response
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8' />";
  html += "<title>Signal Control</title></head><body>";
  html += "<h1>Car Signal Control</h1>";

  html += "<p>Brake: "   + String(brakeEnabled   ? "Enabled" : "Disabled") + " <a href='/toggleBrake'>[Toggle]</a></p>";
  html += "<p>Left: "    + String(leftEnabled    ? "Enabled" : "Disabled") + " <a href='/toggleLeft'>[Toggle]</a></p>";
  html += "<p>Right: "   + String(rightEnabled   ? "Enabled" : "Disabled") + " <a href='/toggleRight'>[Toggle]</a></p>";
  html += "<p>Hazard: "  + String(hazardEnabled  ? "Enabled" : "Disabled") + " <a href='/toggleHazard'>[Toggle]</a></p>";

  html += "</body></html>";

  // Send HTTP Response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();  
  client.println(html);
  client.stop();
}

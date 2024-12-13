/***connecting ardunio uno to em 18 rfid, servo motor and 3 ir sensor. this is a smart parking project. 
1st the car with rfid will scan in with em 18 if it is match then the servo work as a gate here it will open for 3 second and close again . later the car move to parking  space where we have 3 slots in it. if already someother car is parked then it should say this slot is filled. we are using nodemcu connected to ardunio for transfering data so that the website will display the slot infomation. give code for this concept with step by step connection process.***/

// **Connections**
// 1. EM-18 RFID Reader
//    - EM-18 VCC to Arduino 5V
//    - EM-18 GND to Arduino GND
//    - EM-18 TX to Arduino RX (Pin 2)

// 2. SG90 Servo Motor (Gate)
//    - Servo VCC to Arduino 5V
//    - Servo GND to Arduino GND
//    - Servo Signal to Arduino Pin 9

// 3. IR Sensors (3 slots)
//    - IR Sensor 1 OUT to Arduino Pin 3
//    - IR Sensor 2 OUT to Arduino Pin 4
//    - IR Sensor 3 OUT to Arduino Pin 5
//    - All IR VCCs to Arduino 5V
//    - All IR GNDs to Arduino GND

// 4. NodeMCU (via Serial Communication)
//    - NodeMCU TX to Arduino Pin 7
//    - NodeMCU RX to Arduino Pin 6
//    - NodeMCU GND to Arduino GND

#include <Servo.h>
#include <SoftwareSerial.h>

Servo gateServo; // Servo motor for the gate
SoftwareSerial nodeMCU(6, 7); // RX, TX for NodeMCU communication

// IR Sensors Pins
const int IR_Slot1 = 3;
const int IR_Slot2 = 4;
const int IR_Slot3 = 5;

// RFID Setup
const int rfidRxPin = 2; // RFID Reader TX connected to Arduino RX (Pin 2)
String allowedRFID = "1234567890"; // Example RFID UID (modify accordingly)

void setup() {
  Serial.begin(9600); // RFID Reader communication
  nodeMCU.begin(9600); // NodeMCU communication

  gateServo.attach(9); // Servo motor pin
  gateServo.write(0); // Ensure the gate is closed initially

  // Initialize IR sensors
  pinMode(IR_Slot1, INPUT);
  pinMode(IR_Slot2, INPUT);
  pinMode(IR_Slot3, INPUT);
}

void loop() {
  // **RFID Scanning and Gate Control**
  if (Serial.available() > 0) {
    String rfidData = "";
    while (Serial.available() > 0) {
      rfidData += char(Serial.read());
      delay(10);
    }
    rfidData.trim();

    if (rfidData == allowedRFID) {
      Serial.println("RFID Matched. Gate Opening.");
      gateServo.write(90); // Open gate
      delay(3000); // Wait for 3 seconds
      gateServo.write(0); // Close gate
    } else {
      Serial.println("RFID Not Matched. Access Denied.");
    }
  }

  // **Check Parking Slot Availability**
  int slot1Status = digitalRead(IR_Slot1);
  int slot2Status = digitalRead(IR_Slot2);
  int slot3Status = digitalRead(IR_Slot3);

  String slotData = "";
  slotData += (slot1Status == LOW ? "Slot1:Occupied," : "Slot1:Available,");
  slotData += (slot2Status == LOW ? "Slot2:Occupied," : "Slot2:Available,");
  slotData += (slot3Status == LOW ? "Slot3:Occupied" : "Slot3:Available");

  // Send slot status to NodeMCU
  nodeMCU.println(slotData);

  // Print slot data for debugging
  Serial.println(slotData);

  delay(1000); // Wait before the next update
}

// HTML for displaying the parking slot status
// Place this HTML and CSS in the NodeMCU's SPIFFS or serve through HTTP
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Parking System</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      background: #f4f4f9;
      color: #333;
    }
    .container {
      text-align: center;
      background: #fff;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
    }
    h1 {
      color: #007bff;
      margin-bottom: 20px;
    }
    .slot {
      font-size: 1.2em;
      margin: 10px 0;
    }
    .available {
      color: #28a745;
    }
    .occupied {
      color: #dc3545;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Parking Slot Status</h1>
    <div id="slot1" class="slot">Slot 1: <span class="available">Available</span></div>
    <div id="slot2" class="slot">Slot 2: <span class="occupied">Occupied</span></div>
    <div id="slot3" class="slot">Slot 3: <span class="available">Available</span></div>
  </div>
  <script>
    setInterval(() => {
      fetch("/slotdata")
        .then(response => response.json())
        .then(data => {
          document.getElementById('slot1').innerHTML = `Slot 1: <span class="${data.slot1 === 'Available' ? 'available' : 'occupied'}">${data.slot1}</span>`;
          document.getElementById('slot2').innerHTML = `Slot 2: <span class="${data.slot2 === 'Available' ? 'available' : 'occupied'}">${data.slot2}</span>`;
          document.getElementById('slot3').innerHTML = `Slot 3: <span class="${data.slot3 === 'Available' ? 'available' : 'occupied'}">${data.slot3}</span>`;
        });
    }, 1000);
  </script>
</body>
</html>
)rawliteral";

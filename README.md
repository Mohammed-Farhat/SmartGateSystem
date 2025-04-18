# Smart Gate System

**Smart Gate System** is an IoT‐based project designed to control and monitor a gate using an **ESP8266 (NodeMCU)**. It integrates:

- **RFID** for authorized access  
- **Physical buttons** for manual control  
- **Blynk app** for remote control and status  
- **WhatsApp alerts** via CallMeBot for unauthorized attempts  

---

## 🔑 Features

- **RFID Access Control**  
  Opens the gate only for authorized cards.  
- **Manual Control**  
  Push‑buttons for local open/close.  
- **Mobile Control**  
  Blynk app widgets to open/close and display gate status.  
- **WhatsApp Alerts**  
  Real‑time unauthorized‑access notifications.  
- **Visual Indicators**  
  LEDs show “Open” (green) and “Closed” (red) states.

---

## 🛠 Components

| Component              | Purpose                                   |
|------------------------|-------------------------------------------|
| ESP8266 (NodeMCU)      | Main microcontroller                      |
| MFRC522 RFID Reader    | Reads card UIDs                           |
| Servo Motor            | Moves gate between open/closed positions  |
| LEDs                   | Indicate gate status                      |
| Push Buttons           | Manual open/close control                 |
| Blynk App              | Remote control & status dashboard         |
| CallMeBot WhatsApp API | Sends unauthorized‑access alerts          |

---
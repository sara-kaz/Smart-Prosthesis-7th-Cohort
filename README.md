# Smart Prosthesis 7th Cohort: Foot Controller

## Introduction

Prosthetic arm controllers have seen advancements ranging from myoelectric sensors to voice recognition systems and brain activity scanners. However, these controllers often come with drawbacks such as reliability issues, mental fatigue, and limited ability to detect repeated commands. To address these challenges and improve prosthetic acceptance rates amid rising rejection rates, the Infinity Foot Controller has been developed. This controller aims to provide users with a simple yet effective way to control their prosthetic devices, leveraging the natural connection in movement between their arms and feet.

The Infinity Foot Controller utilizes the latest wireless communication technologies like Bluetooth Low Energy and WiFi 6. Its design focuses on noninvasive control, where users use their toes for finger control and ankle movement for wrist control. The foot controller sleeve is designed to be shoe-independent and adjustable, ensuring comfort and compatibility for various users.

## Goals

The goals of the foot control system include:

- Creating a shoe-independent foot sleeve
- Implementing up-to-date communication protocols
- Improving walking detection
- Ensuring real-time data transmission
- Enhancing servo control
- Simplifying manufacturing processes
- Increasing security and safety features

## Tasks

The tasks involved in achieving these goals are:

- Designing a wireless foot sleeve using WiFi 6 and ESP-NOW Protocols
- Utilizing flexible materials like TPU 95A for comfort and adjustability
- Moving the foot controller unit to BLE for stability
- Implementing real-time data transfer
- Simplifying device design and assembly
- Ensuring repairability and long battery life
- Enhancing security measures
- Implementing successful walking detection algorithms
- Transitioning to parallel servo actuation and efficient payload processing
- Infinity Foot Controller

## Repository Structure

### /Arm_Code/

- **ArmCode_4_9_ESPBLEBTNCombined.ino**: Main code for the arm controller integrating ESP and BLE button functionality.
- **SP23_24Logo.png**: Project logo image.
- **SP_Logo.h**: Header file for the project logo.
- **armServer.h**: Header file for the arm server.
- **processToeButtons.h**: Header file for processing toe button inputs.
- **wristRotations.h**: Header file for controlling wrist rotations.

### /Foot-Controller/

- **BatteryCharger.h**: Header file for the battery charger module.
- **FootControl_4_9_Button.ino**: Main code for the foot control with button integration.
- **SeeedAcceloTrigger.h**: Header file for the accelerometer trigger.

### /Foot-Sleeve/

- **FootSleeve_4_9_ESPNOW.ino**: Main code for the foot sleeve using ESP-NOW protocol.

## Components Overview

### Foot Controller Unit (FCU)
The FCU is responsible for controlling wrist rotation and bending using foot movements. It includes:

- Microcontroller with gyroscope for motion detection.
- Bluetooth connectivity for wireless communication.
- Fast data processing capabilities for real-time responsiveness.

### Foot Controller Sleeve (FCS)
The FCS enables finger flexion and extension control via buttons. It features:

- Flexible TPU material for a custom fit and comfort.
- Adjustable design for compatibility with various users.
- USB-C connectivity for modularity and ease of connection.

## Bluetooth and ESP-NOW Integration

The system utilizes Bluetooth Low Energy (BLE) for efficient wireless communication between the foot controller and the prosthetic arm. The ArduinoBLE library facilitates secure data transmission, while ESP-NOW is used to directly connect buttons to the arm, reducing wiring complexity and allowing flexible placement.

## Walking Detection Algorithm

Sophisticated algorithms analyze gyroscope and accelerometer data to detect walking movements accurately. When specific conditions are met, such as significant foot movement indicating walking, the foot controller enters a temporary sleep mode to avoid sending unintended commands to the arm.

## Contact

For any questions or issues, please contact sarakhaled.kaz@gmail.com.

---
Feel free to reach out with any questions or feedback regarding this Foot Controller in C/C++. Happy coding!


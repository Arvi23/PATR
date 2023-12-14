# Tunnel Monitoring System

## Overview
The Tunnel Monitoring System is a robust and efficient application designed to ensure safety and manage traffic flow within tunnels. It features real-time monitoring capabilities for smoke detection, natural gas levels, and vehicle counting. Developed in C, this system utilizes POSIX threads for concurrent operations, ensuring high responsiveness and reliability.

## Features
- **Smoke Detection**: Constant monitoring of smoke levels to quickly identify fire hazards.
- **Natural Gas Detection**: Real-time measurement of natural gas concentrations to prevent explosions or health risks.
- **Vehicle Counting**: Accurate tracking of the number of vehicles in the tunnel, aiding in traffic management and congestion prevention.

## Getting Started

### Prerequisites
- GCC Compiler
- POSIX-compliant environment (Linux/Unix)

### Installation
1. Clone the repository:
   ```bash
   git clone [repository-url]
2. Compile the program:
   ```bash
   gcc -o tunnel_monitor tunnel_monitor.c -lpthread
3. Run the program:
   ```bash
   ./tunnel_monitor.c
   
##  Interact with the program using the following commands:
    p - Pause monitoring
    r - Reset sensor values and tunnel state
    c - Continue monitoring
    s - Display smoke level
    g - Display natural gas level
    t - Display number of cars in the tunnel
    a - Display alert flag status
    q - Quit the program


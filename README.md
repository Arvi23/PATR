#Tunnel Monitoring System
Overview

This Tunnel Monitoring System is designed to ensure the safety and efficiency of vehicle passage through a tunnel. It features real-time monitoring of smoke levels, natural gas levels, and vehicle count within the tunnel. The system is implemented in C and uses POSIX threads for concurrent operations.
Features

    Smoke Detection: Monitors the smoke level within the tunnel to detect potential fire hazards.
    Natural Gas Detection: Measures the natural gas level to prevent explosions or health hazards.
    Car Counting: Keeps track of the number of vehicles in the tunnel to manage traffic flow and detect congestion.

Requirements

    GCC Compiler
    POSIX compliant environment for pthreads

    Installation

Navigate to the project directory:

bash

cd tunnel-monitoring-system

Compile the program:

bash gcc -o tunnel_monitor main.c -lpthread

./tunnel_monitor

Interact with the program using the following commands:

    p - Pause monitoring
    r - Reset sensor values and tunnel state
    c - Continue monitoring
    q - Quit the program
    s - Display smoke level
    g - Display natural gas level
    t - Display number of cars in the tunnel
    a - Display alert flag status
    [Other commands as implemented]

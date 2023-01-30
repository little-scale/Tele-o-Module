# Firmware examples for Tele-o Module

## tele_o_OSC.ino
• Example of receiving OSC data at /cv/0 [float], /cv/1 [float], /cv/2 [float], /cv/3 [float], /gate/0 [int], /gate/1 [int], 
• Don't forget to change: char ssid[] = "your network name"; and char pass[] = "your network password"; to actual string values
• Also, you may wish to change the IP address and port number

## little-scale.tele-o.amxd
• Accompanying Max for Live device that sends data via the UDP object formatted as OSC messages

# Esp32-LoRa-Wan-CO2-Sender
Esp32 CO2 and Temperature Sensor
This sensor is a self built LoRa Wan Esp32 based microcontroller with CO2 Sensor. 
The LoRa Wan module used is the SX1276 at the frequency 868MHz and the CO2 Sensor used is the MH-Z19C.
For convenience the board only needs to be flashed once, for further adjustment a configuration website is hosted at the ip address displayed on the COM port.
The IP-adresse is given dynamically by the router. This Esp32 LoRa Wan module is intended to be used with the LoRa Wan Gateway, but can also be used with the COM port.  

#Initial Setup
Flash the esp32 with Plattform IO and create two files:
1. wifipwd.txt
2. wifissid.txt

In the data directory of this repository on your local machine. To flash these two files upload the "OTA filesystem" and restart the esp32. 
The Esp32 should then display you the IP-Adresse it has been assigned by the router via the COM-port, further adjustments can be made on the website reachable 
under https://the-assigned-ip-of-your-esp32.



#include <JPEGDecoder.h>
#include <SD.h>
#include "Arduino.h"
#include "sensoriser.h"

int sendSensors(String sensorReadings) {

  // Create a buffer for the packet
  char dataBuff[packetSize];

  // Fill the buffer with zeros
  initBuff(dataBuff);

  // Create a packet with info about the sensors
  String header = "$ITSNS,";
  header += sensorReadings;
  header += stopSymbol;
  header.toCharArray(dataBuff, packetSize);

  // Send the packet
  sendBuff(dataBuff);

  return 0;
}

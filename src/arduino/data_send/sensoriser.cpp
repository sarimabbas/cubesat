#include <JPEGDecoder.h>
#include <SD.h>
#include "Arduino.h"
#include "sensoriser.h"

int sendSensors(double* sensorReadings, String* sensorNames, int numSensors) {

  // Create a buffer for the packet
  char dataBuff[packetSize];

  // Fill the buffer with zeros
  initBuff(dataBuff);

  // Create a packet with info about the sensors
  String header = "$SNS,";
  for(int i = 0; i < numSensors; i++) {
    header += sensorNames[i];
    header += sensorReadings[i];
    header += ",";
  }

  // copy header to buffer
  header.toCharArray(dataBuff, packetSize);

  // Send the packet
  sendBuff(dataBuff);

  return 0;
}

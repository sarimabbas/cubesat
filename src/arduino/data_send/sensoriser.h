

#ifndef sensoriser_h
#define sensoriser_h

#include <SD.h>
#include "Arduino.h"
#include "packets.h"

const String stopSymbol = "$";

int sendSensors(String sensorReadings);

#endif



#ifndef sensoriser_h
#define sensoriser_h

#include <SD.h>
#include "Arduino.h"
#include "packets.h"

int sendSensors(double* sensorReadings, String* sensorNames, int numSensors);

#endif

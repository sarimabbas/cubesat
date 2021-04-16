

#ifndef pixeliser_h
#define pixeliser_h

#include <SD.h>
#include "Arduino.h"

int pixeliseJpeg(File jpegFile);

// Function to fill the packet buffer with zeros
void initBuff(char *buff);


#endif

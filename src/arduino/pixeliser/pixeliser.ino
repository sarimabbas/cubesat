
// Include the library
#include "pixeliser.h"
#include <JPEGDecoder.h>

// Define Slave Select pin
#define SD_CS 6

void setup()
{
  // Set pin 13 to output, otherwise SPI might hang
  pinMode(13, OUTPUT);

  // Begin Serial port for communication with PC
  Serial.begin(115200);

   while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.println("Setting up...");

  // Start the SD
  if (!SD.begin(SD_CS))
  {
    // If the SD can't be started, loop forever
    Serial.println("SD failed or not present!");
    while (1)
      ;
  }

  // Open the root directory
//  File root = SD.open("/");

  // Wait for the PC to signal
  while(!Serial.available());

  // Send all files on the SD card
  while (true)
  {
    // Open the next file
    File jpegFile = SD.open("1.JPG");
    
    pixeliseJpeg(jpegFile);

    jpegFile.close();
    

    break;
  }

  // Safely close the root directory
//  root.close();
}


void loop()
{
  // Nothing here
  // We don't need to send the same images over and over again
}

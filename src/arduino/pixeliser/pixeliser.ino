
// Include the library
#include "pixeliser.h"
#include <JPEGDecoder.h>

// Define Slave Select pin
#define SD_CS 6

const int NUM_IMAGES = 2;
String imageNames[] = {"1.JPG", "2.JPG"};
int imageCounter = 0;

void setup()
{
  // Set pin 13 to output, otherwise SPI might hang
  pinMode(13, OUTPUT);

  // Begin Serial port for communication with PC
  Serial.begin(115200);

  while (!Serial)
  {
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
}

void loop()
{
  // get the current image
  String currentImageToTransmit = imageNames[imageCounter];

  // pixelise it
  File jpegFile = SD.open(currentImageToTransmit);
  pixeliseJpeg(jpegFile);
  jpegFile.close();

  // transmit next image next iteration
  imageCounter = (imageCounter + 1) % NUM_IMAGES;

  // wait 5 seconds before next image
  delay(5000);
}

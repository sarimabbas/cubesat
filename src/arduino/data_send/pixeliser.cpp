#include <JPEGDecoder.h>
#include <SD.h>
#include "Arduino.h"
#include "pixeliser.h"

int pixeliseJpeg(File jpegFile)
{
  // Decode the JPEG file
  JpegDec.decodeSdFile(jpegFile);

  // Create a buffer for the packet
  char dataBuff[packetSize];

  // Fill the buffer with zeros
  initBuff(dataBuff);

  // Create a header packet with info about the image
  String header = "$ITHDR,";
  header += JpegDec.width;
  header += ",";
  header += JpegDec.height;
  header += ",";
  header += JpegDec.MCUSPerRow;
  header += ",";
  header += JpegDec.MCUSPerCol;
  header += ",";
  header += jpegFile.name();
  header += ",";
  header.toCharArray(dataBuff, packetSize);

  // Send the header packet
  sendBuff(dataBuff);

  // Pointer to the current pixel
  uint16_t *pImg;

  // Color of the current pixel
  uint16_t color;

  // Create a data packet with the actual pixel colors
  strcpy(dataBuff, "$ITDAT");
  uint8_t i = 6;

  // Repeat for all MCUs in the image
  while (JpegDec.read())
  {
    // prepare data packet
    //    initBuff(dataBuff);
    //    strcpy(dataBuff, "$ITDAT");
    //    uint8_t i = 6;

    // Save pointer the current pixel
    pImg = JpegDec.pImage;

    // Get the coordinates of the MCU we are currently processing
    int mcuXCoord = JpegDec.MCUx;
    int mcuYCoord = JpegDec.MCUy;

    // Get the number of pixels in the current MCU
    uint32_t mcuPixels = JpegDec.MCUWidth * JpegDec.MCUHeight;

    // Repeat for all pixels in the current MCU
    while (mcuPixels--)
    {
      // Read the color of the pixel as 16-bit integer
      color = *pImg;
      pImg++;

      // Split it into two 8-bit integers
      dataBuff[i] = color >> 8;
      dataBuff[i + 1] = color;
      i += 2;

      // If the packet is full, send it
      if (i == packetSize)
      {
        sendBuff(dataBuff);
        i = 6;
      }

      // END OF IMAGE
      // If we reach the end of the image, send a packet
      if ((mcuXCoord == JpegDec.MCUSPerRow - 1) &&
          (mcuYCoord == JpegDec.MCUSPerCol - 1) &&
          (mcuPixels == 1))
      {

        // Send the pixel values
        for (int j = 0; j < i; j++)
        {
          Serial.write(dataBuff[j]);
        }

        // Fill the rest of the packet with zeros
        for (int k = i; k < packetSize; k++)
        {
          Serial.write((byte)0);
        }
      }
    }
  }

  return 0;
}

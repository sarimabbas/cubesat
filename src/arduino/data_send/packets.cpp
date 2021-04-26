#include "Arduino.h"
#include "packets.h"

// Function to fill the packet buffer with zeros
void initBuff(char *buff)
{
  for (int i = 0; i < packetSize; i++)
  {
    buff[i] = 0;
  }
}

// send the buffer over Serial
void sendBuff(char *buff)
{
  Serial.write(buff, packetSize);
}

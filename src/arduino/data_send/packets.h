

#ifndef packets_h
#define packets_h

const int packetSize = 240;

// Function to fill the packet buffer with zeros
void initBuff(char *buff);

// send the buffer over Serial
void sendBuff(char *buff);

#endif

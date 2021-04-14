#include <ArduCAM.h>
#include <JPEGDecoder.h>
#include <memorysaver.h>
#include <SPI.h>
#include <Wire.h>

// Define SD card chip select pin
#define SD_CS  6

// Define camera 1 and 2 chip select pins
const int CS1 = 7;
const int CS2 = 5;

//This can only work on OV5640_MINI_5MP_PLUS or OV5642_MINI_5MP_PLUS platform.
#if !(defined (OV5640_MINI_5MP_PLUS)||defined (OV5642_MINI_5MP_PLUS))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define   FRAMES_NUM    0x00
bool CAM2_EXIST = false;
bool CAM1_EXIST = false; 

//Check which version cameras exist
#if defined (OV5640_MINI_5MP_PLUS)
  ArduCAM myCAM1(OV5640, CS1);
  ArduCAM myCAM2(OV5640, CS2);
#else
  ArduCAM myCAM1(OV5642, CS1);
  ArduCAM myCAM2(OV5642, CS2);
#endif



int k=0; //this will be the image file number
int l=0; //this will be used for the previous image file number
int zero = 0;

char str[8]; //used to hold the string for the filename
char srtLast[8];  //used to hold the string for the previous filename

//For Camera and SD card (SPI):
uint8_t vid, pid;
uint8_t temp; 



void setup() {

// Set the chip selects
  pinMode(CS1,OUTPUT);
  digitalWrite(CS1,HIGH);
  pinMode(CS2,OUTPUT);
  digitalWrite(CS2,HIGH);
  pinMode(SD_CS,OUTPUT);
  digitalWrite(SD_CS,HIGH);

// toggle Arduino on-board LED as indication of board start-up 
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  
// Begin Serial port for communication with PC
//  Serial.begin(115200); // initial setting
  Serial.begin(230400);   // speed up serial, reliable with USB - serial
//  Serial.begin(500000); // didn't work at all
//  Serial.begin(250000); // didn't work at all

// Begin serial I2C communications
  Wire.begin();

// Begin SPI communications
  SPI.begin();
  SPI.beginTransaction(SPISettings(2000000,MSBFIRST,SPI_MODE0)); // default Arduino SPI speed is 4MHz - SD card seemed to have trouble at 4MHz

// Start the SD
  if(!SD.begin(SD_CS)) {
    // If the SD can't be started, loop forever
    Serial.println("SD failed or not present!");
    delay(1000);
   while(1);
  }

// Start the ArduCAMs
//  Serial.println(F("ArduCAM start"));
 
// Reset the CPLDs in the ArduCAMs
  myCAM1.write_reg(0x07,0x80);
  delay(100);
  myCAM1.write_reg(0x07, 0x00);
  delay(100); 
  myCAM2.write_reg(0x07, 0x80);
  delay(100);
  myCAM2.write_reg(0x07, 0x00);
  delay(100); 

// toggle Arduino on-board LED as indication of board start-up
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);

//Check if the 2 ArduCAM Mini 5MP Plus Cameras' SPI bus is OK
  while(1){
    myCAM1.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM1.read_reg(ARDUCHIP_TEST1);
    if(temp != 0x55)
    {
      Serial.println(F("SPI1 interface Error!"));
    }else{
      CAM1_EXIST = true;
      }
    myCAM2.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM2.read_reg(ARDUCHIP_TEST1);
    if(temp != 0x55){
      Serial.println(F("SPI2 interface Error!"));
      }else{
        CAM2_EXIST = true;
//        Serial.println(F("SPI2 interface OK."));
        }
  if(!(CAM1_EXIST||CAM2_EXIST)){    //cameras' SPI bus working - ok to continue
    delay(1000);continue;
    }else
      break;
  }

// toggle Arduino on-board LED as indication of board start-up
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);

// verify camera is present based on camera type
  #if defined (OV5640_MINI_5MP_PLUS)
  while(1){
    //Check if the camera module type is OV5640
   myCAM1.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
   myCAM1.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
   if ((vid != 0x56) || (pid != 0x40)){
     Serial.println(F("Can't find OV5640 module!"));
     delay(1000);continue;
     }else{
        break;
        }   
    }

  #else
    while(1){
    //Check if the camera module type is OV5642
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42)){
      Serial.println(F("Can't find OV5642 module!"));
      delay(1000);continue;
      }else{
        break;
        }  
    }
  #endif

//Change to JPEG capture mode and initialize the OV5642 module
  myCAM1.set_format(JPEG);
  myCAM1.InitCAM();
  myCAM1.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
  myCAM2.set_format(JPEG);
  myCAM2.InitCAM();
  myCAM2.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
  myCAM1.clear_fifo_flag();
  myCAM1.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
  myCAM2.clear_fifo_flag();
  myCAM2.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
  myCAM1.clear_fifo_flag();
  myCAM2.clear_fifo_flag(); 

// toggle Arduino on-board LED as indication of board start-up
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
}
//////////////////END OF SETUP///////////////

//////////////////BEGIN LOOP/////////////////

void loop() {

// char str[8];

 //Construct a file name
  k = k + 1;
  itoa(k, str, 10);
  strcat(str, ".jpg");

//clear out the serial read buffer to prepare to receive image request - should improve on this method
  clearReadBuffer();

//other resolution choices: OV5642_320x240,640x480,1024x768,1280x960,1600x1200,2048x1536,2592x1944
  if(CAM1_EXIST)
  myCAM1.OV5642_set_JPEG_size(OV5642_320x240);delay(1000);
// myCAM1.OV5642_set_JPEG_size(OV5642_640x480);delay(1000);
// myCAM1.OV5642_set_JPEG_size(OV5642_2592x1944);delay(1000);
  myCAMSaveToSDFile(myCAM1);
  delay(250);

//Construct a file name
  k = k + 1;
  itoa(k, str, 10);
  strcat(str, ".jpg");
 
  if(CAM2_EXIST)
  myCAM2.OV5642_set_JPEG_size(OV5642_320x240);delay(1000);
// myCAM2.OV5642_set_JPEG_size(OV5642_640x480);delay(1000);
// myCAM2.OV5642_set_JPEG_size(OV5642_2592x1944);delay(1000);   
  myCAMSaveToSDFile(myCAM2); 
  delay(250);

//  send the ready packet with sensor information to Processing
  sendReadyPacket();

// wait for response from Processing
  delay(5000);

// received something from Processing - assume image request, send last two images acquired
  if(Serial.available()){
    l = k - 1;  //find the filename of the previous image
    itoa(l, str, 10);
    strcat(str, ".jpg");
    pixelize();
    delay(1000);
    itoa(k, str, 10);
    strcat(str, ".jpg");
    pixelize();
    delay(1000);
  }
}
////////////////END LOOP////////////////////

////////////////myCAMSaveToSDFile///////////
// routine to acquire image and save file to SD card

void myCAMSaveToSDFile(ArduCAM myCAM){
//  char str[8];
  byte buf[256];
  static int i = 0;
  uint8_t temp = 0,temp_last=0;
  uint32_t length = 0;
  bool is_header = false;
  File outFile;
  
//Flush the FIFO
  myCAM.flush_fifo();
  
//Clear the capture done flag
  myCAM.clear_fifo_flag();
  
//Start capture
  myCAM.start_capture();
  
//  Serial.println(F("start Capture"));
  while(!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
  
//  Serial.println(F("Capture Done."));  
  length = myCAM.read_fifo_length();
  if (length >= MAX_FIFO_SIZE) //8M
  {
  return ;
  }
  if (length == 0 ) //0 kb
  {
    return ;
  }

// open file on SD card with write access
  outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
  if(!outFile){
    Serial.print("File ");
    Serial.print(k);
    Serial.println(" open failed");
    return;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    
  //Read JPEG data from FIFO
    if ((temp == 0xD9) && (temp_last == 0xFF)){ //If find the end, break while
     buf[i++] = temp;                           //save the last  0XD9
          
    //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      outFile.write(buf, i);
          
    //Close the file
      outFile.close();
      is_header = false;
      i = 0;
    }  
    if (is_header == true)
    { 
    //Write image data to buffer if not full
      if (i < 256)
        buf[i++] = temp;
      else
    {
      //Write 256 bytes image data to file
      myCAM.CS_HIGH();
      outFile.write(buf, 256);
      i = 0;
      buf[i++] = temp;
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();
    }        
  }
  else if ((temp == 0xD8) & (temp_last == 0xFF)){
    is_header = true;
    buf[i++] = temp_last;
    buf[i++] = temp;   
    } 
  }
 
}

////////////////////pixelize//////////////////////

void pixelize() {
  
//open the desired file
 File jpgFile = SD.open(str);

 JpegDec.decodeSdFile(jpgFile);
    
// Create a buffer for the packet
  char dataBuff[240];

// Fill the buffer with zerosllll
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
  header += jpgFile.name();
  header += ",";
  header.toCharArray(dataBuff, 240);


// Send the header packet
  for(int j=0; j<240; j++) {
    Serial.write(dataBuff[j]);
    }

// Pointer to the current pixel
   uint16_t *pImg;

// Color of the current pixel
   uint16_t color;

// Create a data packet with the actual pixel colors
   strcpy(dataBuff, "$ITDAT");
   uint8_t i = 6;

// Repeat for all MCUs in the image
   while(JpegDec.read()) {
   // Save pointer the current pixel
   pImg = JpegDec.pImage;

// Get the coordinates of the MCU we are currently processing
   int mcuXCoord = JpegDec.MCUx;
   int mcuYCoord = JpegDec.MCUy;

// Get the number of pixels in the current MCU
   uint32_t mcuPixels = JpegDec.MCUWidth * JpegDec.MCUHeight;

// Repeat for all pixels in the current MCU
   while(mcuPixels--) {
   // Read the color of the pixel as 16-bit integer
   color = *pImg++;
        
   // Split it into two 8-bit integers
   dataBuff[i] = color >> 8;
   dataBuff[i+1] = color;
   i += 2;

    // If the packet is full, send it
    if(i == 240) {
      for(int j=0; j<240; j++) {
      Serial.write(dataBuff[j]);
      }
        i = 6;
     }

// If we reach the end of the image, send a packet
     if((mcuXCoord == JpegDec.MCUSPerRow - 1) && 
          (mcuYCoord == JpegDec.MCUSPerCol - 1) && 
          (mcuPixels == 1)) {
          
      // Send the pixel values
         for(int j=0; j<i; j++) {
          Serial.write(dataBuff[j]);
          }
          
      // Fill the rest of the packet with zeros
         for(int m=i; m<240; m++) {
          Serial.write(zero);  
          }
          
       }
     }
   }
}  

// Function to fill the packet buffer with zeros
void initBuff(char* buff) {
  for(int i = 0; i < 240; i++) {
    buff[i] = 0;
  }
}

// Function to clear the serial input buffer
void clearReadBuffer() {
  byte w = 0;
  for(int i = 0; i<10; i++)
  {
    while(Serial.available()>0)
    {
      char c = Serial.read();
      w++;
      delay(1);
    }
    delay(1);
  }
}

// Function to send ready packet

  void sendReadyPacket() {

   char dataBuff[240];
  
   String header = "$ITRDY,";
   
//  bogus sensor information to test parsing
    header += k;
    header += ",";
    header += "24";
    header += ",";
    header += "48";
    header += ",";
    header += "96";
    header += ",";
    header += "192";
    header += ",";
    header.toCharArray(dataBuff, 240);


    // Send the header packet
    for(int j=0; j<240; j++) {
      Serial.write(dataBuff[j]);
    }
  }

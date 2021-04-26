//Working 4/12/21
//saves pictures from 2 cameras, one at a time, over and over

#include <ArduCAM.h>
#include <JPEGDecoder.h>
String fileNum;
String fileType;
String fullFileName;
static int k; //k is the number in the JPEG file name

#define SD_CS 6
// set pins 5 and7 as the slave select for SPI:
const int CS1 = 7;
const int CS2 = 5;
#include <SD.h>
#include "memorysaver.h"
File dataFile; //file to store the sensor data

// include the SD library and SPI interface:
#include <SPI.h>

#include <Wire.h> // library for the I2C devices

//This can only work on OV5640_MINI_5MP_PLUS or OV5642_MINI_5MP_PLUS platform.
#if !(defined(OV5640_MINI_5MP_PLUS) || defined(OV5642_MINI_5MP_PLUS))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define FRAMES_NUM 0x00
bool CAM2_EXIST = false;
bool CAM1_EXIST = false;

#if defined(OV5640_MINI_5MP_PLUS)
ArduCAM myCAM1(OV5640, CS1);
ArduCAM myCAM2(OV5640, CS2);
#else
ArduCAM myCAM1(OV5642, CS1);
ArduCAM myCAM2(OV5642, CS2);
#endif

void setup()
{

  //Begin serial I2C communications
  Wire.begin();
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }

  //For Camera and SD card (SPI):
  uint8_t vid, pid;
  uint8_t temp;

  Serial.println(F("ArduCAM Start!"));
  // set the CS output:
  pinMode(CS1, OUTPUT);
  digitalWrite(CS1, HIGH);
  pinMode(CS2, OUTPUT);
  digitalWrite(CS2, HIGH);

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  // initialize SPI:
  SPI.begin();
  //Reset the CPLD
  myCAM1.write_reg(0x07, 0x80);
  delay(100);
  myCAM1.write_reg(0x07, 0x00);
  delay(100);
  myCAM2.write_reg(0x07, 0x80);
  delay(100);
  myCAM2.write_reg(0x07, 0x00);
  delay(100);

  //Check if the 2 ArduCAM Mini 5MP PLus Cameras' SPI bus is OK
  while (1)
  {
    myCAM1.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM1.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55)
    {
      Serial.println(F("SPI1 interface Error!"));
    }
    else
    {
      CAM1_EXIST = true;
      Serial.println(F("SPI1 interface OK."));
    }
    myCAM2.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM2.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55)
    {
      Serial.println(F("SPI2 interface Error!"));
    }
    else
    {
      CAM2_EXIST = true;
      Serial.println(F("SPI2 interface OK."));
    }
    if (!(CAM1_EXIST || CAM2_EXIST))
    {
      delay(1000);
      continue;
    }
    else
      break;
  }
  //Initialize SD Card
  while (!SD.begin(SD_CS))
  {
    Serial.println(F("SD Card Error"));
    delay(1000);
  }
  Serial.println(F("SD Card detected."));
#if defined(OV5640_MINI_5MP_PLUS)
  while (1)
  {
    //Check if the camera module type is OV5640
    myCAM1.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
    myCAM1.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x40))
    {
      Serial.println(F("Can't find OV5640 module!"));
      delay(1000);
      continue;
    }
    else
    {
      Serial.println(F("OV5640 detected."));
      break;
    }
  }
#else
  while (1)
  {
    //Check if the camera module type is OV5642
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42))
    {
      Serial.println(F("Can't find OV5642 module!"));
      delay(1000);
      continue;
    }
    else
    {
      Serial.println(F("OV5642 detected."));
      break;
    }
  }
#endif
  //Change to JPEG capture mode and initialize the OV5642 module
  myCAM1.set_format(JPEG);
  myCAM1.InitCAM();
  myCAM1.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK); //VSYNC is active HIGH
  myCAM2.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK); //VSYNC is active HIGH
  //myCAM3.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
  //myCAM4.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
  myCAM1.clear_fifo_flag();
  myCAM1.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
  myCAM2.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);

  myCAM1.clear_fifo_flag();
  myCAM2.clear_fifo_flag();
  k = 0; //for number in JPEG file name
}
//////////////////////////////////////////////////////////////////////////////////////
void loop()
{

  //other resolution choices: OV5642_320x240,640x480,1024x768,1280x960,1600x1200,2048x1536,2592x1944
  if (CAM1_EXIST)
    Serial.println("CAM1 detected");
  myCAM1.OV5642_set_JPEG_size(OV5642_320x240);
  delay(1000);
  //myCAM1.OV5642_set_JPEG_size(OV5642_2592x1944);delay(1000);
  myCAMSaveToSDFile(myCAM1);
  delay(250);

  if (CAM2_EXIST)
    Serial.println("CAM2 detected");
  myCAM2.OV5642_set_JPEG_size(OV5642_320x240);
  delay(1000);
  //myCAM2.OV5642_set_JPEG_size(OV5642_2592x1944);delay(1000);
  myCAMSaveToSDFile(myCAM2);
  delay(250);
}

////////////////////////////////////////////
void myCAMSaveToSDFile(ArduCAM myCAM)
{
  char str[8];
  byte buf[256];
  static int i = 0;
  //static int k = 0; //declare k up top so can access in other methods
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  bool is_header = false;
  File outFile;
  //Flush the FIFO
  myCAM.flush_fifo();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();
  Serial.println(F("start Capture"));
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
    ;
  Serial.println(F("Capture Done."));
  length = myCAM.read_fifo_length();
  Serial.print(F("The fifo length is :"));
  Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE) //8M
  {
    Serial.println(F("Over size."));
    return;
  }
  if (length == 0) //0 kb
  {
    Serial.println(F("Size is 0."));
    return;
  }
  //Construct a file name
  k = k + 1;
  itoa(k, str, 10);
  strcat(str, ".jpg");
  //Open the new file
  outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
  if (!outFile)
  {
    Serial.println(F("File open faild"));
    return;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  while (length--)
  {
    temp_last = temp;
    temp = SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ((temp == 0xD9) && (temp_last == 0xFF)) //If find the end ,break while,
    {
      buf[i++] = temp; //save the last  0XD9
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      outFile.write(buf, i);
      //Close the file
      outFile.close();
      Serial.println(F("Image save OK."));
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
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;
    }
  }
}

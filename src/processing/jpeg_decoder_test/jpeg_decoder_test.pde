// Import the library
import processing.serial.*;

Serial port;

void setup() {
  // Set the default window size to 200 by 200 pixels
  size(200, 200);
  
  // Set the background to grey
  background(#888888);
  
  // Set as high framerate as we can
  frameRate(1000000);
  
  // Start the COM port communication
  // You will have to replace "COM30" with the Arduino COM port number
  port = new Serial(this, "/dev/cu.usbmodem14401", 115200);
  
  // Read 240 bytes at a time
  port.buffer(240);
}

// String to save the trimmed input
String trimmed;

// Buffer to save data incoming from Serial port
byte[] byteBuffer = new byte[240];

// The coordinate variables
int x, y, mcuX, mcuY;

// A variable to measure how long it takes to receive the image
long startTime;

// A variable to save the current time
long currentTime;

// Flag to signal end of transmission
boolean received = false;

// Flag to signal reception of header packet
boolean headerRead = false;

// The color of the current pixel
int inColor, r, g, b;

// Image information variables
int jpegWidth, jpegHeight, jpegMCUSPerRow, jpegMCUSPerCol, mcuWidth, mcuHeight, mcuPixels;

// This function will be called every time any key is pressed
void keyPressed() {
  // Send something to Arduino to signal the start
  port.write('s');
}

// This function will be called every time the Serial port receives 240 bytes
void serialEvent(Serial port) {
  // Read the data into buffer
  port.readBytes(byteBuffer);
  
  // Make a String out of the buffer
  String inString = new String(byteBuffer);
  
  // Detect the packet type
  if (inString.indexOf("$ITHDR") == 0) {
    // Header packet
    
    // Remove all whitespace characters
    trimmed = inString.trim();
    
    // Split the header by comma
    String[] list = split(trimmed, ',');
    
    // Check for completeness
    if (list.length != 7) {
      println("Incomplete header, terminated");
      while(true);
    } else {
      // Parse the image information
      jpegWidth = Integer.parseInt(list[1]);
      jpegHeight = Integer.parseInt(list[2]);
      jpegMCUSPerRow = Integer.parseInt(list[3]);
      jpegMCUSPerCol = Integer.parseInt(list[4]);
      
      // Print the info to console
      println("Filename: " + list[5]);
      println("Parsed JPEG width: " + jpegWidth);
      println("Parsed JPEG height: " + jpegHeight);
      println("Parsed JPEG MCUs/row: " + jpegMCUSPerRow);
      println("Parsed JPEG MCUs/column: " + jpegMCUSPerCol);
      
      // Start the timer
      startTime = millis();
    }
    
    // Set the window size according to the received information
    surface.setSize(jpegWidth, jpegHeight);
    
    // Get the MCU information
    mcuWidth = jpegWidth / jpegMCUSPerRow;
    mcuHeight = jpegHeight / jpegMCUSPerCol;
    mcuPixels = mcuWidth * mcuHeight;
    
  } else if (inString.indexOf("$ITDAT") == 0) {
    // Data packet
    
    // Repeat for every two bytes received
    for (int i = 6; i < 240; i += 2) {
      // Combine two 8-bit values into a single 16-bit color
      inColor = ((byteBuffer[i] & 0xFF) << 8) | (byteBuffer[i + 1] & 0xFF);
      
      // Convert 16-bit color into RGB values
      r = ((inColor & 0xF800) >> 11) * 8;
      g = ((inColor & 0x07E0) >> 5) * 4;
      b = ((inColor & 0x001F) >> 0) * 8;
      
      // Paint the current pixel with that color
      set(x + mcuWidth * mcuX, y + mcuHeight * mcuY, color(r, g, b));
      
      // Move onto the next pixel
      x++;
      
      if (x == mcuWidth) {
        // MCU row is complete, move onto the next one
        x = 0;
        y++;
      }
      
      if (y == mcuHeight) {
        // MCU is complete, move onto the next one
        x = 0;
        y = 0;
        mcuX++;
      }
      
      if (mcuX == jpegMCUSPerRow) {
        // Line of MCUs is complete, move onto the next one
        x = 0;
        y = 0;
        mcuX = 0;
        mcuY++;
      }
      
      if (mcuY == jpegMCUSPerCol) {
        // The entire image is complete
        received = true;
      }
    }
  }
}

void draw() {
  // If we received a full image, start the whole process again
  if (received) {
    
    // Reset coordinates
    x = 0;
    y = 0;
    mcuX = 0;
    mcuY = 0;
    
    // Reset the flag
    received = false;
    
    // Measure how long the whole thing took
    long timeTook = millis() - startTime;
    println("Image receiving took: " + timeTook + " ms");
    println();
  }
}

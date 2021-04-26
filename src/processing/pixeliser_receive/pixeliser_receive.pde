// Import the library
import processing.serial.*;

Serial port;

boolean setupComplete = false;

void setup() {
  // Set the default window size to 200 by 200 pixels
  size(200, 200);

  // Set the background to grey
  background(#888888);

  // Set as high framerate as we can
  frameRate(1000000);

  // Start the COM port communication
  // You will have to replace "COM30" with the Arduino COM port number
  try {
    port = new Serial(this, "/dev/cu.usbmodem14301", 115200);
    port.buffer(240); // Read 240 bytes at a time
  } 
  catch(Exception e) {
    println("Could not open port. Exiting...");
    exit();
  }

  println("finishing setup...");
  setupComplete = true;
}

// The coordinate variables
int x, y, mcuX, mcuY;

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

void handleHeaderPacket(byte[] packet) {
  String packetAsString = new String(packet);

  // Remove all whitespace characters
  String trimmed = packetAsString.trim();

  // Split the header by comma
  String[] list = split(trimmed, ',');  

  // Check for completeness
  if (list.length != 7) {
    println("Incomplete header, terminated");
    return;
  }

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

  // Set the window size according to the received information
  surface.setSize(jpegWidth, jpegHeight);

  // Get the MCU information
  mcuWidth = jpegWidth / jpegMCUSPerRow;
  mcuHeight = jpegHeight / jpegMCUSPerCol;
  mcuPixels = mcuWidth * mcuHeight;
}

void handleDataPacket(byte[] byteBuffer) {
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

// takes a 240 character string packet and handles the image
void handlePacket(byte[] packet) {
  // convert packet to string
  String packetAsString = new String(packet);

  // get the packet type 
  String packetType = packetAsString.substring(0, 6);
  println(packetType);

  // if a header packet, send to the header packet handler function
  if (packetType.indexOf("$ITHDR") == 0) {
    handleHeaderPacket(packet);
  } 
  // if a data packet, send to the data packet handler function
  else if (packetType.indexOf("$ITDAT") == 0) {
    handleDataPacket(packet);
  } 
  // if the header is not one of the above, 
  // some mismatch has occured, in which case clear the port
  // and try again on the next packet
  else {
    port.clear();
  }
}

// This function will be called every time the Serial port receives 240 bytes
void serialEvent(Serial port) {
  if (!setupComplete) {
    return;
  }
  try {
    // Read data into buffer and send to function
    byte[] byteBuffer = new byte[240];
    port.readBytes(byteBuffer);
    handlePacket(byteBuffer);
  }
  catch(Exception e) {
    e.printStackTrace();
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
  }
}

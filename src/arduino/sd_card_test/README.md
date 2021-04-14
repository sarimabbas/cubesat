Please consult https://learn.adafruit.com/adafruit-micro-sd-breakout-board-card-tutorial/arduino-library

Also, there's a few useful things you can do with File objects:

- You can `seek()` on a file. This will move the reading/writing pointer to a new location. For example `seek(0)` will take you to the beginning of the file, which can be very handy!
- Likewise you can call `position()` which will tell you where you are in the file.
- If you want to know the size of a file, call `size()` to get the number of bytes in the file.
- Directories/folders are special files, you can determine if a file is a directory by calling `isDirectory()`
- Once you have a directory, you can start going through all the files in the directory by calling `openNextFile()`
- You may end up with needing to know the name of a file, say if you called `openNextFile()` on a directory. In this case, call `name()` which will return a pointer to the 8.3-formatted character array you can directly `Serial.print()` if you want.

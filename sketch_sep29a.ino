#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 (or 0x3F depending on your LCD), 16 columns and 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Address may differ

void setup() {
  // Initialize the LCD with the number of columns and rows
  lcd.begin(16, 2);  // Begin with 16 columns and 2 rows
  lcd.backlight();  // Turn on the backlight
  
  // Print "Hello World" to the LCD
  lcd.setCursor(0, 0);  // Set cursor to column 0, row 0
  lcd.print("Hello, World!");  // Display "Hello, World!"
  
  // Optionally, you can print more text on the second row
  lcd.setCursor(0, 1);  // Set cursor to column 0, row 1
  lcd.print("I2C LCD Module");  // Display more text on the second row
}

void loop() {
  // No action required in the loop for this simple "Hello World" example
}
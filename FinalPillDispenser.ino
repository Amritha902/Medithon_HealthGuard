/*
   https://github.com/NormanDunbar/ArduinoClock/blob/master/Sources/ArduinoClock/ArduinoClock.ino

   This works well good way to set the RTC via serial input Enter u to start

  This is working WELL with 28byj stepper
  https://www.makerguides.com/28byj-48-stepper-motor-arduino-tutorial/
  button wiring
  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Button

*/
#include <Arduino.h>
#include <Wire.h>                   // For I2C communication
#include <LiquidCrystal_I2C.h>      // For LCD
#include <RTClib.h>                 // For RTC

// ORIGINAL: My LCD uses address 0x3F and is 20 by 4.
//Had to change to the following for my LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // Create LCD with I2C address 0x27, 16 characters per line, 2 lines


RTC_DS3231 rtc;                     // Create RTC for the DS3231 RTC module, address is 0x68 fixed.

// Include the AccelStepper library:
#include <AccelStepper.h>

// Motor pin definitions:
#define motorPin1  8      // IN1 on the ULN2003 driver
#define motorPin2  9      // IN2 on the ULN2003 driver
#define motorPin3  10     // IN3 on the ULN2003 driver
#define motorPin4  11     // IN4 on the ULN2003 driver

// Define the AccelStepper interface type; 4 wire motor in half step mode MotorInterfaceType 8: 4096 steps per rotation
// For full step mode change MotorInterfaceType to 4
#define MotorInterfaceType 8

// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper library with 28BYJ-48 stepper motor:
AccelStepper stepper = AccelStepper(MotorInterfaceType, motorPin1, motorPin3, motorPin2, motorPin4);

//time settings for pill dispensing
//AM
int amHr = 8;
int amMin = 0;
int amSec = 0;
//PM
int pmHr = 21;
int pmMin = 00;
int pmSec = 0;

// constants won't change. They're used here to set pin numbers:
const int led = 2;
const int button = 5;

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status


/* WIRING DIAGRAM:

    ARDUINO     RTC     LCD DISPLAY
    -------------------------------
        A4      SDA         SDA
        A5      SCL         SCL
        5V      VCC         VCC
       GND      GND         GNG


    The code below is a modified version of the code at:
    https://www.circuitbasics.com/how-to-use-a-real-time-clock-module-with-the-arduino
    and was modified by:

    Norman Dunbar [norman (at) dunbar-it (dot) co (dot) uk] to:

    Use built in RTCLib functions to
    format the date and time for display on the LCD.

    carry out validation checks on the user input and
    the DateTime created from same, before updating the RTC.


    USAGE:

    The clock runs happily without problems, as long as it has 5V down the USB!

    Open the serial monitor, or similar, and type u (and enter). The LCD will display "Edit mode"
    and you will be prompted to enter the years, months etc to set the clock.

    While entering details, if you need to abort, type -1 for the value and all changes so
    far will be aborted. The LCD will return to showing the date and time as before.

    All inputs are validated for a certain range of values. Months are 1 to 12, but what about
    leap years? Those are not validated (yet) and neither are "30 days hath September ...".

    After all inputs are done, a temporary DateTime is created with the entered values. If
    this is a valid date time - leap year ok, days in month ok etc - then the RTC will be
    set. If the DateTime is invalid, then we go around all the inputs again.

*/

// Minimum and maximum values structure for each input.
typedef struct minMax_t {
  int minimum;
  int maximum;
};


/*
   Function to validate user input.
   Returns TRUE if value in range, FALSE otherwise.
*/
bool checkInput(const int value, const minMax_t minMax) {
  if ((value >= minMax.minimum) &&
      (value <= minMax.maximum))
    return true;

  Serial.print(value);
  Serial.print(" is out of range ");
  Serial.print(minMax.minimum);
  Serial.print(" - ");
  Serial.println(minMax.maximum);
  return false;
}

/*
   Function to update RTC time using user input.
*////////////////////////////////////////////////////////////////////////////////////////////////////////
void updateRTC()
{

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Edit Mode...");

  // Prompts for user input.
  const char txt[6][15] = { "year [4-digit]", "month [1~12]", "day [1~31]",
                            "hours [0~23]", "minutes [0~59]", "seconds [0~59]"
                          };


  // Define limits for the 6 user inputs.
  const minMax_t minMax[] = {
    {2000, 9999},   // Year
    {1, 12},        // Month
    {1, 31},        // Day
    {0, 23},        // Hours
    {0, 59},        // Minutes
    {0, 59},        // Seconds
  };

  String str = "";

  long newDate[6];
  DateTime newDateTime;

  // The outer loop. Goes around and around until we get a valid
  // date and time. Thats all 6 inputs valid. It does not validate
  // February and/or leap years - it doesn't have to DateTime.isValid()
  // does that for us.
  while (1) {
    while (Serial.available()) {
      Serial.read();  // Clear serial buffer
    }

    // We have 6 different user inputs to capture.
    for (int i = 0; i < 6; i++) {

      // This loop exits when one user input is valid in
      // as far as being numeric and in range, sort of.
      // Leap years and month end dates are validated later
      // when we have the complete date and time.
      while (1) {
        Serial.print("Enter ");
        Serial.print(txt[i]);
        Serial.print(" (or -1 to abort) : ");

        while (!Serial.available()) {
          ; // Wait for user input
        }

        str = Serial.readString();  // Read user input

        // The actual value depends on the line ending configured in
        // the Serial Monitor. The configured line end character(s)
        // are part of the input string!
        // If the value is -1, then we abort the clock change
        // completely and bale out of this function.
        if ((str == "-1")   || (str == "-1\n") ||
            (str == "-1\r") || (str == "-1\r\n")) {
          Serial.println("\nABORTED");
          return;
        }

        newDate[i] = str.toInt();   // Convert user input to number and save to array

        // Validate input is in range, exit this inner while() loop
        // if so, otherwise, lets go round again.
        if (checkInput(newDate[i], minMax[i]))
          break;

      }

      Serial.println(newDate[i]); // Show user their input
    }

    // We have all the user input, was it valid - leap years,
    // days in months etc. If the DateTime is valid, we are done
    // in the outer while() loop and will exit to set the RTC.
    newDateTime = DateTime(newDate[0], newDate[1], newDate[2], newDate[3], newDate[4], newDate[5]);
    if (newDateTime.isValid())
      break;

    // Otherwise, we have to do it all again.
    Serial.println("Date/time entered was invalid, please try again.");
  }

  // Update RTC as we have a valid date & time.
  rtc.adjust(newDateTime);
  Serial.println("RTC Updated!");
}

/*
   Function to update LCD text
*///////////////////////////////////////////////////////////////////////////////////////////////////////
void updateLCD()
{
  // Get time and date from RTC.
  DateTime rtcTime = rtc.now();

  /*
     Buffers to format the date and time (on separate lines of the LCD)

     Parameters are:

      | specifier | output                                                 |
      |-----------|--------------------------------------------------------|
      | YYYY      | the year as a 4-digit number (2000--2099)              |
      | YY        | the year as a 2-digit number (00--99)                  |
      | MM        | the month as a 2-digit number (01--12)                 |
      | MMM       | the abbreviated English month name ("Jan"--"Dec")      |
      | DD        | the day as a 2-digit number (01--31)                   |
      | DDD       | the abbreviated English day of the week ("Mon"--"Sun") |
      | AP        | either "AM" or "PM"                                    |
      | ap        | either "am" or "pm"                                    |
      | hh        | the hour as a 2-digit number (00--23 or 01--12)        |
      | mm        | the minute as a 2-digit number (00--59)                |
      | ss        | the second as a 2-digit number (00--59)                |

      If either "AP" or "ap" is used, the "hh" specifier uses 12-hour mode
      (range: 01--12). Otherwise it works in 24-hour mode (range: 00--23).

      The specifiers within _buffer_ will be overwritten with the appropriate
      values from the DateTime. Any characters not belonging to one of the
      above specifiers are left as-is.
  */

  char dateBuffer[] = "DD-MMM-YYYY DDD";
  //char timeBuffer[] = "hh:mm:ss AP"; //12 hour with AP
  char timeBuffer[] = "hh:mm:ss"; //24 hour
  char hoursBuffer[] = "hh";
  char minsBuffer[] = "mm";

  // Move LCD cursor to top line, far left position.
  //lcd.setCursor(0, 0);
  // lcd.print(rtcTime.toString(dateBuffer));

  // Move LCD cursor to second line, far left position.
  lcd.setCursor(0, 0);
  lcd.print("Time");
  lcd.setCursor(5, 0);
  lcd.print(rtcTime.toString(timeBuffer));

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{

  // Set stepper maximum steps per second:
  stepper.setMaxSpeed(1000);

  Serial.begin(9600);

  Serial.print(__FILE__);


  // ORIGINAL: My LiquidCrystal_I2C library is a different one and is
  //           built in to the Arduino. It uses begin() instead of init()
  //           to initialise the LCD.
  //    lcd.init();       // Initialize lcd

  lcd.begin();        // Initialize lcd
  lcd.backlight();    // Switch-on lcd backlight
  //lcd.noBacklight();    // Switch-off lcd backlight
  rtc.begin();        // Initialise RTC module.

  // set led pin to output
  pinMode(led, OUTPUT);
  // set button pin to INPUT
  pinMode(button, INPUT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  // Get time and date from RTC.
  DateTime rtcTime = rtc.now();

  // read the state of the pushbutton value:
  buttonState = digitalRead(button);

  updateLCD();
  if (rtcTime.hour() == amHr && rtcTime.minute() == amMin && rtcTime.second() == amSec)
  {
    lcd.clear();
    rotateAM(); //start stepper
  }
  if (rtcTime.hour() == pmHr && rtcTime.minute() == pmMin && rtcTime.second() == pmSec)
  {
    lcd.clear();
    rotatePM(); //start stepper
  }
  // Press button when pills taken. Check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {

    //lcd.backlight();    // Switch-on lcd backlight
    lcd.clear();
    digitalWrite(led, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Pills Taken");

    // rotateOver();
  }
  if (Serial.available()) {
    char input = Serial.read();
    if (input == 'u') updateRTC();
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void rotateAM() {
  // Set the current position to 0:
  stepper.enableOutputs();
  stepper.setCurrentPosition(0);

  // Run the motor forward at 500 steps/second until the motor reaches 4096 steps (1 revolution):
  // Gear ratio is 60 to 8 teeth ie 7.5:1 therefore 1/2 revolution will be 1/15th of 360 degrees = 2048 steps
  while (stepper.currentPosition() != 2048) {
    stepper.setSpeed(500);
    stepper.runSpeed();
  }
  stepper.disableOutputs();
  digitalWrite(led, HIGH);
  lcd.setCursor(0, 1);
  lcd.print("AM Pills Ready");

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void rotatePM() {
  // Set the current position to 0:
  stepper.enableOutputs();
  stepper.setCurrentPosition(0);

  // Run the motor forward at 500 steps/second until the motor reaches 4096 steps (1 revolution):
  // Gear ratio is 60 to 8 teeth ie 7.5:1 therefore 1/2 revolution will be 1/15th of 360 degrees = 2048 steps
  while (stepper.currentPosition() != 2048) {
    stepper.setSpeed(500);
    stepper.runSpeed();
  }
  stepper.disableOutputs();
  lcd.setCursor(0, 1 );
  lcd.print("PM Pills Ready");
  digitalWrite(led, HIGH);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Override timed rotation
void rotateOver() {
  // Set the current position to 0:
  stepper.enableOutputs();
  stepper.setCurrentPosition(0);

  // Run the motor forward at 500 steps/second until the motor reaches 4096 steps (1 revolution):
  while (stepper.currentPosition() != 2048) {
    stepper.setSpeed(500);
    stepper.runSpeed();
  }
  stepper.disableOutputs();
  lcd.setCursor(0, 1 );
  lcd.print("Timer Overridden");
  digitalWrite(led, HIGH);
}

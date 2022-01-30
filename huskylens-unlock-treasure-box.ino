#include "HUSKYLENS.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

//
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
//
#define SERVOMIN 70
#define SERVOMAX 600
#define CENTER 375
//
int x_servo = 0;
int y_servo = 1;
float x_movetopos;
float y_movetopos;
char cmd;
int sound_played = 0;
int switch_pin = 7;
int sound_playing = 0;
int locked = 1;

// for Huskylens
HUSKYLENS huskylens;
void printResult(HUSKYLENSResult result);
//
// for DFPlyer
SoftwareSerial dfplayerSerial(12, 13); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);
void lock();
void unlock();

//
void setup() {
  Serial.begin(115200);
  pinMode(switch_pin, INPUT);
  //--------------------------------------------------------
  // prepare DFPlayer
  dfplayerSerial.begin(9600);
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  if (!myDFPlayer.begin(dfplayerSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true) {
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  
  delay(1000);
  //--------------------------------------------------------
  // prepare huskylens
  Wire.begin();
  while (!huskylens.begin(Wire))
  {
    Serial.println(F("Begin failed!"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>I2C)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }
  delay(1000);
  //--------------------------------------------------------
  // prepare servo
  pwm.begin();
  pwm.setPWMFreq(60);
  for (int i = 0; i < 16; i++)
    pwm.setPWM(i, 0, CENTER);
  lock();
  Serial.println("Servo is Ready");
  delay(1000);

}

void loop() {
  Serial.print("sound_played is ");
  Serial.println(sound_played);
  Serial.println("=======================================");
  Serial.print("locked is ");
  Serial.println(locked);
  Serial.println("=======================================");


  int switch_value = digitalRead(switch_pin);
  Serial.print("switch value is :");
  Serial.println(switch_value);
  if (switch_value == 0) {
    Serial.println("SWITCHED PUSHED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!=======================================");
    if ( locked == 0) {
      lock();
    } else {
      Serial.println("did nothing");
    }

  }

  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
    if (myDFPlayer.readType() == DFPlayerPlayFinished && myDFPlayer.read() == 2) {
      unlock();

    }
  }

  if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
  else if (!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
  else if (!huskylens.available()) Serial.println(F("No block or arrow appears on the screen!"));
  else
  {
    Serial.println(F("###########"));
    while (huskylens.available())
    {
      HUSKYLENSResult result = huskylens.read();
      printResult(result);
      if (result.ID == 1) {
        if (locked == 1 && sound_playing == 0) {
          myDFPlayer.play(2);
          sound_playing = 1;
        }
      }
    }
  }


}
//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void lock() {
  sound_played = 0;
  sound_playing = 0;
  locked = 1;
  Serial.print("(locked!)sound_played is");
  Serial.println(sound_played);
  pwm.setPWM(x_servo, 0, SERVOMIN);
}
void unlock() {
  sound_played = 1;
  locked = 0;
  myDFPlayer.play(1);  //Play the first mp3 jyajyan!
  pwm.setPWM(x_servo, 0, SERVOMAX);

}
// just print things for debugging
void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block:xCenter=") + result.xCenter + F(",yCenter=") + result.yCenter + F(",width=") + result.width + F(",height=") + result.height + F(",ID=") + result.ID);
  }
  else if (result.command == COMMAND_RETURN_ARROW) {
    Serial.println(String() + F("Arrow:xOrigin=") + result.xOrigin + F(",yOrigin=") + result.yOrigin + F(",xTarget=") + result.xTarget + F(",yTarget=") + result.yTarget + F(",ID=") + result.ID);
  }
  else {
    Serial.println("Object unknown!");
  }
}
void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

}

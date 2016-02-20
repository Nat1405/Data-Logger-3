/*
  SD card datalogger

 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10

 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <LiquidCrystal.h>

Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

const int chipSelect = 10;

LiquidCrystal lcd(8, 9, 5, 4, 3, 2);

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; 
  }

  lcd.begin(16, 2);
  lcd.print("3-Axis Data:");
  
  Serial.println("Accelerometer Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}

void loop()
{
  // make a string for assembling the data to log:
  String dataString1 = "";
  String dataString2 = "";
  String dataString3 = "";

  sensors_event_t event; 
  accel.getEvent(&event);
  
 
  
    
    dataString1 += String(event.acceleration.x,1);
    lcd.setCursor(0,1);
    lcd.print(dataString1);
    dataString1 += ",";
    dataString2 += String(event.acceleration.y,1);
    lcd.setCursor(6,1);
    lcd.print(dataString2);
    dataString2 += ",";
    dataString3 += String(event.acceleration.z,1);
    lcd.setCursor(12,1);
    lcd.print(dataString3);
    dataString3 += ",";
  
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("bikeride.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString1);dataFile.print(dataString2);dataFile.print(dataString3);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString1);
    Serial.println(dataString2);
    Serial.println(dataString3);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  delay(500);

  lcd.setCursor(12,0);
  lcd.print(millis()/1000);
}










#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>

Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);

const int chipSelect = 10;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; 
  }
  
  Serial.println("Accelerometer Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }

  Serial.println("Magnetometer Test"); Serial.println("");
  
  /* Enable auto-gain */
  mag.enableAutoRange(true);
  
  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
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

  //strings for collecting potentiometer data:

  String dataString4 = "";
  String dataString5 = "";

  String dataString6 = "";
  String dataString7 = "";

  String dataString8 = "";

  //strings for collecting magnetometer details

  String dataString9 = "";
  String dataString10 = "";
  String dataString11 = "";

  String dataString12 = "";

  sensors_event_t event; 
  accel.getEvent(&event);
  mag.getEvent(&event);
  
 
  
    
    dataString1 += String(event.acceleration.x,1);
    dataString1 += ",";
    dataString2 += String(event.acceleration.y,1);
    dataString2 += ",";
    dataString3 += String(event.acceleration.z,1);
    dataString3 += ",";
    dataString4 += String(analogRead(A0));   //Pot1
    dataString4 += ",";
    dataString5 += String(analogRead(A1));   //Pot2
    dataString5 += ",";
    dataString6 += String(analogRead(A2));   //PotX
    dataString6 += ",";
    dataString7 += String(analogRead(A3));   //PotY
    dataString7 += ",";
    dataString8 += String(millis());
    dataString8 += ",";
    dataString9 += String(event.magnetic.x);
    dataString9 += ",";
    dataString10 += String(event.magnetic.y);
    dataString10 += ",";
    dataString11 += String(event.magnetic.z);
    dataString11 += ",";
    dataString12 += String(analogRead(A4));
    dataString12 += ",";

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("bikeride.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {// print to the serial port too:
    dataFile.print(dataString1);dataFile.print(dataString2);dataFile.print(dataString3);
    dataFile.print(dataString4);dataFile.print(dataString5);dataFile.print(dataString6);
    dataFile.print(dataString7);dataFile.print("");dataFile.print(dataString8);
    dataFile.print(dataString9);dataFile.print(dataString10);dataFile.print(dataString11);
    dataFile.println(dataString12);
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
}










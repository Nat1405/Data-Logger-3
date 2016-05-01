#include <SPI.h>
#include <Time.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
SdFat sd;
SdFile logFile;

Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);

//reduce size by making constants limited in size to thier requirements
const uint8_t chipSelect = 10;
//IMPORTANT: This value is the sample rate, and must be based on the SD card quality and it's write time
const double SAMPLE_MS = 10000;
// 1 if the setup is running on a breadboard
const boolean ON_BREADBOARD =1;
// make 0 if you don't want a header of column titles included
#define DATAHEADER 1

uint32_t logTime;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  Serial.println("Accelerometer Test"); Serial.println("");


  // Initialise the sensor
  if(!accel.begin())
  {
    // There was a problem detecting the ADXL345 ... check your connections
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }

  Serial.println("Magnetometer Test"); Serial.println("");


  mag.enableAutoRange(true);



  if(!mag.begin())
  {
    // There was a problem detecting the LSM303 ... check your connections
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }

  Serial.print("Initializing SD card...");

  // initalize SDFAT with speed dependent on if on breadboard, as Full speed
  // on breadboard or long wires can cause corruption in data
  if (!ON_BREADBOARD) {
  if(!sd.begin(chipSelect,SPI_FULL_SPEED))sd.initErrorHalt();
  } else  {
  if(!sd.begin(chipSelect,SPI_HALF_SPEED))sd.initErrorHalt();
  }

  logFile.open("datalog.txt",O_RDWR|O_CREAT|O_AT_END);// open with properties of Read/write, create if non-existant, and start at EOF
}//setup

void printHead() {
  Serial.print("Printing headers");
  logFile.print("Time (ms)");
  writeComma();
  logFile.print("accel-x");
  writeComma();
  logFile.print("accel-y");
  writeComma();
  logFile.print("accel-z");
  writeComma();
  logFile.print("A0");
  writeComma();
  logFile.print("A1");
  writeComma();
  logFile.print("A2");
  writeComma();
  logFile.print("Mag x");
  writeComma();
  logFile.print("Mag y");
  writeComma();
  logFile.print("A4");
  logFile.print("\n");
}//printHead

//stores data to write in cache, to be written by .sync
void collectData() {

  sensors_event_t event;
  accel.getEvent(&event);
  mag.getEvent(&event);

   logFile.print((double) micros());
   Serial.print(logTime);
   writeComma();
   logFile.print(event.acceleration.x);
   writeComma();
   logFile.print(event.acceleration.y);
   writeComma();
   logFile.print(event.acceleration.z);
   writeComma();
   logFile.print(analogRead(A0));
   writeComma();
   logFile.print(analogRead(A1));
   writeComma();
   logFile.print(analogRead(A2));
   writeComma();
   logFile.print(analogRead(A3));
   writeComma();
   logFile.print(event.magnetic.x);
   writeComma();
   logFile.print(event.magnetic.y);
   writeComma();
   logFile.print(event.magnetic.z);
   writeComma();
   logFile.print(analogRead(A4));
   logFile.write("\n");//new row
}//collectData

void writeComma() {
  logFile.write(",");
}//writeComma

void loop()
 {
   #if DATAHEADER
     Serial.print("entered if");
     if (firstRun) {
       printHead();
       firstRun = 0;
     }
   #endif //DATAHEADER true
   delay(400);  // catch Due reset problem
   logTime += (1000UL*SAMPLE_MS);//seconds to check runtime for to prevent corruption
   int32_t elapsed;
    do {
    elapsed = micros()-logTime;
  }while(elapsed < 0);
   //collect data

   Serial.print("Press any key to end");
   while (Serial.read() <= 0) {

   collectData();
   logFile.sync();//wite modified data to field
   Serial.print(".");
  }
  logFile.close();
  while(1);
 }//loop

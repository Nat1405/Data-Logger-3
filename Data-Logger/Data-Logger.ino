#include <SPI.h>
#include <Time.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM9DS0.h>
SdFat sd;
SdFile logFile;
//IMPORTANT: This value is the sample rate, and must be based on the SD card quality and it's write time
const int SAMPLE_MS = 5;
// 1 if the setup is running on a breadboard
const boolean ON_BREADBOARD =1;
// make 0 if you don't want a header of column titles included
#define DATAHEADER 1
#define START_ON_LAUNCH 1

Adafruit_LSM9DS0 lsm = Adafruit_LSM9DS0(1000);  // Use I2C, ID #1000
#define LSM9DS0_XM_CS 10
#define LSM9DS0_GYRO_CS 9
#define LSM9DS0_SCLK 13
#define LSM9DS0_MISO 12
#define LSM9DS0_MOSI 11

uint32_t logTime;
void setupSensors() {
  // accelerometer range
  lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_6G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_16G);

  //magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS0_MAGGAIN_2GAUSS);
  //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_12GAUSS);

  //gyroscope
  lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_2000DPS);
}//setupSensors
void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  Serial.print(F("LSM check"));
    if(!lsm.begin())
  {
    /* There was a problem detecting the LSM9DS0 ... check your connections */
    Serial.print(F("Ooops, no LSM9DS0 detected ... Check your wiring!"));
    while(1);
  }
  setupSensors();
  Serial.println(F("Found LSM9DS0 9DOF"));

  Serial.println(F("Initializing SD card..."));

  // initalize SDFAT with speed dependent on if on breadboard, as Full speed
  // on breadboard or long wires can cause corruption in data
  if (!ON_BREADBOARD) {
  if(!sd.begin(10,SPI_FULL_SPEED))sd.initErrorHalt();
  } else  {
  if(!sd.begin(10,SPI_HALF_SPEED))sd.initErrorHalt();
  }

  logFile.open("datalog.csv",O_RDWR|O_CREAT|O_AT_END);// open with properties of Read/write, create if non-existant, and start at EOF
  #if DATAHEADER
    printHead();
  #endif //DATAHEADER true

  #if START_ON_LAUNCH
  Serial.println(F("Waiting for launch"));
  sensors_event_t accel, mag, gyro, temp;
  while((accel.acceleration.z-9.81)<0) lsm.getEvent(&accel, &mag, &gyro, &temp);
  #endif

}//setup

void printHead() {
  logFile.print("Time(s)");
  writeComma();
  logFile.print("acceleration x(m/s^2)");
  writeComma();
  logFile.print("acceleration y(m/s^2)");
  writeComma();
  logFile.print("acceleration z(m/s^2)");
  writeComma();
  logFile.print("mag x(uT)");
  writeComma();
  logFile.print("mag y(uT)");
  writeComma();
  logFile.print("mag z(uT)");
  writeComma();
  logFile.print("gyro x(rad/s)");
  writeComma();
  logFile.print("gyro y(rad/s)");
  writeComma();
  logFile.print("gyro z(rad/s)");
  writeComma();
  logFile.print("Temp(C)");
  logFile.print("\n");
}//printHead

//stores data to write in cache, to be written by .sync
void collectData() {
   logFile.print((double) millis()/1000);
   Serial.println(millis());
   writeComma();
   sensors_event_t accel, mag, gyro, temp;
   lsm.getEvent(&accel, &mag, &gyro, &temp);
   logFile.print((double)accel.acceleration.x);
   writeComma();
   logFile.print((double)accel.acceleration.y);
   writeComma();
   logFile.print((double)accel.acceleration.z);
   writeComma();
   logFile.print((double)mag.magnetic.x);
   writeComma();
   logFile.print((double)mag.magnetic.y);
   writeComma();
   logFile.print((double)mag.magnetic.z);
   writeComma();
   logFile.print((double)gyro.gyro.x);
   writeComma();
   logFile.print((double)gyro.gyro.y);
   writeComma();
   logFile.print((double)gyro.gyro.z);
   writeComma();
   logFile.print((double)temp.temperature);
   logFile.print("\n");//new row
}//collectData

void writeComma() {
  logFile.write(",");
}//writeComma

void loop()
 {
   Serial.print(F("Press any key to end"));
   while (Serial.read() <= 0) {
    int32_t elapsed;
    do {
      elapsed = millis();
    }while(elapsed % SAMPLE_MS);
     //collect data

    collectData();
    logFile.sync();//wite modified data to field
   }// if keypress detected
    logFile.close();
    Serial.print(F("Time Elapsed (s)"));
    Serial.println((millis()/1000));
    while(1);
 }//loop

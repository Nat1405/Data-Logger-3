/*A simpler, fallback data-logger that uses more recognizable methods to write to SD
However, it still uses the SDfat libary in certain ways to improve logging
consider using this if you want a simpler logger with less fiddling, for testing
and if the binary logger breaks in some way */
#include <SPI.h>
#include <Time.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM9DS0.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>


//---------------------------------User Options---------------------------------
//Pin which the light data reader is on
const int LIGHT_PIN = 2;
//Pin which the funtion button is connected
const int BUTTON_PIN = 0;
//Data pin for temperature sensor
const int ONE_WIRE_BUS = 2;
//Pin for status light
const int ERROR_LED_PIN = 7;
//IMPORTANT: This value is the sample rate, and must be based on the SD card quality and it's write time
const int SAMPLE_MS = 100;
// 1 if the setup is running on a breadboard
const boolean ON_BREADBOARD =1;
// Set to 0 if you don't want a header of column titles included
#define DATAHEADER 1
//Set to 1 if you want launch detection, otherwise will use delay below
#define START_ON_LAUNCH 0
//delay in minutes to wait before starting
const int TIME_DELAY = 1;
//------------------------------------------------------------------------------

//convert to millsecs
long MSTime = TIME_DELAY*1000;
SdFat sd;
SdFile logFile;
Adafruit_LSM9DS0 lsm = Adafruit_LSM9DS0(1000);  // Use I2C, ID #1000
#define LSM9DS0_XM_CS 10
#define LSM9DS0_GYRO_CS 9
#define LSM9DS0_SCLK 13
#define LSM9DS0_MISO 12
#define LSM9DS0_MOSI 11
//setup for temperature sensors
int resolution =9;
unsigned long lastTempRequest = 0;
int  delayInMillis = 0;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);
DeviceAddress tempDeviceAddress;

int buttonState = 0;

uint32_t logTime;
void setupSensors() {
  // accelerometer range
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_6G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_8G);
  lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_16G);

  //magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS0_MAGGAIN_2GAUSS);
  //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_12GAUSS);

  //gyroscope
  //lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_500DPS);
  lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_2000DPS);
  //pull up our first temp reading
  tempSensor.begin();
  tempSensor.getAddress(tempDeviceAddress, 0);
  tempSensor.setResolution(tempDeviceAddress, resolution);
  tempSensor.setWaitForConversion(false);
  tempSensor.requestTemperatures();
  delayInMillis = 750 / (1 << (12 - resolution));
  lastTempRequest = millis();
}//setupSensors

void lightSetup() {
  Adafruit_NeoPixel eggLight = Adafruit_NeoPixel(12, LIGHT_PIN, NEO_RGBW + NEO_KHZ800);
  eggLight.begin();
  //set each pixel to half brightness
  for(int n=0;n<12;n++) {
  eggLight.setPixelColor(n, 0, 0, 0, 50);
  }
  eggLight.show();
}//lightSetup

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {}
  pinMode(BUTTON_PIN,INPUT);
  pinMode(ERROR_LED_PIN, OUTPUT);
  digitalWrite(ERROR_LED_PIN, HIGH);
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
  lightSetup();
  digitalWrite(ERROR_LED_PIN, LOW);
  while(!buttonState==HIGH) {
    //Wait and check button
    buttonState = digitalRead(BUTTON_PIN);
  }
  digitalWrite(ERROR_LED_PIN, HIGH);
  #if START_ON_LAUNCH
  Serial.println(F("Waiting for launch"));
  sensors_event_t accel, mag, gyro, temp;
  while((accel.acceleration.z-9.81)<0) lsm.getEvent(&accel, &mag, &gyro, &temp);
  #else
  Serial.println(F("Timed launch"));
  delay(MSTime);
  #endif
  digitalWrite(ERROR_LED_PIN, LOW);

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
   if (millis() - lastTempRequest >= delayInMillis) {
      logFile.print((double) tempSensor.getTempCByIndex(0));
      tempSensor.setResolution(tempDeviceAddress, resolution);
      tempSensor.requestTemperatures();
    }
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

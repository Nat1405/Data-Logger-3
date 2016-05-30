/* Adaptaion of Bill Greiman's LowLatancyLogger SdFat example to make use of the LSM9DS0 and DS18B20
   Uses RAM buffer and SDFat's buffer to write data in 512 byte blocks, allowing higher writing speed
   Will write to a binary file, and as such must be converted using the binaryDecoder found in computer-side coverter
   If you need a Simpler logger, use the basic data-logger.
*/
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM9DS0.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>

#define LIGHT_PIN 2
//addresses and values for writing to the LSM9DS0
typedef enum {
  LSM9DS0_REGISTER_OUT_X_L_G           = 0x28,
  LSM9DS0_REGISTER_CTRL_REG4_G         = 0x23,
} lsm9ds0GyroRegisters_t;
typedef enum {
  LSM9DS0_REGISTER_TEMP_OUT_L_XM       = 0x05,
  LSM9DS0_REGISTER_OUT_X_L_M           = 0x08,
  LSM9DS0_REGISTER_CTRL_REG1_XM        = 0x20,
  LSM9DS0_REGISTER_OUT_X_L_A           = 0x28,
  LSM9DS0_REGISTER_CTRL_REG6_XM        = 0x25,
}lsm9ds0MagAccelRegisters_t;
typedef enum {
  LSM9DS0_ACCELDATARATE_1600HZ         = (0b1010 << 4)
}lm9ds0AccelDataRate_t;
typedef enum {
  LSM9DS0_MAGDATARATE_100HZ            = (0b101 << 2)
} lsm9ds0MagDataRate_t;
typedef enum {
  LSM9DS0_GYROSCALE_2000DPS            = (0b10 << 4)
} lsm9ds0GyroScale_t;


//----------------------------------------------
#include "UserDataType.h"
Adafruit_LSM9DS0 lsm = Adafruit_LSM9DS0();
//For Temperature sensor
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
#define TEMP_RESOLUTION 9 //can chose a resolution of 9,10,11,12 bits (more bits results in a longer read time)
int32_t delayInMillis =0;
uint64_t lastTempRequest = 0;
//----------------------------------------------
void acquireData(data_t* data) {
    data->time = micros();
    lsm.readAccel();
    data->adc[0] = lsm.accelData.x;
    data->adc[1] = lsm.accelData.y;
    data->adc[2] = lsm.accelData.z;
    lsm.readGyro();
    data->adc[3] = lsm.gyroData.x;
    data->adc[4] = lsm.gyroData.y;
    data->adc[5] = lsm.gyroData.z;
    data->adc[6] = 0;
    //only collect Temperature readings when the sensor has collected it
    if (millis() - lastTempRequest >= 1000) {
    data->adc[6] = sensors.getTempCByIndex(0);
    Serial.println(F("got temp"));
    sensors.requestTemperatures(); //get a new temp
    lastTempRequest = millis();
    }

}//acquireData
//-------------------------------------------
//----------------------------------------------
//set up an object to control the light sensor for the eggs

//setup the light ring for the eggs
void lightSetup() {
  Adafruit_NeoPixel eggLight = Adafruit_NeoPixel(12, LIGHT_PIN, NEO_RGBW + NEO_KHZ800);
  eggLight.begin();
  //set each pixel to half brightness
  for(int n=0;n<12;n++) {
  eggLight.setPixelColor(n, 0, 0, 0, 50);
  }
  eggLight.show();
}//lightSetup

//Interval between data records in microseconds.
const uint32_t LOG_INTERVAL_USEC = 6000;
// SD chip select pin.
const uint8_t SD_CS_PIN = SS;

// Maximum file size in blocks.
// The program creates a contiguous file with FILE_BLOCK_COUNT 512 byte blocks.
// This file is flash erased using special SD commands.  The file will be
// truncated if logging is stopped early.
const uint32_t FILE_BLOCK_COUNT = 256000;

// log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "data"

//select number of buffer blocks based on end address of ram uno has 1

const uint8_t BUFFER_BLOCK_COUNT = 1;
#define TMP_FILE_NAME "tmp_log.bin"

// Size of file base name.  Must not be larger than six.
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;

SdFat sd;

SdBaseFile binFile;

char binName[13] = FILE_BASE_NAME "00.bin";

// Number of data records in a block.
const uint16_t DATA_DIM = (512 - 4)/sizeof(data_t);

//Compute fill so block size is 512 bytes.  FILL_DIM may be zero.
const uint16_t FILL_DIM = 512 - 4 - DATA_DIM*sizeof(data_t);
#pragma pack(1)
struct block_t {
  uint16_t count;
  uint16_t overrun;
  data_t data[DATA_DIM];
  uint8_t fill[FILL_DIM];
};

const uint8_t QUEUE_DIM = BUFFER_BLOCK_COUNT + 2;

block_t* emptyQueue[QUEUE_DIM];
uint8_t emptyHead;
uint8_t emptyTail;

block_t* fullQueue[QUEUE_DIM];
uint8_t fullHead;
uint8_t fullTail;

// Advance queue index.
inline uint8_t queueNext(uint8_t ht) {
  return ht < (QUEUE_DIM - 1) ? ht + 1 : 0;
}
// Error messages stored in flash.
#define error(msg) errorFlash(F(msg))
//------------------------------------------------------------------------------
void errorFlash(String msg) {
  Serial.println(msg);
}
//--------------------------------------------------------------------------
// log data
uint32_t const ERASE_SIZE = 262144L;
void logData() {
  uint32_t bgnBlock, endBlock;

  // Allocate extra buffer space.
  block_t block[BUFFER_BLOCK_COUNT];
  block_t* curBlock = 0;
  Serial.println();

  // Find unused file name.
  if (BASE_NAME_SIZE > 6) {
    error("FILE_BASE_NAME too long");
  }
  while (sd.exists(binName)) {
    if (binName[BASE_NAME_SIZE + 1] != '9') {
      binName[BASE_NAME_SIZE + 1]++;
    } else {
      binName[BASE_NAME_SIZE + 1] = '0';
      if (binName[BASE_NAME_SIZE] == '9') {
        error("Can't create file name");
      }
      binName[BASE_NAME_SIZE]++;
    }
  }
  // Delete old tmp file.
  if (sd.exists(TMP_FILE_NAME)) {
    Serial.println(F("Deleting tmp file"));
    if (!sd.remove(TMP_FILE_NAME)) {
      error("Can't remove tmp file");
    }
  }
  // Create new file.
  Serial.println(F("Creating new file"));
  binFile.close();
  if (!binFile.createContiguous(sd.vwd(),
                                TMP_FILE_NAME, 512 * FILE_BLOCK_COUNT)) {
    error("createContiguous failed");
  }
  // Get the address of the file on the SD.
  if (!binFile.contiguousRange(&bgnBlock, &endBlock)) {
    error("contiguousRange failed");
  }
  // Use SdFat's internal buffer.
  uint8_t* cache = (uint8_t*)sd.vol()->cacheClear();
  if (cache == 0) {
    error("cacheClear failed");
  }

  // Flash erase all data in the file.
  Serial.println(F("Erasing all data"));
  uint32_t bgnErase = bgnBlock;
  uint32_t endErase;
  while (bgnErase < endBlock) {
    endErase = bgnErase + ERASE_SIZE;
    if (endErase > endBlock) {
      endErase = endBlock;
    }
    if (!sd.card()->erase(bgnErase, endErase)) {
      error("erase failed");
    }
    bgnErase = endErase + 1;
  }
  // Start a multiple block write.
  if (!sd.card()->writeStart(bgnBlock, FILE_BLOCK_COUNT)) {
    error("writeBegin failed");
  }
  // Initialize queues.
  emptyHead = emptyTail = 0;
  fullHead = fullTail = 0;

  // Use SdFat buffer for one block.
  emptyQueue[emptyHead] = (block_t*)cache;
  emptyHead = queueNext(emptyHead);

  // Put rest of buffers in the empty queue.
  for (uint8_t i = 0; i < BUFFER_BLOCK_COUNT; i++) {
    emptyQueue[emptyHead] = &block[i];
    emptyHead = queueNext(emptyHead);
  }
  Serial.println(F("Logging - type any character to stop"));
  // Wait for Serial Idle.
  Serial.flush();
  delay(10);
  uint32_t bn = 0;
  uint32_t t0 = millis();
  uint32_t t1 = t0;
  uint32_t overrun = 0;
  uint32_t overrunTotal = 0;
  uint32_t count = 0;
  uint32_t maxLatency = 0;
  int32_t diff;
  // Start at a multiple of interval.
  uint32_t logTime = micros()/LOG_INTERVAL_USEC + 1;
  logTime *= LOG_INTERVAL_USEC;
  bool closeFile = false;
  while (1) {
    // Time for next data record.
    logTime += LOG_INTERVAL_USEC;
    if (Serial.available()) {
      closeFile = true;
    }

    if (closeFile) {
      if (curBlock != 0 && curBlock->count >= 0) {
        // Put buffer in full queue.
        fullQueue[fullHead] = curBlock;
        fullHead = queueNext(fullHead);
        curBlock = 0;
      }
    } else {
      if (curBlock == 0 && emptyTail != emptyHead) {
        curBlock = emptyQueue[emptyTail];
        emptyTail = queueNext(emptyTail);
        curBlock->count = 0;
        curBlock->overrun = overrun;
        overrun = 0;
      }
      do {
        diff = logTime - micros();
      } while(diff > 0);
      if (diff < -10) {
        error("LOG_INTERVAL_USEC too small");
      }
      if (curBlock == 0) {
        overrun++;
      } else {
        acquireData(&curBlock->data[curBlock->count++]);
        //if we reach max number of logs per buffer block move in queue
        if (curBlock->count == DATA_DIM) {
          fullQueue[fullHead] = curBlock;
          fullHead = queueNext(fullHead);
          curBlock = 0;
        }
      }
    }

    if (fullHead == fullTail) {
      // Exit loop if done.
      if (closeFile) {
        break;
      }
    } else if (!sd.card()->isBusy()) {
      // Get address of block to write.
      block_t* pBlock = fullQueue[fullTail];
      fullTail = queueNext(fullTail);
      // Write block to SD.
      uint32_t usec = micros();
      if (!sd.card()->writeData((uint8_t*)pBlock)) {
        error("write data failed");
      }
      usec = micros() - usec;
      t1 = millis();
      if (usec > maxLatency) {
        maxLatency = usec;
      }
      count += pBlock->count;

      // Add overruns and possibly light LED.
      if (pBlock->overrun) {
        overrunTotal += pBlock->overrun;
      }
      // Move block to empty queue.
      emptyQueue[emptyHead] = pBlock;
      emptyHead = queueNext(emptyHead);
      bn++;
      if (bn == FILE_BLOCK_COUNT) {
        // File full so stop
        break;
      }
    }
  }
  if (!sd.card()->writeStop()) {
    error("writeStop failed");
  }
  // Truncate file if recording stopped early.
  if (bn != FILE_BLOCK_COUNT) {
    Serial.println(F("Truncating file"));
    if (!binFile.truncate(512L * bn)) {
      Serial.println(F("Catch truncate error"));
    }
  }
  if (!binFile.rename(sd.vwd(), binName)) {
    Serial.println(F("Catch rename error"));
  }
  Serial.print(F("Samples/sec: "));
  Serial.println((1000.0)*count/(t1-t0));
  Serial.println(F("Done"));
}//logData
//------------------------------------------------------------------------------

void setup(void) {
  Serial.begin(115200);
  while (!Serial) {}
  //turn on the light ring
  lightSetup();
  //setup sensors and write to set sensetivity
  lsm.begin();
  //accelation
  lsm.write8(XMTYPE, LSM9DS0_REGISTER_CTRL_REG1_XM, LSM9DS0_ACCELDATARATE_1600HZ);
  //gyro
  lsm.write8(XMTYPE, LSM9DS0_REGISTER_CTRL_REG4_G, LSM9DS0_GYROSCALE_2000DPS);
  //Temperature
  Serial.print(F("Record size"));
  Serial.println(sizeof(data_t));
  Serial.print(F("Records/block: "));
  Serial.println(DATA_DIM);
  if (sizeof(block_t) != 512) {
    error("Invalid block size");
  }
  // initialize file system.
  if (!sd.begin(SD_CS_PIN, SPI_FULL_SPEED)) {
    sd.initErrorPrint();
  }
  //start the Temperature sensor
  sensors.begin();
  sensors.getAddress(tempDeviceAddress, 0);
  sensors.setResolution(tempDeviceAddress, TEMP_RESOLUTION);
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
  //delayInMillis = 750 / (1 << (12 - TEMP_RESOLUTION));
  lastTempRequest = millis();
}//setup

void loop(void) {
  // discard any input
  while (Serial.read() >= 0) {}
  Serial.println();
  Serial.println(F("type:"));
  Serial.println(F("r - record data"));
  while(!Serial.available()) {}
  char c = tolower(Serial.read());

  // Discard extra Serial data.
  do {
    delay(10);
  } while (Serial.read() >= 0);
   if (c == 'r') {
    logData();
  } else {
    Serial.println(F("Invalid entry"));
  }
}//loop

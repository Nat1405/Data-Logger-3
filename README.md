# UVR Data-Logger
Payload Data Logger for [UVic Rocketry Team](http://rocketry.engr.uvic.ca/).
Records Temperature, accleration (xyz),magnetic field(xyz),and rotation(xyz).
Currently Logs to .csv file, at a user-specified speed.
## Arduino Pin Setup
### UNO
![Uno Pin Setup](https://vgy.me/Xjas9s.jpg)

|UNO PIN|LSM9DS0  |MicroSD  |
|-------|:-------:|:-------:|
|A5     |SCL      |   -     |
|A4     |SDA      |   -     |
|5V     |Vin      |   -     |
|3.3V   |    -    |   3v    |
|GND    |GND      |   GND   |
|13     |    -    |   CLK   |
|12     |    -    |   DO    |
|11     |    -    |   DI    |
|10     |    -    |   CS    |
##Setup
* Install required libraries, time, sdFat, Adafruit unified sensor, and LSM9DS0 through library mangager
* Upload and run sdFat's [benchmark](https://github.com/greiman/SdFat/blob/master/SdFat/examples/bench/bench.ino) to dertermine card quality 
* Use this to select run speed in ms for sample rate, as file corruption can occur if you try to write to the card too frequently
* If your card is a lower speed one, consider choosing the normal logger for ease of use, otherwise use the binary logger

##Use

###Data-Logger.ino Use
* Will run upon serial connection, and will finish and upon keypress through serial monitor
* If RUN_ON_LAUNCH is enabled will start connection on detection of z-accleceration increase. (currently ~+1 m/s^2)
* If board loses power or is disconnected from serial during run, file should still be synced.
* Easy way to check if writing is that red light on SD board should flash with every run.
* There will be some varience between time steps of about +3-4ms
* If there are multiple tables in csv file, lowest one will contain newest data

###Binary-Data-Logger Use
* Will run on keypress, and uses arduino's onboard RAM buffers to save to the card
* If board loses power, the data will be saved to the "tmp_log.bin"
* Otherwise it will save to "logXX.bin" depending on how many files already exist on it
* To convert, simply compile the binaryDecoder.cpp to C++11 standard, and place the binary file in the same folder and run.

##Thanks
Thanks to [@fdpierson](https://github.com/fdpierson) for helping with binary file reading

![martlet-ascii](https://vgy.me/AaAJJm.png)

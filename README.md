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
* Select variables for breadboard, header inclusion and sensor options 
* If you wish, change filename/type on line 72

##Use
* Will run upon serial connection, and will finish and upon keypress through serial monitor
* If board loses power or is disconnected from serial during run, file should still be synced.
* Easy way to check if writing is that red light on SD board should flash with every run.
* There will be some varience between time steps of about +3-4ms
* If there are multiple tables in csv file, lowest one will contain newest data

##Planned Features
* Output to binary file to increase speeds if necessary, to be convereted computer-side.
* Computer-Side java program to check over files to summarize data and any errors/skips that may have occured in flight
* Start write on launch detection, end after landing.
* RTC support


![martlet-ascii](https://vgy.me/AaAJJm.png)

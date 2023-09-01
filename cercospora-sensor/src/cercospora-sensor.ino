/* Project: Sugarbeet Cercospora Sensor
 * Description: Code for Particle Boron with 1 sht31 units for temp and rh%
   readings in sugarbeet field.  GPS is included. Data are streamed to Ubidots platform.
 * Author: Emmanuel Deleon, AJ Brown
 * Sponsor: Western Sugar
 * Project Start Date:1 June 2022
 * Last update: 11 August 2023
 */

// v12.00  Added takeMeasurementsNow function to allow user to take readings when device is on without going to sleep


//PRODUCT_ID(PLATFORM_ID);
PRODUCT_VERSION(12); // increment each time you upload firmware to the console
/*
Cercospora leaf spot monitor - Version 12

-Uses SHT31 sensor. 

-30 min. readings 

-Optimized solar charging

-GPS OFF and commented out ENTIRELY to alleviate any possibility of power drain

-Sleep between readings (27 min.)

-Stream data to Ubidots

-Includes "TakeMeasurement" function to get readings immediately while device is awake
*/

// Libs

//#include <Adafruit_GPS.h>
#include <adafruit-sht31.h>
#include <RunningAverage.h>

// Definitions
#define subsamples 5
//#define GPSSerial Serial1
#define DELAY_BEFORE_REBOOT 2000

// GPS configuration
//#define GPSSerial Serial1
    // Periodic (sleep) mode command for GPS module
    // 2 is periodic standby mode
    // 60000 is the active time in milliseconds (1min; time to acquire signal)
    // 43140000 is sleep time in milliseconds (719 min; 11hr 59 min)
    // 1740000 is sleep time in milliseconds (29 min)
    // Leave the mode alone. Change the times to fit your needs
    // REMEMBER to CHANGE the CHECKSUM every time you change timings
    // A checksum is a sequence of numbers and letters used to check data for errors
    // Checksum calculator is here:
    // http://www.hhhh.org/wiml/proj/nmeaxor.html
    // CURRENT ISSUE: gps only awake for 1 min from when device turned on; this may not coincide with first reading, making the timing continually off
    //                future work should create an interrupt that turns on boron to read GPS then goes back to sleep.
    //                see: https://forums.adafruit.com/viewtopic.php?f=8&t=54272&hilit=+turtle
    //                see: https://forums.adafruit.com/viewtopic.php?f=31&p=547903
    //                see: https://forums.adafruit.com/viewtopic.php?f=31&p=256775
    // GPS manual: https://cdn.sparkfun.com/assets/parts/1/2/2/8/0/PMTK_Packet_User_Manual.pdf
//#define PMTK_PERIODIC_MODE "$PMTK225,2,60000,43140000,0,0*1D"   
//#define PMTK_PERIODIC_MODE "$PMTK225,2,60000,1740000,0,0*2D"
// permanent standby mode to save power:
//#define PMTK_CMD_STANDBY_MODE "$PMTK161,0*28"

// GPS configuration
/*
float lat;
float lng;
float alt;
Adafruit_GPS GPS(&GPSSerial);
*/

// initialize sht31 T and RH sensor
Adafruit_SHT31 sht31 = Adafruit_SHT31();
  // Configure temp and RH vars
float temp;
float hum;

// Configure running average variables for 3 sensors
RunningAverage myRA1(subsamples);
RunningAverage myRA2(subsamples);

// Boron diagnostics
FuelGauge fuel;
float batteryVoltage;
int stateOfCharge;
int rat;
float strengthPercentage = 0.0;
float qualityPercentage = 0.0;

// Timers for loops
int Time_old;

// Remote system reset variables
unsigned int rebootDelayMillis = DELAY_BEFORE_REBOOT;
unsigned long rebootSync = millis();
bool resetFlag = false;

// Remote takeMeasurementsNow variable
bool resetFlagReadings = false;

// Variables Related To Particle Mobile Application Reporting and Webhook
char SignalString[64];
char batteryString[16];
char tempString[16];   // Temp Sensor in Celcius
char humString[16];   // Relative Humidity %
char positionString[64];    // GPS location in lat/long decimal degrees

// Sleep function: uncomment to add
SystemSleepConfiguration config;

// Power Configuration for Solar Charging
SystemPowerConfiguration conf;

void setup() {
    // Serial print & publish program device info upon setup initiation
    Serial.begin(9600);
    Particle.publish("Cercospora Sensor Module Initiated: "+Time.timeStr());
    Particle.publish("Settings: Readings; 30 min, Sleep; 27 min, GPS; off");
    Serial.println("Pile Temperature Sensor Module Initiated: "+Time.timeStr());
    Serial.println(">>> GPS enabled: 12 hr readings");
    Serial.println(">>> Temp (MCP9808) enabled: 2 hr readings");
    Serial.println(">>> Sleep mode: Yes");

    // Sensor Setup
     // turn on Sensors
    Wire.begin();
    sht31.begin(0x44);
    myRA1.clear();
    myRA2.clear();
     // Particle variables that enable monitoring via mobile app
    Particle.variable("BattVolt", batteryString);
    Particle.variable("SensorT", tempString);
    Particle.variable("SensorRH", humString);
    Particle.variable("Signal", SignalString);
    Particle.variable("Position", positionString);
        
    //  Remote Reset Function Setup
    Particle.function("reset", cloudResetFunction);
    
    //  Take Measurements Function Setup
    Particle.function("TakeMeasurementsNow", takeMeasurementNow);

    // Sleep Setup: uncomment to add
    config.mode(SystemSleepMode::STOP)
          .duration(27min);   // set sleep duration in minutes here

    //GPS Setup
    /*
     // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
    GPS.begin(9600);
     // Turn on RMC (recommended minimum) and GGA (fix data) including altitude
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
     // uncomment this line to turn on only the "minimum recommended" data
     // GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
     // Set the update rate
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
     // Request updates on antenna status, comment out to keep quiet
    GPS.sendCommand(PGCMD_ANTENNA);
     // Ask for firmware version
    GPSSerial.println(PMTK_Q_RELEASE);
     // Set GPS into Periodic Mode
    //GPS.sendCommand(PMTK_PERIODIC_MODE);
     // give it some time to initiate
    //delay(1000);
     // Set GPS into permanent standby as needed
    GPS.sendCommand(PMTK_CMD_STANDBY_MODE);
    */
}

void loop(){
    /*
	// Read GPS (how does this behave when GPS is in stanby between readings?)
	char c = GPS.read();
	  // check for new NMEA string
	
    #if (GPS.newNMEAreceived()) {
    #    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
    #        return; // we can fail to parse a sentence in which case we should just wait for another
    #}
    */
    
	// Collect sensor readings and send payload
	    // Samples every 30 min; change the "30" to change sample interval in min (0 - 59)
    if(Time.minute() % 30 == 0 && Time_old != Time.minute()){ 
        getSignalStrength(); // signal diagnostics
        takeMeasurements(); // read temperatures
        //getPosition(); // collect location
        sendData(); // send payload
        Time_old = Time.minute(); // resetting timer
        delay(9000); // needed to allow time for data to send prior to sleep initiation

        // Sleep
        System.sleep(config); // set device to sleep
    }
    
    //  Remote Reset Function
    if ((resetFlag) && (millis() - rebootSync >=  rebootDelayMillis)) {
        // do things here before reset if necessary and then push the button
        Particle.publish("Debug", "Remote Reset Initiated", 300, PRIVATE);
        System.reset();
    }    
    //  Take Measurements Now Function
    if (resetFlagReadings) {
        Particle.publish("User Input Recieved: Taking Sensor Measurements Now: "+Time.timeStr());
        getSignalStrength(); // signal diagnostics
        takeMeasurements(); // read temperatures
        //getPosition(); // collect location
        sendData(); // send payload
        resetFlagReadings = false;
    }
}

void sendData() { 
    // send data to particle cloud
      // defining what timestamp_second is
    unsigned long timestamp_seconds = Time.now(); 
      // create a message with 512 charaters. 
      // note that says this needs be declared locally (in fxn), not globally --AJB
    char msg[512];  
      //note: removing variables from msg still sends to ubidots, ie it's okay to not have new position variable; it will still send other variables

      // to publish data to cloud it needs to be a string format.
      // New msg w/ position
    //snprintf(msg, sizeof(msg) - 1, "{\"T\":%4.1f, \"RH\":%4.1f, \"battery\":%4.1f, \"charge\":%i, \"Signal\":%4.1f, \"timestamp\":%lu000, \"alt\":%.6f, \"lat\":%.6f, \"lng\":%.6f, \"position\":%s}", temp, hum, batteryVoltage, stateOfCharge, strengthPercentage, timestamp_seconds, alt, lat, lng, positionString);
      // Old msg w/o position; uncomment to use
    snprintf(msg, sizeof(msg) - 1, "{\"T\":%4.1f, \"RH\":%4.1f, \"battery\":%4.1f, \"charge\":%i, \"Signal\":%4.1f, \"timestamp\":%lu000}", temp, hum, batteryVoltage, stateOfCharge, strengthPercentage, timestamp_seconds); //to publish data to cloud it needs to be a string format.
    
      //send data to cloud
    Particle.publish("AWQP_Cercospora", msg, PRIVATE);
    Serial.println("Particle Published!");
    //Serial.println(msg);
  }

bool takeMeasurements() {

  if (Cellular.ready()){
  // Get battery voltage level and state  of charge
  batteryVoltage = fuel.getVCell();  // = 4.0 Voltage level of battery
  snprintf(batteryString, sizeof(batteryString), "%4.1f V", batteryVoltage);
  stateOfCharge = int(fuel.getSoC());

    for (byte i=0; i<=subsamples; i++)   {
      temp = sht31.readTemperature();
      hum = sht31.readHumidity();
      delay(30);
      myRA1.addValue(temp);
      myRA2.addValue(hum);
    }
    temp=myRA1.getAverage();
    hum=myRA2.getAverage();


    snprintf(tempString,sizeof(tempString), "%4.1f C", temp);
    snprintf(humString,sizeof(humString), "%4.1f%%", hum);
    myRA1.clear();
    myRA2.clear();

  }
  return 1;
}
/*
int getPosition() {
    // Indicate if GPS fix is aquired
    Particle.publish("GPS fix status (1=True,0=False): "+String(GPS.fix));
    Serial.println("GPS fix status (1=True,0=False): "+String(GPS.fix));
    lat = GPS.latitudeDegrees;
    lng = GPS.longitudeDegrees;
    alt = GPS.altitude;
    
    // Publish the physical location of the device
    Particle.publish("Location: ", String(GPS.latitudeDegrees) + ", " + String(GPS.longitudeDegrees)  + ", " + String(GPS.altitude));
    Serial.println((String)"Location: "+ GPS.latitudeDegrees + ", " + GPS.longitudeDegrees  + ", " + GPS.altitude);
    
    // build payload variable
      // format from ubidots: https://help.ubidots.com/en/articles/901448-change-default-device-location
      // {"position": {"value":1, "context":{"lat":25.7742700, "lng": -80.1936600}}
    snprintf(positionString,sizeof(positionString), "{\"value\":%.6f%, \"context\":{\"lat\":%.6f%, \"lng\": %.6f%}}",alt, lat, lng);
    return 1;
}
*/

void getSignalStrength() {
    CellularSignal sig = Cellular.RSSI();
    rat = sig.getAccessTechnology();
    strengthPercentage = sig.getStrength();
    qualityPercentage = sig.getQuality();
    snprintf(SignalString,sizeof(SignalString), "S:%2.0f%%, Q:%2.0f%% ", strengthPercentage, qualityPercentage);
}

int cloudResetFunction(String command) {//  Remote Reset Function
    resetFlag = true;
    rebootSync = millis();
    return 1;
    // You would call the function by typing “true” in the particle console
}

int takeMeasurementNow(String command) {
    resetFlagReadings = true;
    return 1;
}

void batt_settings() {
    conf.powerSourceMaxCurrent(1024) // default is 900mA this let's me charge faster
    .powerSourceMinVoltage(5080) // Minimum allowed plane voltage = 5.080v
    .batteryChargeCurrent(1024) // default is 2048mA (011000) = 512mA+1024mA+512mA)
    .batteryChargeVoltage(4112); // default is 4.112V termination voltage
    //.feature(SystemPowerFeature::USE_VIN_SETTINGS_WITH_USB_HOST);
    System.setPowerConfiguration(conf); // retains these settings
}


/* Project: Sugarbeet Pile Temperature Sensor
 * Description: Code for sugarbeet pile temperature monitoring using particle boron
   microcontroller. Readings are every 4 hours with boron sleep between
   readings. Data are sent via cell data to particle cloud, then webhook to Ubidots
   data service.
 * Author: Emmanuel Deleon, AJ Brown
 * Sponsor: Western Sugar
 * Project Start Date:1 July 2021
 * Last update: 9 Oct 2023
 */

// PT code version 8, Firmware version 14
// uses single SHT31 sensor instead of 3, MCP9808 sensors
// for testing only! (5 minute readings with no sleep)


//PRODUCT_ID(PLATFORM_ID);
PRODUCT_VERSION(15); // increment each time you upload firmware to the console
/*
Pile Temperature code version 9, Firmware version 15
-for use in active, deployed sensors

-Uses 1x SHT31 sensor instead of 3x MCP9808 sensors. 

-2 hour readings

-Optimized solar charging for 2500 mAh, 3.7V, LiPo battery

-GPS REMOVED

-Sleeps between readings

-Stream data to Ubidots

-Includes "TakeMeasurement" function to get readings immediately while device is awake

-Includes "TakeReadingsNow" function to remotely trigger data collection
*/

// Libs

//#include <Adafruit_GPS.h>
#include <adafruit-sht31.h>
#include <RunningAverage.h>

// Definitions
#define subsamples 5
//#define GPSSerial Serial1
#define DELAY_BEFORE_REBOOT 2000


// initialize sht31 T and RH sensor
Adafruit_SHT31 sht31 = Adafruit_SHT31();
  // Configure temp and RH vars
float temp;
float hum;
float tempFVal1; // for use of SHT31 as pile temp sensor

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
    Particle.publish("Pile Temperature Sensor Module Initiated: SHT31"+Time.timeStr());
    Particle.publish("Settings: Sensor; SHT31, Readings; 5 min, Sleep; NONE, GPS; off");
    Serial.println(">>> Pile Temperature Sensor Module Initiated: "+Time.timeStr());
    Serial.println(">>> Settings: Sensor; SHT31, Readings; 5 min, Sleep; NONE, GPS; off");

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
          .duration(117min);   // set sleep duration in minutes here, wake up 3 minutes before readings

}

void loop(){
    
	// Collect sensor readings and send payload
        // Samples every 2 hr. change the "2" to change sample interval in hours (0 - 23)
    if(Time.hour() % 2 == 0 && Time_old != Time.hour()){ 
        getSignalStrength(); // signal diagnostics
        takeMeasurements(); // read temperatures
        sendData(); // send payload
        Time_old = Time.hour(); // resetting timer
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
        Particle.publish("Input Recieved: Taking Measurements. "+Time.timeStr());
        getSignalStrength(); // signal diagnostics
        takeMeasurements(); // read temperatures
        sendData(); // send payload
        Particle.publish("Measurements taken and payload sent. "+Time.timeStr());
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
      //note: removing variables from msg still sends to ubidots, ie it's okay to not have the new "position" variable; it will still send other variables as possible

      // to publish data to cloud it needs to be a string format.
      // Payload for SHT31 use as a Pile Temp sensor (instead of MCP9808 sensors); note that same temp sent for F1-F3 values as a "trick" to Ubidots
    snprintf(msg, sizeof(msg) - 1, "{\"battery\":%4.1f, \"TempF1\" :%f, \"TempF2\" :%f, \"TempF3\":%f, \"charge\":%i, \"Signal\": %4.1f,\"timestamp\":%lu000}", batteryVoltage, tempFVal1, tempFVal1, tempFVal1, stateOfCharge, strengthPercentage, timestamp_seconds);
      // New msg w/ position
    //snprintf(msg, sizeof(msg) - 1, "{\"T\":%4.1f, \"RH\":%4.1f, \"battery\":%4.1f, \"charge\":%i, \"Signal\":%4.1f, \"timestamp\":%lu000, \"alt\":%.6f, \"lat\":%.6f, \"lng\":%.6f, \"position\":%s}", temp, hum, batteryVoltage, stateOfCharge, strengthPercentage, timestamp_seconds, alt, lat, lng, positionString);
      // Old msg w/o position; uncomment to use
    //snprintf(msg, sizeof(msg) - 1, "{\"T\":%4.1f, \"RH\":%4.1f, \"battery\":%4.1f, \"charge\":%i, \"Signal\":%4.1f, \"timestamp\":%lu000}", temp, hum, batteryVoltage, stateOfCharge, strengthPercentage, timestamp_seconds);
    
      //send data to cloud
    Particle.publish("AWQP_Pile_Temp", msg, PRIVATE); // changed for Pile Temp with SHT31
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
    temp=myRA1.getAverage(); //temp in Celcius
    hum=myRA2.getAverage();
    // added below lines to easily send SHT31 data as pile-temp-like payload to "trick" Ubidots
    tempFVal1 = ((9.0/5.0)*temp + 32); //conversion to F and name vaule tempFVal


    snprintf(tempString,sizeof(tempString), "%4.1f C", temp);
    snprintf(humString,sizeof(humString), "%4.1f%%", hum);
    myRA1.clear();
    myRA2.clear();

  }
  return 1;
}

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
    // Optimized for smaller 2500 mAh LiPo battery: https://acrobat.adobe.com/id/urn:aaid:sc:US:15adc4fb-8183-4845-b129-5039b2164b1e
    conf.powerSourceMaxCurrent(1500) // for 2500 mAh bat., this is 1 C5A or 1.0*2500 = 2500 mA; default is 900 mA; due to boron components, real max is ~1500
    .powerSourceMinVoltage(3750) // Set minimum voltage required for VIN to be used = 3.75V; default is 3880
    .batteryChargeCurrent(500) // for 2500 mAh bat., this is 0.2 C5A or 0.2*2500 = 500 mA; default is 896 mA
    .batteryChargeVoltage(4200); // set at 4.2V for 2500 mAh bat.; default is 4.112V termination voltage
    //.feature(SystemPowerFeature::USE_VIN_SETTINGS_WITH_USB_HOST);
    System.setPowerConfiguration(conf); // retains these settings
}


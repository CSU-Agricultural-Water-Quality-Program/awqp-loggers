/* Project: Sugarbeet Monitoring Sensor
 * Description: Firmware for particle boron device that is connected
 to and SHT3X sensor that can stream temp. and relative humidity values
 via cellular connection for Cercospora Leaf Spot and sugarbeet storage
 pile temperature monitoring.
 * Author: Emmanuel Deleon, AJ Brown
 * Sponsor: Western Sugar
 * Project Start Date:1 July 2021
 * Last update: 5 February 2024
 */

// increment each time you upload firmware to the particle console
PRODUCT_VERSION(18);
/*
__**AWQP Logger SHT31 sensor V1, Firmware version 16**__
Firmware for particle boron devices to use a single, SHT3X sensor
that reads temp. and RH% at 30 min. intervals and sends to the
cloud.  This version will be the single firmware version moving forward
after the decision that no sensor swapping was needed between
CLS and PT monitoring seasons, thus alleviating the need to change
firmwares to accomodate.

**Firmware Attributes:**
-for use in active sensors (i.e., not testing)

-Uses 1x SHT3X sensor (firmware works on SHT30, 31, & 35 units)

-30 minute readings

-Sleeps between readings (27 min.)

-Stream data to AWQP Ubidots and WS Azure IoT Hub Accounts

-Includes "TakeMeasurement" function for manual data collection while device is awake

-Includes "reset" function to remotely reset device when awake

-No solar panel

-GPS REMOVED
*/

// Libs

//#include <Adafruit_GPS.h>
#include <adafruit-sht31.h>
#include <RunningAverage.h>

// Definitions
#define subsamples 5 // # of readings to average for SHT3X sensor output
#define DELAY_BEFORE_REBOOT 2000


// initialize sht31 T and RH sensor
Adafruit_SHT31 sht31 = Adafruit_SHT31();
  // Configure temp and RH vars
float temp;
float hum;

// Configure running average variables
RunningAverage myRA1(subsamples);
RunningAverage myRA2(subsamples);

// Boron diagnostics
FuelGauge fuel;
float batteryVoltage;
int stateOfCharge;
int rat;
float strengthPercentage = 0.0;
float qualityPercentage = 0.0;

// Timers for loop()
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
    Particle.publish("AWQP Sensor Initiated: SHT3X, "+Time.timeStr());
    Particle.publish("Settings: Readings; 30 min, Sleep; Yes");
    Serial.println(">>> AWQP Sensor Initiated: SHT3X, "+Time.timeStr());
    Serial.println(">>> Settings: Readings; 30 min, Sleep; Yes");

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
          .duration(27min);   // set sleep duration in minutes here, wake up 3 minutes before readings
          
    //  set D7 pin to OUTPUT for LED blink upon measurement
    pinMode(D7, OUTPUT);

}

void loop(){
    
	// Collect sensor readings and send payload
        // Samples every 2 hr. change the "2" to change sample interval in hours (0 - 23)
    if(Time.minute() % 30 == 0 && Time_old != Time.minute()){  
        getSignalStrength(); // signal diagnostics
        takeMeasurements(); // read temperatures
        sendData(); // send payload
        digitalWrite(D7, HIGH); // turn on LED
        Time_old = Time.minute(); // resetting timer
        delay(8000); // needed to allow time for data to send prior to sleep initiation
        digitalWrite(D7, LOW); // turn off LED
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
        digitalWrite(D7, HIGH); // turn on LED
        Particle.publish("Measurements taken and payload sent. "+Time.timeStr());
        delay(3000); // needed to allow time for data to send
        resetFlagReadings = false; // reset function to use again
        digitalWrite(D7, LOW); // turn off LED
    }
}

void sendData() { 
    // send data to particle cloud
        // defining what timestamp_seconds is
    unsigned long timestamp_seconds = Time.now(); 
    // create a message with 512 charaters. 
        // note that says this needs be declared locally (in fxn), not globally --AJB
    char msg[512];  
    // to publish data to cloud it needs to be a string format.
              //note: removing variables from msg still sends to ubidots, ie it's okay to not have new position variable; it will still send other variables
    snprintf(msg, sizeof(msg) - 1, "{\"T\":%4.1f, \"RH\":%4.1f, \"battery\":%4.1f, \"charge\":%i, \"Signal\":%4.1f, \"timestamp\":%lu000}", temp, hum, batteryVoltage, stateOfCharge, strengthPercentage, timestamp_seconds); //to publish data to cloud it needs to be a string format.
    
    //send data to cloud
    Particle.publish("AWQP_Cercospora", msg, PRIVATE); // Send payload to AWQP Ubidots
    Particle.publish("AWQP_Sensor_SHT", msg, PRIVATE); // Send payload to WS IoT Hub
    Serial.println("Particle Published!");
    //Serial.println(msg);
  }

bool takeMeasurements() {

  if (Cellular.ready()){
  // Get battery voltage level and state  of charge
  batteryVoltage = fuel.getVCell();  // = 4.0 Voltage level of battery
  snprintf(batteryString, sizeof(batteryString), "%4.1f V", batteryVoltage);
  stateOfCharge = int(fuel.getSoC());
  
  // take 30ms readings and average for robust data
    for (byte i=0; i<=subsamples; i++)   { 
      temp = sht31.readTemperature();
      hum = sht31.readHumidity();
      delay(30);
      myRA1.addValue(temp);
      myRA2.addValue(hum);
    }
    temp=myRA1.getAverage(); // temp in Celcius (average of 5 readings)
    hum=myRA2.getAverage(); // rh in % (average of 5 readings)

  // convert to string characters
    snprintf(tempString,sizeof(tempString), "%4.1f C", temp);
    snprintf(humString,sizeof(humString), "%4.1f%%", hum);
  
  // reset running average vars
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


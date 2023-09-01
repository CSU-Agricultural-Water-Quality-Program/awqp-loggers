/* Project: Sugarbeet Pile Temperature Sensor
 * Description: Code for Particle Boron with 3 MCP9809 units for simultanouse temp.
   readings at 3 depths.  Readings are every 4 hours with boron sleep between
   readings.  GPS will acquire location every 12 hours and enter standby.
   Data are sent via cell data to particle cloud, then webhook to Ubidots
   data service.
 * Author: Emmanuel Deleon, AJ Brown
 * Sponsor: Western Sugar
 * Project Start Date:1 July 2021
 * Last update: 1 Sep 2023
 */

// v7.00  removed GPS code


//PRODUCT_ID(PLATFORM_ID);
PRODUCT_VERSION(7); // increment each time you upload firmware to the console

// Libs
#include <MCP9808.h>
#include <RunningAverage.h>

// Definitions
#define subsamples 5
#define DELAY_BEFORE_REBOOT 2000

// Temp sensor addresses
MCP9808 mcp1 = MCP9808(0x18);
MCP9808 mcp2 = MCP9808(0x19);
MCP9808 mcp3 = MCP9808(0x1A);

// Configure running average variables for 3 sensors
RunningAverage myRA1(subsamples);
RunningAverage myRA2(subsamples);
RunningAverage myRA3(subsamples);

// Configure temp variables in C and F
float tempVal1;
float tempVal2;
float tempVal3;
float tempFVal1;
float tempFVal2;
float tempFVal3;

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

// Variables Related To Particle Mobile Application Reporting and Webhook
char SignalString[64];
char batteryString[16];
char tempFVal1String[16];   // Temp Sensor 1 in Farenheit
char tempFVal2String[16];   // Temp Sensor 2 ...
char tempFVal3String[16];   // Temp Sensor 3 ...

// Sleep function: uncomment to add
SystemSleepConfiguration config;

void setup() {
    // Serial print & publish program device info upon setup initiation
    Serial.begin(9600);
    Particle.publish("Pile Temperature Sensor Module Initiated: "+Time.timeStr());
    Particle.publish("Settings: Readings; 2 hr, Sleep; Yes, GPS; 12 hr");
    Serial.println("Pile Temperature Sensor Module Initiated: "+Time.timeStr());
    Serial.println(">>> GPS enabled: 12 hr readings");
    Serial.println(">>> Temp (MCP9808) enabled: 2 hr readings");
    Serial.println(">>> Sleep mode: Yes");

    // Sensor Setup
     // turn on Sensors
    mcp1.begin();
    mcp2.begin();
    mcp3.begin();
     // set resolution on sensors
    mcp1.setResolution(MCP9808_SLOWEST);
    mcp2.setResolution(MCP9808_SLOWEST);
    mcp3.setResolution(MCP9808_SLOWEST);
     // Particle variables that enable monitoring via mobile app
    Particle.variable("BattVolt", batteryString);
    Particle.variable("Signal", SignalString);
    Particle.variable("PT1", tempFVal1String);
    Particle.variable("PT2", tempFVal2String);
    Particle.variable("PT3", tempFVal3String);

    //  Remote Reset Function Setup
    Particle.function("reset", cloudResetFunction);

    // Sleep Setup: uncomment to add
    config.mode(SystemSleepMode::STOP)
          .duration(117min);   // set sleep duration in minutes here
}

void loop(){
	// Collect sensor readings and send payload
	    // Samples every 2 hr. change the "2" to change sample interval in min (0 - 23)
    if(Time.hour() % 2 == 0 && Time_old != Time.hour()){ 
        getSignalStrength(); // signal diagnostics
        takeMeasurements(); // read temperatures
        sendData(); // send payload
        Time_old = Time.minute(); // resetting timer
        delay(9000); // needed to allow time for data to send prior to sleep initiation

        // Sleep
        System.sleep(config); // set device to sleep
    }
    //  Remote Reset Function
    if ((resetFlag) && (millis() - rebootSync >=  rebootDelayMillis)) {
        // do things here  before reset and then push the button
        Particle.publish("Debug", "Remote Reset Initiated", 300, PRIVATE);
        System.reset();
    }
}

void sendData() { //send data to particle cloud
    unsigned long timestamp_seconds = Time.now(); //defining what timestamp_second is
    char msg[512];  //create a message with 512 charaters. I have a note that says this needs be declared locally, not globally --AJB
    //note: removing variables from msg still sends to ubidots, ie it's okay to not have new position variable; it will still send other variables

    // to publish data to cloud it needs to be a string format.
     // New msg w/ position
    //snprintf(msg, sizeof(msg) - 1, "{\"battery\":%4.1f, \"TempF1\":%f, \"TempF2\":%f, \"TempF3\":%f, \"charge\":%i, \"Signal\":%4.1f,\"timestamp\":%lu000,\"alt\":%.6f,\"lat\":%.6f,\"lng\":%.6f,\"position\":%s}", batteryVoltage, tempFVal1,tempFVal2,tempFVal3,stateOfCharge,strengthPercentage,timestamp_seconds,alt,lat,lng,positionString);
     // Old msg w/o position; uncomment to use
    snprintf(msg, sizeof(msg) - 1, "{\"battery\":%4.1f, \"TempF1\" :%f, \"TempF2\" :%f, \"TempF3\":%f, \"charge\":%i, \"Signal\": %4.1f,\"timestamp\":%lu000}", batteryVoltage, tempFVal1, tempFVal2, tempFVal3, stateOfCharge, strengthPercentage, timestamp_seconds); //to publish data to cloud it needs to be a string format.

    Particle.publish("AWQP_Pile_Temp", msg, PRIVATE); //send data to cloud
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
            tempVal1 = mcp1.getTemperature();  //read temp in C and name value tempVal
            tempFVal1 = ((9.0/5.0)*tempVal1 + 32); //conversion to F and name vaule tempFVal
            tempVal2 = mcp2.getTemperature();  //read temp in C and name value tempVal
            tempFVal2 = ((9.0/5.0)*tempVal2 + 32); //conversion to F and name vaule tempFVal
            tempVal3 = mcp3.getTemperature();  //read temp in C and name value tempVal
            tempFVal3 = ((9.0/5.0)*tempVal3 + 32); //conversion to F and name vaule tempFVal
            delay(30);
            myRA1.addValue(tempFVal1);
            myRA2.addValue(tempFVal2);
            myRA3.addValue(tempFVal3);
        }

        tempFVal1=myRA1.getAverage();
        tempFVal2=myRA2.getAverage();
        tempFVal3=myRA3.getAverage();

        snprintf(tempFVal1String,sizeof(tempFVal1String), "%4.1f F", tempFVal1);
        snprintf(tempFVal2String,sizeof(tempFVal2String), "%4.1f F", tempFVal2);
        snprintf(tempFVal3String,sizeof(tempFVal3String), "%4.1f F", tempFVal3);
        myRA1.clear();
        myRA2.clear();
        myRA3.clear();
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
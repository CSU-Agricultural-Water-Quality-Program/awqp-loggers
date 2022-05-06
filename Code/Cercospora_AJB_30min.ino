/*
AWQP Cercospora monitoring apparatus
Created by Emmanuel Deleon and A.J. Brown, May 3, 2022

Device that estimates sugar beet susceptibility to cercospora bacterial infection
by monitoring in-canopy temperature and relative humidity.

Particle pinout for I2C: https://docs.particle.io/tutorials/learn-more/about-i2c/
*/

// This #include statement was automatically added by the Particle IDE.
#include <adafruit-sht31.h>

// This #include statement was automatically added by the Particle IDE.
#include <RunningAverage.h>

//TODO: check for ubidots webhook correctness: https://help.ubidots.com/en/articles/513304-connect-your-particle-device-to-ubidots-using-particle-webhooks
//TODO: incorporate sd card: https://docs.particle.io/cards/libraries/s/SdFat/
//TODO: use Mike Otto spreadsheet to display infection values on Ubidots
#define subsamples 5

Adafruit_SHT31 sht31 = Adafruit_SHT31();      // SHT31 or SHT35 in air,  T/RH

RunningAverage myRA1(subsamples);                    // statistics
RunningAverage myRA2(subsamples);                // statistics
FuelGauge fuel;

int Time_old;

float panelTemperature;
float panelHumidity;

// Variables Related To Particle Mobile Application Reporting
char SignalString[64];
char panelHumidityString[16];             //  RH
char panelTemperatureString[16];          //  T
char batteryString[16];

float batteryVoltage;
int stateOfCharge;
int rat;
float strengthPercentage =0.0;
float qualityPercentage=0.0;

SystemSleepConfiguration config; // more on sleep: https://docs.particle.io/tutorials/learn-more/about-sleep/

void setup() {

 Wire.begin();

 Particle.variable("BattVolt", batteryString); // Particle variables that enable monitoring using the mobile app
 Particle.variable("SensorT", panelTemperatureString);
 Particle.variable("SensorRH", panelHumidityString);
 Particle.variable("Signal", SignalString);

 config.mode(SystemSleepMode::STOP) // configure sleep mode; sleep 27 minutes between readings
       .duration(27min);

 sht31.begin(0x44); // SHT31 T/RH

 myRA1.clear();
 myRA2.clear();

}

void loop() {

    if(Time.minute() % 30 == 0 && Time_old != Time.minute()){ //Samples every 30 min. change the "30" to change sample interval in min (1 - 59)

    digitalWrite(D7, HIGH); //turn on led

    takeMeasurements();

    getSignalStrength();

    sendData();

    Time_old = Time.minute(); // resetting time

    System.sleep(config); // set device to sleep

    }
}

void sendData() { //send data to particle cloud
  char data[512]; // Store the date in this character array - not global
  unsigned long timeStampValue = Time.now(); // timestamps
    snprintf(data, sizeof(data),"{\"T\":%4.1f, \"RH\":%4.1f, \"battery\":%4.1f, \"charge\":%i, \"Signal\": %4.1f, \"Quality\": %4.1f, \"timestamp\":%lu000}",panelTemperature, panelHumidity, batteryVoltage, stateOfCharge, strengthPercentage, qualityPercentage, timeStampValue);
    Particle.publish("AWQP_Cercospora_Monitor", data, PRIVATE);
  }

bool takeMeasurements() {

    if (Cellular.ready()){
  // Get battery voltage level and state  of charge
  batteryVoltage = fuel.getVCell();  // = 4.0 Voltage level of battery
  snprintf(batteryString, sizeof(batteryString), "%4.1f V", batteryVoltage);
  stateOfCharge = int(fuel.getSoC());

    for (byte i=0; i<=subsamples; i++)   {
      panelTemperature = sht31.readTemperature();
      panelHumidity = sht31.readHumidity();
      delay(30);
      myRA1.addValue(panelTemperature);
      myRA2.addValue(panelHumidity);
    }
    panelTemperature=myRA1.getAverage();
    panelHumidity=myRA2.getAverage();


    snprintf(panelTemperatureString,sizeof(panelTemperatureString), "%4.1f C", panelTemperature);
    snprintf(panelHumidityString,sizeof(panelHumidityString), "%4.1f%%", panelHumidity);
    myRA1.clear();
    myRA2.clear();

  }

  return 1;
}

void getSignalStrength()
{
  CellularSignal sig = Cellular.RSSI();
  rat = sig.getAccessTechnology();

    strengthPercentage = sig.getStrength();
    qualityPercentage = sig.getQuality();

 snprintf(SignalString,sizeof(SignalString), "S:%2.0f%%, Q:%2.0f%% ", strengthPercentage, qualityPercentage);
}

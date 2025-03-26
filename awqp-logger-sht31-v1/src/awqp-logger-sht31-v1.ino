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

PRODUCT_VERSION(18);

#include <adafruit-sht31.h>
#include <RunningAverage.h>
#include <Wire.h>

#define TCAADDR 0x70
#define subsamples 5
#define DELAY_BEFORE_REBOOT 2000

// Initialize sensors and mux
Adafruit_SHT31 sht31 = Adafruit_SHT31();
float temp1, hum1;
float temp2, hum2;
RunningAverage raT1(subsamples);
RunningAverage raH1(subsamples);
RunningAverage raT2(subsamples);
RunningAverage raH2(subsamples);

// Diagnostics
FuelGauge fuel;
float batteryVoltage;
int stateOfCharge;
float strengthPercentage = 0.0;
float qualityPercentage = 0.0;
int rat;

// Timer
int Time_old;

// Remote triggers
unsigned int rebootDelayMillis = DELAY_BEFORE_REBOOT;
unsigned long rebootSync = millis();
bool resetFlag = false;
bool resetFlagReadings = false;

// Cloud Variables
char SignalString[64];
char batteryString[16];
char tempString[32];
char humString[32];
char positionString[64];

SystemSleepConfiguration config;
SystemPowerConfiguration conf;

void tcaSelect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(9600);
  Particle.publish("AWQP Sensor Initiated: Dual SHT3X, "+Time.timeStr());

  Wire.begin();
  tcaSelect(0);
  sht31.begin(0x44);
  tcaSelect(1);
  sht31.begin(0x44);
  raT1.clear(); raH1.clear();
  raT2.clear(); raH2.clear();

  Particle.variable("BattVolt", batteryString);
  Particle.variable("SensorT", tempString);
  Particle.variable("SensorRH", humString);
  Particle.variable("Signal", SignalString);
  Particle.variable("Position", positionString);
  Particle.function("reset", cloudResetFunction);
  Particle.function("TakeMeasurementsNow", takeMeasurementNow);

  config.mode(SystemSleepMode::STOP).duration(27min);
  pinMode(D7, OUTPUT);
}

void loop(){
  if(Time.minute() % 30 == 0 && Time_old != Time.minute()){  
    getSignalStrength();
    takeMeasurements();
    sendData();
    digitalWrite(D7, HIGH);
    Time_old = Time.minute();
    delay(8000);
    digitalWrite(D7, LOW);
    System.sleep(config);
  }

  if ((resetFlag) && (millis() - rebootSync >=  rebootDelayMillis)) {
    Particle.publish("Debug", "Remote Reset Initiated", 300, PRIVATE);
    System.reset();
  }    

  if (resetFlagReadings) {
    Particle.publish("Input Recieved: Taking Measurements. "+Time.timeStr());
    getSignalStrength();
    takeMeasurements();
    sendData();
    digitalWrite(D7, HIGH);
    Particle.publish("Measurements taken and payload sent. "+Time.timeStr());
    delay(3000);
    resetFlagReadings = false;
    digitalWrite(D7, LOW);
  }
}

void sendData() {
  unsigned long timestamp_seconds = Time.now(); 
  char msg[512];
  snprintf(msg, sizeof(msg) - 1,
    "{\"T1\":%4.1f, \"RH1\":%4.1f, \"T2\":%4.1f, \"RH2\":%4.1f, \"battery\":%4.1f, \"charge\":%i, \"Signal\":%4.1f, \"timestamp\":%lu000}",
    temp1, hum1, temp2, hum2, batteryVoltage, stateOfCharge, strengthPercentage, timestamp_seconds);

  Particle.publish("AWQP_Cercospora", msg, PRIVATE);
  Particle.publish("AWQP_Sensor_SHT", msg, PRIVATE);
  Serial.println("Particle Published!");
}

bool takeMeasurements() {
  if (Cellular.ready()){
    batteryVoltage = fuel.getVCell();
    snprintf(batteryString, sizeof(batteryString), "%4.1f V", batteryVoltage);
    stateOfCharge = int(fuel.getSoC());

    for (byte i=0; i<=subsamples; i++) {
      tcaSelect(0);
      raT1.addValue(sht31.readTemperature());
      raH1.addValue(sht31.readHumidity());
      tcaSelect(1);
      raT2.addValue(sht31.readTemperature());
      raH2.addValue(sht31.readHumidity());
      delay(30);
    }

    temp1 = raT1.getAverage();
    hum1 = raH1.getAverage();
    temp2 = raT2.getAverage();
    hum2 = raH2.getAverage();

    snprintf(tempString,sizeof(tempString), "T1: %4.1f C, T2: %4.1f C", temp1, temp2);
    snprintf(humString,sizeof(humString), "RH1: %4.1f%%, RH2: %4.1f%%", hum1, hum2);

    raT1.clear(); raH1.clear();
    raT2.clear(); raH2.clear();
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

int cloudResetFunction(String command) {
  resetFlag = true;
  rebootSync = millis();
  return 1;
}

int takeMeasurementNow(String command) {
  resetFlagReadings = true;
  return 1;
}

void batt_settings() {
  conf.powerSourceMaxCurrent(1500)
      .powerSourceMinVoltage(3750)
      .batteryChargeCurrent(500)
      .batteryChargeVoltage(4200);
  System.setPowerConfiguration(conf);
}

// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain


#include "DHT.h"

#define DESIREDTEMP 8.00f //FDA desired temp: 4 Celsius; Temperature for testing on home-fridge: 8 Celsius

#define DHTPIN 2     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor for normal 16mhz Arduino
//DHT dht(DHTPIN, DHTTYPE);
// NOTE: For working with a faster chip, like an Arduino Due or Teensy, you
// might need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// Example to initialize DHT sensor for Arduino Due:
DHT dht(DHTPIN, DHTTYPE, 6);

//Variables associated with checking for frequency:
unsigned int second_tick;//counts every 2 seconds, resets every 60 seconds
unsigned int loop_tick; //counts every 15 minutes, resets AFTER 2 loops/aka 30 minutes. Triggers change-tick reset
int change_tick; //tracks amount of changes in a given loop

//Variables associated with checking for Hi-Temp Duration.
unsigned int c_Second_tick;//counts every 2 seconds, resets every 60 seconds. ONLY WHEN TEMP IS TOO HIGH

int err_code; //if 0, no err; if 1, temperature has been high for too long; if 2, temperature has changed too much, too many times, over small time duration.
bool hiTempAlert;

float t_old;//temperature of previous loop

void setup() {
  Serial.begin(9600); 
  Serial.println("DHTxx test!");
 
  dht.begin();

  second_tick = 0;
  c_Second_tick = 0;
  change_tick = -1;//ignore first setting of t_old to t.
  hiTempAlert = false;
  err_code = 3; //set to value != 0-2
}

//

void loop(){
  /*
   * Measurement Phase
   */
  // Wait a ~2 seconds between measurements.
  delay(2000); //2,000 = ~2 sec
  second_tick = second_tick + 1;
  if(second_tick >= 450){//if 15 minutes have passed - note every tick = 2 seconds
    second_tick = 0;
    loop_tick = loop_tick + 1;
  }
  if(loop_tick > 2){
    loop_tick = 0;
  }

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius
  float t = dht.readTemperature();

  /*
   * Measurement Verification Phase
   */

   // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
   Serial.print(" - Temperature: "); 
   Serial.print(t);
   Serial.print(" *C - ");
   //Serial.print(f);
   //Serial.print(" *F\t\n");

  /*
   *  Deduction Phase
   */
   err_code = deduceError(t);
   if(err_code == 0){}{
     Serial.print("\n");
   }
   if( err_code == 1 ){
     Serial.print("Issue! - Temperature has risen above safe levels for over 15 minutes! \n");
   }//duration check (15 minutes)
   if( err_code == 2){
     Serial.print("Issue! - Temperature has changed by 5*C over 10 times \n");
   }//frequency check
     
   
}//end loop

/* 
 *  DEDUCTION/DETERMINE ERR PHASE
*/

int deduceError(float temp){
  
  int returnMe = 0;
  
  //Temperature check
   if( (temp > DESIREDTEMP) ){
    hiTempAlert = true;
    c_Second_tick = c_Second_tick + 1;
   }else{
    hiTempAlert = false;
   }
   if((c_Second_tick > 450) && (hiTempAlert == true)){
    c_Second_tick = 0;
    returnMe = 1;
    return returnMe;
   }

   //Freq. check
   if( (temp-t_old > 10) || (t_old - temp > 10) ){
    t_old = temp;
    change_tick = change_tick + 1;
   }else if(loop_tick >= 2){
    change_tick = 0;
   }
   if( (change_tick > 15) && (loop_tick < 0) ){
    returnMe = 2;
    second_tick = 0;
    loop_tick = 0;
    return returnMe;
   }

   return returnMe;
}//end deduceError

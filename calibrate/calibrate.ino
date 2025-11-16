/**
 * ArduiBeeScale - Scale Calibration Utility
 *
 * Interactive calibration tool for the HX711 load cell amplifier.
 * Determines the SCALE factor and OFFSET values required for accurate weight readings.
 *
 * Usage:
 * 1. Upload this sketch to Arduino
 * 2. Open Serial Monitor at 9600 baud
 * 3. Remove all weight from scale
 * 4. Place known weight on scale and adjust calibration
 * 5. Copy the output SCALE and OFFSET values to arduino.ino
 *
 * Project: ArduiBeeScale (Arduino Beehive Scale)
 * Maintained by: Jeremy JEANNE
 *
 * Original calibration code by Nathan Seidle (SparkFun Electronics)
 * HX711 library by bodge: https://github.com/bogde/HX711
 *
 * License: GNU GPLv3
 */

#include "HX711.h"

#define DIGITALOUT  5     //HX711 DT
#define CLOCK       6     //HX711 CKL

float   SCALE =     false;  //Set to 'false' on your first try, then enter your calibration factor
long    OFFSET =    false; //Set to 'false' if you dont know your offset / zero factor.
//My values with 200kg sensor on 3.3V
//float   SCALE =     -19689.35;  //Set to 'false' on your first try, then enter your calibration factor
//long    OFFSET =    -145680; //Set to 'false' if you dont know your offset / zero factor.

HX711 scale(DIGITALOUT, CLOCK);

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or s to decrease calibration factor");
  Serial.println("Press t to tare and set scale to zero");


  scale.set_scale();

  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor / OFFSET: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
  zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor / OFFSET: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);

  if(OFFSET){
      scale.set_offset(OFFSET); //set offset
  } else {
      scale.tare(); //Else reset scale to zero
  }
}

void loop() {
  if(SCALE){
      scale.set_scale(SCALE);
  }
  Serial.print("Reading: ");
  Serial.print(scale.get_units(5));
  Serial.print(" weight");
  Serial.print(" calibration_factor: ");
  Serial.print(SCALE);
  Serial.println();

  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      SCALE += 10;
    else if(temp == '-' || temp == 's')
      SCALE -= 10;
    else if(temp == 't'){
      scale.tare();
      SCALE = 0.00;
    }
  }
}

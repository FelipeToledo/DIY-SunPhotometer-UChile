/* Copyright 2017 Felipe Toledo, Cristobal Garrido
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// The user can associate an ID with the prototype that is printed in each output line
#define ID 1

// Import libraries for peripherals
#include <LiquidCrystal.h> //The Arduino LiquidCrystal library
#include <SD.h> //SD library
#include <Time.h> //Time library  
#include <Wire.h> //I2C library  
#include <DS1307RTC.h> // RTC Library  
#include <Adafruit_BMP085.h> // Adafruit's BMP085 library
#include <SPI.h> // SPI comunication library


// Initialize LCD screen
LiquidCrystal lcd(9, 10, 6, 4, 3, 2);


// Associate the buttons to arduino pins
const int buttonPin = A2;
const int buttonPin0 = 7;
const int buttonPin1 = A3;

const int chipSelect = 8;  // Datalogger pin configuration. For our model its value is fixed to 8

#define PIN_BUZZER 5  // Buzzer pin

#define TIEMPO_MEDICION 15000  // sampling time in millis

#define SEA_LEVEL_PRESSURE 101500  // Sea level prssure


// Initialize buttons
int buttonState = 0;
int buttonState0 = 0;
int buttonState1 = 0;

// Initialize file and buffer for writing
String data = "";  // Buffer for the output line
File dataFile;

// Initialize BMP085
Adafruit_BMP085 bmp;  //call to BMP 085 sensor


// Init routine ------------------------------------------------------------------
void setup()
{

  inicioBuzzer();  // Buzzer

  // Welcome screen
  lcd.begin(16, 2);
  lcd.setCursor(3,0);  
  lcd.print("Fotometro");
  lcd.setCursor(2, 1);
  lcd.print("FCFM Uchile");

  setSyncProvider(RTC.get);   // Start the time object from the RTC

  // If the time does not work, an error will be displayed
  if(timeStatus()!= timeSet) {
    lcd.clear();
    lcd.home();
    lcd.print("RTC Error");
  }

  pinMode(chipSelect, OUTPUT);  // Setup Olimex-MCI datalogger

  // Init input pins
  pinMode(buttonPin, INPUT);
  pinMode(buttonPin0, INPUT);
  pinMode(buttonPin1, INPUT);
   
  /* Check if the SD card is present and can be initialized.
   * It it isn't working the instrument plays an error sound
   * and stops execution */
  if (!SD.begin(chipSelect)) {
    lcd.clear();//SD validation
    lcd.home();
    lcd.print("SD card Error");
    errorSD();    
  }
  
  // If the output file does not exist, create one with header
  if(!SD.exists("DATALOG.csv")){
    File dataFile = SD.open("DATALOG.csv", FILE_WRITE);
    dataFile.println("Year,Month,Day,Hour,Minute,Second,Sens_yellow_nm,Sens_blue_nm,Temperature_C,Pressure_Pa,Altitude_m,ID");
    dataFile.close();
  }

  /* Check if the BMP 085 or 180 is present. It it isn't, print an
   * error and stop execution */
  if (!bmp.begin()) {
    lcd.clear();
    lcd.home();
    lcd.print("BMP Error");
    while (1) {
    }
  }

  // Init succesfull! Show main menu.
  lcd.clear();
  lcd.home();
  lcd.print("Medir");
  lcd.setCursor(14, 0);
  lcd.print("<>");
  lcd.setCursor(14, 1);
  lcd.print("ok");

}


// Body of the program ------------------------------------------------------------------

void loop() {
  
  buttonState = digitalRead(buttonPin);
  buttonState0 = digitalRead(buttonPin0);
  buttonState1 = digitalRead(buttonPin1);


      
  /* MEASUREMENT ROUTINE -----------------------------------------------------------------
   * If buttonPin is pushed, the photometer will begin the measurement routine
   * It samples output voltages from the LED sensor board and saves the maximum value
   * registered in TIEMPO_MEDICION sampling time */

  if (buttonState == HIGH) {

    // Emits a sound a change display to measurement state
    inicioBuzzer();
    lcd.clear();
    lcd.home();
    lcd.print("Midiendo...");

    // Open output file
    File dataFile = SD.open("DATALOG.csv", FILE_WRITE);
      
      
    if (dataFile) {

      // Print date to output file
      data += String(year()) + "," + String(month()) + "," + String(day()) + ",";
      data += String(hour()) + "," + String(minute()) + "," + String(second()) + ","; 
      dataFile.print(data);
      data = "";
        
      // Init measurement storage variables (sensX stores current maximum and sensBuffX stores instantaneous measurement)
      int sens1 = 0;
      int sensBuff1 = 0;
      int sens2 = 0;
      int sensBuff2 = 0;

      long inicio = millis();  // Time of sampling start

      // Iterates measurements for TIEMPO_MEDICION milliseconds and stores the maximum voltage for each sensor channel
      while((millis() - inicio) < TIEMPO_MEDICION) {
        
        sensBuff1 = analogRead(A0);
        sensBuff2 = analogRead(A1);
        
        sens1 = ( sens1 < sensBuff1 ) ? sensBuff1 : sens1 ;
        sens2 = ( sens2 < sensBuff2 ) ? sensBuff2 : sens2 ;
      }

      // Register the maximum for each channel in the output file
      data =  String(sens1) + "," + String(sens2) + ",";
      dataFile.print(data);  

      // Read the temperature, pressure and altitude from the BMP sensor and write these in the output file
      data = "";
      data =  doubleToString(bmp.readTemperature()) + ","; 
      data += doubleToString(bmp.readPressure()) + ",";
      dataFile.print(data); 
        
      data = "";
      data = doubleToString(bmp.readAltitude(SEA_LEVEL_PRESSURE)) + "," + ID;
      dataFile.println(data);
      data = "";
      
      // Close output file, play the sound for a complete measurement
      dataFile.close();
      finBuzzer();

      // Print the registered max voltage
      lcd.clear();
      lcd.home();  
      lcd.print("Yellow");
      lcd.setCursor(8, 0);
      lcd.print("Blue");    
      lcd.setCursor(0, 1);
      lcd.print("  ");
      lcd.setCursor(0, 1);
      lcd.print(sens1);
      lcd.print("  ");
      lcd.setCursor(8, 1);
      lcd.print(sens2);

      // Wait for another buttonPin press to return to the main screen
      while(!digitalRead(buttonPin)) {
      }
        
      delay(500);  // Pause before returning to the main screen
    }

    // If the file cannot be open, display an error:
    else {
      lcd.clear();
      lcd.home();
      lcd.println("TXT error");
    }
    
  // Reset screen to the main menu
  lcd.clear();
  lcd.home();
  lcd.print("Medir");
  lcd.setCursor(14, 0);
  lcd.print("<>");
  lcd.setCursor(14, 1);
  lcd.print("ok");
  
  }
  // END OF MEASUREMENT ROUTINE

  /* COMMUNICATION ROUTINE -----------------------------------------------------------------
   * If buttonPin0 is pushed, the photometer will begin the communication routine
   * The user must send any character throught the serial port to the photometer
   * to start the sending routine. In the sending routine the photometer will dump
   * the whole output file in the serial port.
  */
 
  else if (buttonState0 == HIGH){

    // Print connection message on screen
    lcd.clear();
    lcd.home();
    lcd.println("Conectando");

    // Open serial commumication
    Serial.begin(9600);

    // Wait for the serial port to receive any character before sending the data
    while (!Serial.available()) {
    }
    
    // Open the output file
    dataFile = SD.open("DATALOG.csv", FILE_READ);

     // Read each line of the output file and print them to the serial port
    if (dataFile) {
      while (dataFile.available()) {
        Serial.write(dataFile.read());
      }
      
      Serial.write(64);
      dataFile.close();
      lcd.clear();
      lcd.home();
      lcd.print("Medir");
      lcd.setCursor(14, 0);
      lcd.print("<>");
      lcd.setCursor(14, 1);
      lcd.print("ok");
    } 
    else {
      // if the file didn't open, print an error:
      Serial.println("error opening DATALOG.csv");
    }
  }
  // END OF COMUNICATION ROUTINE

  /* TESTING ROUTINE -----------------------------------------------------------------
   * If buttonPin1 is pushed, the photometer will begin the testing routine
   * It displays in the screen the maximum value recorded by each sensor channel
   * registered during the whole testing routine instance.
   * To close this instance buttonPin1 must be pressed again. This also resets the
   * maximums registered.
  */
  
  else if (buttonState1 == HIGH) {

    // Change display to testing routine session
    lcd.clear();
    lcd.home();
    lcd.print("Yellow");
    lcd.setCursor(8, 0);
    lcd.print("Blue");

    // Dummy variables to store the maximum values registered
    int amarillo;
    int azul;
    int maxAmarillo = 0;
    int maxAzul = 0;

    // Protection against long presses of buttonPin1
    buttonState1 = 0;
    delay(500);

    // Iterates measurements and stores the maximum voltage for each sensor channel until buttonPin1 is pressed
    while(!digitalRead(buttonPin1)) {

      // Print the maximum of the whole instance with 300 samples between each update
      for(int i = 0; i < 300; i++) {
      
        amarillo = analogRead(A0);
        azul = analogRead(A1);
        
        maxAmarillo = ( maxAmarillo < amarillo ) ? amarillo : maxAmarillo ;
        maxAzul = ( maxAzul < azul ) ? azul : maxAzul ;
      }

      // Print max values on screen
      lcd.setCursor(0, 1);
      lcd.print(maxAmarillo);
      lcd.setCursor(8, 1);
      lcd.print(maxAzul);
    }

    // After leaving instance reset display to main menu
    delay(500);
    lcd.clear();
    lcd.home();
    lcd.print("Medir");
    lcd.setCursor(14, 0);
    lcd.print("<>");
    lcd.setCursor(14, 1);
    lcd.print("ok");
  }
}


String doubleToString(double num){
  // It receives a double number and return it as a string

  char buf[32]; 
  String palabra;

  dtostrf(num,4,8,buf);
  // dtostrf( [doubleVar] , [sizeBeforePoint] , [sizeAfterPoint] , [WhereToStoreIt] )
  palabra = String(buf);  // cast it to string from char 
  return palabra;
}


void inicioBuzzer() {
  // Configuration for the initial sound
  tone(PIN_BUZZER, 1046, 220); //tone(pin, frecuencia, duracion (milisegundos))
}


void finBuzzer() {
  // Configuration to the final sound
  tone(PIN_BUZZER, 1046);
  delay(220);
  noTone(PIN_BUZZER);
  tone(PIN_BUZZER, 1046, 220);
  delay(220);
}


void errorSD() {
  // Sound for the SD error, puts the program into an infinite loop to cancel execution
  while(true){

    tone(PIN_BUZZER, 698);
    delay(300);
    noTone(PIN_BUZZER);

    delay(550);
  }
}




#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SoftwareSerial.h>

#define SEALEVELPRESSURE_HPA (1013.25) // Typical sea level pressure in hPa (hectopascals)
#define THRESHOLD_ALTITUDE 600 // Altitude threshold for parachute deployment in meters
#define MOTOR_PIN 10 // Motor control pin
#define video 6


Adafruit_BME280 bme; // I2C
int previousAltitude = 0; // Previous altitude reading
int altitudeOffset = 0; // Offset for altitude calibration
unsigned long lastTime = 0; // Last time altitude was measured
bool firstReading = true; // Flag to indicate first altitude reading
bool ascending = false; // Flag to indicate ascending altitude
bool descending = false; // Flag to indicate descending altitude
bool parachuteDeployed = false; // Flag to indicate if parachute is deployed
unsigned long parachuteStartTime = 0; // Parachute deployment start time
String canstatus = "0";
int counter = 0;
bool printData = false;


SoftwareSerial mySerial(7,8); // RX, TX
SoftwareSerial xbee(2,3);


int readAverageAltitude() {
  int sum = 0;
  for (int i = 0; i < 10; i++) { // Take 10 altitude readings
    sum += bme.readAltitude(SEALEVELPRESSURE_HPA); // Read altitude from BME280 sensor
    delay(100); // Delay to stabilize sensor
  }
  return sum / 10; // Calculate and return average altitude
}

void activateBuzzer() {
  Serial.println("activate");
}

void deployParachute() {
//--
unsigned long startTime = millis();
  unsigned long duration = 1000; // 1 second in milliseconds
  
  while (millis() - startTime < duration) {
    digitalWrite(MOTOR_PIN, HIGH); // Turn the motor on
    // You can add additional motor control code here if needed
  }
  
  digitalWrite(MOTOR_PIN, LOW); // Turn the motor off after 1 second
}

void readBMPValues(float bmpValues[]) {
  // Read BMP sensor values (altitude, pressure, temperature, humidity)
  int currentAltitude = bme.readAltitude(SEALEVELPRESSURE_HPA) - altitudeOffset;
  int pastAltitude = previousAltitude; // Assuming you have a global variable 'previousAltitude' storing past altitude

  // Determine status based on altitude values
  //String canstatus;
  if (currentAltitude > pastAltitude) {
    canstatus = "Ascending";
  } else if (currentAltitude == pastAltitude) {
    canstatus = "Stationary";
  } else {
    canstatus = "Descending";
  }
  // Check for special conditions
  if (currentAltitude < 5) {
    canstatus = "ReadyToLaunch";
  } else if (currentAltitude > pastAltitude && currentAltitude < 20) {
    activateBuzzer(); // Call function to activate buzzer
  }
  //Serial.println(altitudeOffset);
  // Store sensor values and status in the BMP sensor values array
  bmpValues[0] = currentAltitude;
  bmpValues[1] = bme.readPressure() / 100.0F; // Convert pressure to hPa
  bmpValues[2] = bme.readTemperature();
  bmpValues[3] = bme.readHumidity();

  // Update the previous altitude for next iteration
  previousAltitude = currentAltitude;

  // Print status
  //Serial.println(canstatus);
  }

void calibrateSensors() {
  
  int currentAltitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  altitudeOffset = currentAltitude;
  //Serial.println(altitudeOffset);
  //Serial.println("Calibratting... & done");
}



void setup() {

  pinMode(video,OUTPUT);
  digitalWrite(video,HIGH);
  Serial.begin(9600);
  mySerial.begin(9600);
  xbee.begin(9600);
  pinMode(MOTOR_PIN, OUTPUT); 
  digitalWrite(MOTOR_PIN,0);// Initialize motor pin as output
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

String sensorData = "0,0,0,0,0,0,";

void loop() {
  // Check if there is data available from the other Arduino
  if (mySerial.available() > 0) {
    // Read the data sent from the other Arduino
    sensorData = mySerial.readStringUntil('\n');
    sensorData.trim();
    Serial.println(sensorData);
    // Print the received sensor data to the Serial Monitor
    //Serial.println(sensorData);
  }
  
  // Check if there is data available from the Serial Monitor
  if (Serial.available() > 0) {
    // Read the command from the Serial Monitor
    char command = Serial.read();
    // If the command is 'c', send 'c' to the other Arduino for calibration
    if (command == 'c') {
      calibrateSensors();
      //mySerial.write('c');
    }
    if (command == 'v') {
      digitalWrite(video,LOW);
    }
    if(command == 'd'){
      deployParachute();
    }
    if (command == 's') {
      printData = true; // Set flag to start printing
      //Serial.println("Printing data...");
    }
  }

  if(printData ){

  unsigned long currentTime = millis(); // Current time

  // Take the first altitude reading
  if (firstReading) {
    if (currentTime - lastTime >= 3000) {
      previousAltitude = bme.readAltitude(SEALEVELPRESSURE_HPA) - altitudeOffset; // Store the average altitude reading
      //Serial.println("First reading taken.");
      firstReading = false; // Set the flag to indicate first reading is done
      lastTime = currentTime; // Update last time altitude was measured
    }
  } else {
    // Take the second altitude reading after 3 seconds
    if (currentTime - lastTime >= 3000) {
      int currentAltitude = bme.readAltitude(SEALEVELPRESSURE_HPA) - altitudeOffset; // Calculate average altitude

      // Determine the trend
      if (currentAltitude > previousAltitude) {
        //Serial.println("Ascending");
        ascending = true;
        descending = false;
      } else if (currentAltitude < previousAltitude) {
        //Serial.println("Descending");
        ascending = false;
        descending = true;
      } else {
        //Serial.println("Stable");
        ascending = false;
        descending = false;
      }
      
      previousAltitude = currentAltitude; // Update previous altitude reading
      lastTime = currentTime; // Update last time altitude was measured
      firstReading = true; // Set the flag for the next iteration
    }
  }

  // Read BMP sensor values and print data
  float bmpValues[4];
  readBMPValues(bmpValues);
  printSensorData(bmpValues,canstatus);
  //printSensorData
}
delay(1000);
}

void printSensorData(float bmpValues[], String canstatus) {
  String data = "2022ASI027,"+(String)counter+",00:00:00,"+ sensorData + "0,0,0," + (String)bmpValues[0] + "," + (String)bmpValues[1] + "," + (String)bmpValues[3] + "," + (String)bmpValues[2] + ",0,0,0,80," + canstatus + "";
  Serial.println(data);
  xbee.println(data);
  counter++;
}

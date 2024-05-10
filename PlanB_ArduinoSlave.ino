#include <Wire.h>
#include <MPU6050_tockn.h>
#include <SoftwareSerial.h>

MPU6050 mpu6050(Wire);
SoftwareSerial mySerial(2,3 ); // RX, TX


// Define calibration offsets
float gyroX_offset = 0.0;
float gyroY_offset = 0.0;
float gyroZ_offset = 0.0;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Wire.begin();
  mpu6050.begin();
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {
      case 'c':
        // Calibrate MPU6050
        calibrateMPU();
        break;
      default:
        break;
    }
  }

  float mpuValues[6]; // Array to store MPU sensor values
  readMPUValues(mpuValues);
  printMPUData(mpuValues); // Print MPU data to Serial Monitor
  sendMPUDataToSerial(mpuValues); // Send MPU data over SoftwareSerial
  // Stabilize platform based on calibrated gyro readings
  delay(1000); // Adjust delay as needed
}

void readMPUValues(float mpuValues[]) {
  mpu6050.update();
  mpuValues[0] = mpu6050.getAccX();
  mpuValues[1] = mpu6050.getAccY();
  mpuValues[2] = mpu6050.getAccZ();
  mpuValues[3] = mpu6050.getGyroX();
  mpuValues[4] = mpu6050.getGyroY();
  mpuValues[5] = mpu6050.getGyroZ();
}

void printMPUData(float mpuValues[]) {
  // Print MPU sensor data to Serial Monitor
  Serial.print("AccX: ");
  Serial.print(mpuValues[0]);
  Serial.print(", AccY: ");
  Serial.print(mpuValues[1]);
  Serial.print(", AccZ: ");
  Serial.print(mpuValues[2]);
  Serial.print(", GyroX: ");
  Serial.print(mpuValues[3]);
  Serial.print(", GyroY: ");
  Serial.print(mpuValues[4]);
  Serial.print(", GyroZ: ");
  Serial.println(mpuValues[5]);
}

void sendMPUDataToSerial(float mpuValues[]) {
  // Send MPU sensor data over SoftwareSerial
  for (int i = 0; i < 6; i++) {
    mySerial.print(mpuValues[i], 2);
    mySerial.print(",");
  }
  mySerial.println(); // End of data set
}

void calibrateMPU() {
  mpu6050.calcGyroOffsets(true);
  Serial.println("MPU6050 calibration completed");

  // Store calibration offsets
  gyroX_offset = mpu6050.getGyroXoffset();
  gyroY_offset = mpu6050.getGyroYoffset();
  gyroZ_offset = mpu6050.getGyroZoffset();
  Serial.print("Calibration offsets: X=");
  Serial.print(gyroX_offset);
  Serial.print(", Y=");
  Serial.print(gyroY_offset);
  Serial.print(", Z=");
  Serial.println(gyroZ_offset);
}

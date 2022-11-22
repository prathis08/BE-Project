#include <SPI.h>
#include <SD.h>

int scan_amount = 40;                    //Amount of scans for each point. The result is the mean. This would increase the delay
String file = "scan_001.txt";            //Name of the saved file on the SD card
int z_axis_height = 12;                  //in cm         //Maximum height of the scaned file
int step_delay = 3000;                   //in us          //Delay for each step for the stepper motor in microseconds
float z_layer_height = 0.5;              //in mm     //Layer height. The amount of mm for each layer.
int lead_screw_rotations_per_cm = 8;     //How many rotations needs the lead screw to make in order to make 1cm.
int steps_per_rotation_for_motor = 200;  //Steps that the motor needs for a full rotation.
int distance_to_center = 8;              //In cm. Distance from sensor to the turntable center in cm

//I/O
int button = 2;
int limit_switch = 10;
//Turntable driver pins
int dir_r = 7;
int step_r = 8;
int enable_r = 9;
//Z-axis driver pins
int dir_z = 3;
int step_z = 5;
int enable_z = 6;

//Variables
File file_values;      //Used for the SD card module
int scan = 0;          //Activate/deactivate scanning
int scan_changed = 0;  //Scan process was changed
float distance = 0;    //Measured distance
float angle = 0;       //Rotation angle for each loop (0º-360º)
float x = 0;           //X, Y and Z coordinate
float y = 0;
float z = 0;
int z_loop = 0;             //variable used for the z-axis motor rotation
int r_loop = 0;             //variable used for the turntable motor rotation
float measured_analog = 0;  //Analog read from the distance sensor
float analog = 0;           //Analog MEAN ß
float RADIANS = 0.0;        //Angle in radians. We calculate this value later in Setup loop
int steps_z_height = 0;     //Variable used for the amount of steps in z-axis
int homed = 0;

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(limit_switch, INPUT);
  pinMode(button, INPUT);
  analogReference(INTERNAL);
  SD.begin(4);
  pinMode(dir_z, OUTPUT);
  pinMode(step_z, OUTPUT);
  pinMode(enable_z, OUTPUT);
  pinMode(dir_r, OUTPUT);
  pinMode(step_r, OUTPUT);
  pinMode(enable_r, OUTPUT);
  digitalWrite(enable_z, HIGH);  //disable the z_azis driver
  digitalWrite(enable_r, HIGH);  //disable the z_azis driver


  //Calculate variables
  RADIANS = (3.141592 / 180.0) * (360 / steps_per_rotation_for_motor);
  steps_z_height = (z_layer_height * steps_per_rotation_for_motor * lead_screw_rotations_per_cm) / 10;

  /*UNCOMENT this if using limit swithc connected to D10
  while(!digitalRead(limit_switch) && homed == 0)
  {
    digitalWrite(enable_z,LOW);     //enable the z_azis driver
    digitalWrite(dir_z,HIGH);       //z_azis spin to left (lowering z-axis)
    digitalWrite(step_z,HIGH);      //z_azis make a step
    delayMicroseconds(step_delay);
    digitalWrite(step_z,LOW);
    delayMicroseconds(step_delay);
  }
  homed = 1;*/
}

void loop() {
  //Wait till the push button is pressed
  if (digitalRead(button)) {
    if (scan == 1 && scan_changed == 0) {
      scan = 0;
      delay(3000);
      scan_changed = 1;
    }
    if (scan == 0 && scan_changed == 0) {
      scan = 1;
      delay(3000);
      scan_changed = 1;
    }
    scan_changed = 0;
  }


  //If the scanning proces is ON
  if (scan == 1) {
    //We stop when we reach the maximum height
    if (z < z_axis_height) {
      for (int loop_cont = 0; loop_cont < steps_per_rotation_for_motor; loop_cont++) {
        getDistance();
        digitalWrite(enable_r, LOW);  //enable the turntable driver
        digitalWrite(dir_r, LOW);     //turntable spin to right
        digitalWrite(step_r, HIGH);   //make a step
        delayMicroseconds(step_delay);
        digitalWrite(step_r, LOW);
        delayMicroseconds(step_delay);
        angle = angle + RADIANS;  //Increase the angle by one more unit
        write_to_SD(x, y, z);     //Write x, y, z f…
      }
    }
  }

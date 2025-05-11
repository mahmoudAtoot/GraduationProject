#include <Servo.h>
Servo myservo;  // create servo object to control a servo
bool sensor_one = 1;
bool sensor_two = 1;
bool main_switch = 0;
bool internal_switch = 0;
#define sensor_one_pin PB3
#define sensor_two_pin PB4
#define main_switch_pin PB5
#define internal_switch_pin PB6
// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(sensor_one_pin, INPUT);
  pinMode(sensor_two_pin, INPUT);
  pinMode(main_switch_pin, INPUT_PULLUP);
  pinMode(internal_switch_pin, INPUT_PULLUP);
  myservo.attach(PA0);  // attaches the servo on PORT A  pin 0 to the servo object
}

// the loop function runs over and over again forever
void loop() {
  sensor_one = digitalRead(sensor_one_pin);
  sensor_two = digitalRead(sensor_two_pin);
  internal_switch = digitalRead(internal_switch_pin);
  main_switch = digitalRead(main_switch_pin);
  // sensors and switchs run on active low 
  
  if (sensor_one == 0 && sensor_two == 0 && main_switch == 0) {
    //open
    myservo.write(130);
    Serial.println("open");
  }
  else if (internal_switch == 0) {
    //close
    Serial.println("close");
    delay(3000);
    myservo.write(0);

  }

  // wait for a second
}

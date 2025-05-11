#include <Servo.h>
Servo myservo;  // create servo object to control a servo
bool knee_sensor = 1;
bool foot_sensor = 1;
bool main_switch = 0;
bool internal_switch = 0;
#define knee_sensor_pin PB3
#define foot_sensor_pin PB4
#define main_switch_pin PB5
#define internal_switch_pin PB6
// the setup function runs once when you press reset or power the board
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(knee_sensor_pin, INPUT);
  pinMode(foot_sensor_pin, INPUT);
  pinMode(main_switch_pin, INPUT_PULLUP);
  pinMode(internal_switch_pin, INPUT_PULLUP);
  myservo.attach(PA0);  // attaches the servo on PORT A  pin 0 to the servo object
}

// the loop function runs over and over again forever
}

void loop() {
  // put your main code here, to run repeatedly:
  knee_sensor = digitalRead(knee_sensor_pin);
  foot_sensor = digitalRead(foot_sensor_pin);
  internal_switch = digitalRead(internal_switch_pin);
  main_switch = digitalRead(main_switch_pin);
  // sensors and switchs run on active low 
  
  if (knee_sensor == 0 && foot_sensor == 0 && main_switch == 0) {
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

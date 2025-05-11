//#define debug


#include "EEPROM.h"


#define EEPROM_SIZE 4000


#include <HardwareSerial.h>

HardwareSerial BCSerial(1);
HardwareSerial NxSerial(2);

#include <ESP32Servo.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "Mahmoud";
const char* password = "12345678";

String id = "";
String idPassword = "";

String points = "0";
String cans_qty = "0";


  const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
        <title>ESP Color Sorting</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            body {font-family: Arial, Helvetica, sans-serif;}

            input[type=text], input[type=password] {
              width: 100&#37;
              padding: 12px 20px;
              margin: 8px 0;
              display: inline-block;
              border: 1px solid #ccc;
              box-sizing: border-box;
            }

            button {
              background-color: #04AA6D;
              color: white;
              padding: 14px 20px;
              margin: 8px 0;
              border: none;
              cursor: pointer;
              width: 100&#37;
            }

            button:hover {
              opacity: 0.8;
            }

            table {
              margin-left: auto;
              margin-right: auto;
            }
        </style>
    </head>
    <body>
        <table id="identificationTable" style="width:50&#37;">
            <tr>
                <th></th>
                <th><h2>Ideintification Search</h2></th>
                <th></th>
            </tr>
            <tr>
                <th>
                    <label for="id">ID: </label>
                </th>
                <th>
                    <input type="text" id="id" name="id" form="identificationForm">
                </th>
                <th></th>
            </tr>
            <tr>
                <th>
                    <label for="password">Password: </label>
                </th>
                <th>
                    <input type="text" id="password" name="password" form="identificationForm">
                </th>
                <th></th>
            </tr>
            <tr>
                <th></th>
                <th>
                    <form action="/get" target="hidden-form" id="identificationForm">
                        <button id="submitButton" type="submit" value="Submit" onclick="submitMessage()">Search</button>
                    </form>
                    <iframe style="display:none" name="hidden-form"></iframe>
                </th>
                <th></th>
            </tr>
            <tr>
                <th>
                    <span >Points: </span>
                </th>
                <th>
                    <span id="pointsSpan" style="color:red">%points%</span>
                </th>
                <th></th>
            </tr>
            <tr>
                <th>
                    <span >cans_qty: </span>
                </th>
                <th>
                    <span id="cansQtySpan" style="color:red">%cans_qty%</span>
                </th>
                <th></th>
            </tr>
        </table>
        <script type = "text/javascript">
            function submitMessage() {
                setTimeout(function(){ document.location.reload(true); }, 500);
            }
        </script>
    </body>
  </html>
  )rawliteral";



//-------------- variables ------------------
int MAXb = 20;

String BCbottle[20] =  {       "5449000151490",          "5449000161482",        "5906485301012",        "90377679",                     "90375675",                    "6281100510154",          "6258883332129",    "6258883332136",   "54493704", "5449000096258", "TL001236", "5449000008046"};
String BCcompany[20] = {  "Cappy Orng 1.5L",    "Cappy Grape 1.5L",     "Cappy Mango 1.5L",   "Cappy Grape 330",      "Cappy Orng 330ml",  "Original Mango 1.5L",   "Ein Asia 1.5L",        "Ein Asia 550ml",      "Arwa 550ml", "Jericho 1.5L", "test", "Coke 330"};
String BCtype[20] =  {       "plastic",          "metla",        "glass",        "plastic",                     "plastic",                    "plastic",          "plastic",    "plastic",   "plastic", "plastic", "plastic", "metal"};
float BCpoints[20] = {10, 10, 15, 10, 10, 10, 10, 10, 10, 10, 10, 10};

int MAXusers = 3;
String userNames[20] =  {"21712340", "21712341", "21712342"};
String userPwd[20] =  {"0000", "1111", "2222"};
String inUserName = "";
String inUserPass = "";

int eeprom_storage_address_plastic = 600;
int eeprom_storage_address_metal = 610;
int eeprom_storage_address_glass = 620;

String BCdataIn = "";
String NxdataIn = "";

int bottles_counter = 0;

float tot_points = 0;
String company = "";
int z = 0;
int relay_pin = 23;
int IDWeb = 0;
// create four servo objects
Servo topServo;
Servo midServo;
Servo stearingServo;

// Published values for SG90 servos; adjust if needed
int minUs = 600;
int maxUs = 2400;

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42
int topServoPin = 2;
int midServoPin = 4;
int stearingServoPin = 21;

int pos = 0;      // position in degrees
int ID = 0;

int max_storage = 1;
bool start_flag = 0;

String Strbuffer = "";
int buttles_in_storage_palstic;
int buttles_in_storage_metal;
int buttles_in_storage_glass;


int close_angle = 23;
int open_angle = 135;
int mid_close_angle = 63;
int mid_open_angle = 175;
int plasticAngle = 0;
int glassAngle = 90;
int metalAngle = 180;
int cansQty = 0;
bool cred_flag = 0;



String processor(const String& var) {
 Serial.println("processor var: " + var);
  if (var == "points") {
    return points;
  }
  else if (var == "cans_qty") {
    return cans_qty;
  }
 Serial.println("Processoe can't recognize placeholder: " + var);
  return String();
}


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}





ESP32PWM pwm;

void setup() {
  pinMode(relay_pin, OUTPUT);//FOR SOLENOID LOCK

  BCSerial.begin(115200, SERIAL_8N1, 18, 19);
  NxSerial.begin(9600, SERIAL_8N1, 16, 17);
   Serial.begin(115200);
  BCSerial.setTimeout(100);

  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  topServo.setPeriodHertz(50);      // Standard 50hz servo
  midServo.setPeriodHertz(50);      // Standard 50hz servo
  stearingServo.setPeriodHertz(50);      // Standard 50hz servo

  topServo.attach(topServoPin, minUs, maxUs);
  midServo.attach(midServoPin, minUs, maxUs);
  stearingServo.attach(stearingServoPin, minUs, maxUs);

  if (!EEPROM.begin(EEPROM_SIZE))
  {
   Serial.println("failed to initialise EEPROM"); delay(1000000);
  }






  topServo.write(close_angle);
  midServo.write(mid_close_angle);
  stearingServo.write(metalAngle);
delay(1000);

  //clearEEPROM();
send_NX_str("t11.txt","Searching WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
   Serial.print(".");
  }
  // Print local IP address and start web server
 Serial.println("");
 Serial.println("WiFi connected.");
 Serial.println("IP address: ");
 Serial.println(WiFi.localIP());
 Strbuffer=WiFi.localIP().toString().c_str();;
  Serial.println(Strbuffer);
 send_NX_str("t11.txt",Strbuffer);

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;

    if (request->hasParam("id")) {
     Serial.println("Param['id'] Received: " + request->getParam("id")->value());
      inputMessage = request->getParam("id")->value();
      id = inputMessage.c_str();
    }
    if (request->hasParam("password")) {
     Serial.println("Param['password'] Received: " + request->getParam("password")->value());
      inputMessage = request->getParam("password")->value();
      idPassword = inputMessage.c_str();
    }
    clientResponse();
    request->send(200);
  });
  server.onNotFound(notFound);
  server.begin();




  
  Strbuffer = eeprom_read_string(eeprom_storage_address_plastic);
  buttles_in_storage_palstic = Strbuffer.toInt();
  Strbuffer = eeprom_read_string(eeprom_storage_address_glass);
  buttles_in_storage_glass = Strbuffer.toInt();
  Strbuffer = eeprom_read_string(eeprom_storage_address_metal);
  buttles_in_storage_metal = Strbuffer.toInt();
  #ifdef debug
  Serial.println("buttles_in_storage_palstic reading:" + String(buttles_in_storage_palstic));
  Serial.println("buttles_in_storage_glass reading:" + String(buttles_in_storage_glass));
  Serial.println("buttles_in_storage_metal reading:" + String(buttles_in_storage_metal));
  #endif

  if (buttles_in_storage_palstic > max_storage || buttles_in_storage_glass > max_storage || buttles_in_storage_metal > max_storage) {
    NX_go_to_page("5");

    send_NX_str("qty.txt", String(buttles_in_storage_palstic + buttles_in_storage_glass + buttles_in_storage_metal));
    delay(1000);
    sendSMS(buttles_in_storage_palstic + buttles_in_storage_glass + buttles_in_storage_metal);

    //Serial.println("over bottles quantity you\nshould take bottles from the machine\nSMS was sent\r");
    while (1) {
      if (NxSerial.available()) {
        #ifdef debug
       Serial.println("b");
       #endif
        String NxdataIn = "";
        NxdataIn = NxSerial.readString();
        #ifdef debug
       Serial.print("NxSerial:");
       Serial.println(String(NxdataIn));
       #endif


        int index = NxdataIn.indexOf("open");
        if (index >= 0) {
          digitalWrite(relay_pin, 1);
          #ifdef debug
         Serial.println("opened");
         #endif

          eeprom_write_string(eeprom_storage_address_plastic, String(0));
          eeprom_write_string(eeprom_storage_address_glass, String(0));
          eeprom_write_string(eeprom_storage_address_metal, String(0));
          break;
        }

      }
    }
  }
  NX_go_to_page("0");





}

void loop() {




  if (BCSerial.available() > 0 && start_flag == 1) {
    BCdataIn = BCSerial.readString();
   Serial.print("BCdataIn:");
   Serial.println(BCdataIn);
    BCdataIn.trim();//remove the \n


    z = 0;
    for ( z ; z < MAXb; z++) {
      if (BCdataIn == BCbottle[z]) {

        // break;
       Serial.println(BCdataIn);
       Serial.print("z:");
       Serial.println(z);
        goto OK_lable;
      }
    }

    Serial.print("z:");
    Serial.println(z);

    if (z == (MAXb)) {

      //error not defined BC
      NX_go_to_page("6");
      delay(2500);
      send_qty_to_nex(bottles_counter);
      send_tpoints_to_nex(tot_points);
      send_comp_to_nex("not defined");

      goto Exit;
    }

OK_lable:


    if (BCtype[z] == "plastic") {
      stearingServo.write(plasticAngle);

      Strbuffer = eeprom_read_string(eeprom_storage_address_plastic);
      buttles_in_storage_palstic = Strbuffer.toInt();
      Serial.println("buttles_in_storage_palstic reading:" + String(buttles_in_storage_palstic));
      buttles_in_storage_palstic = buttles_in_storage_palstic + 1;
      eeprom_write_string(eeprom_storage_address_plastic, String(buttles_in_storage_palstic));
      Serial.println("buttles_in_storage_palstic write:" + String(buttles_in_storage_palstic));

    }
    else if (BCtype[z] == "glass") {
      stearingServo.write(glassAngle);

      Strbuffer = eeprom_read_string(eeprom_storage_address_glass);
      buttles_in_storage_glass = Strbuffer.toInt();
      Serial.println("buttles_in_storage_glass reading:" + String(buttles_in_storage_glass));
      buttles_in_storage_glass = buttles_in_storage_glass + 1;
      eeprom_write_string(eeprom_storage_address_glass, String(buttles_in_storage_glass));
      Serial.println("buttles_in_storage_glass write:" + String(buttles_in_storage_glass));

    }
    else if (BCtype[z] == "metal") {
      stearingServo.write(metalAngle);

      Strbuffer = eeprom_read_string(eeprom_storage_address_metal);
      buttles_in_storage_metal = Strbuffer.toInt();
      Serial.println("buttles_in_storage_metal reading:" + String(buttles_in_storage_metal));
      buttles_in_storage_metal = buttles_in_storage_metal + 1;
      eeprom_write_string(eeprom_storage_address_metal, String(buttles_in_storage_metal));
      Serial.println("buttles_in_storage_metal write:" + String(buttles_in_storage_metal));


    }

    delay(2000);


    for ( int i = mid_close_angle; i < mid_open_angle; i++) {
      midServo.write(i);
      delay(10);
    }
    delay(1000);
    for ( int i = mid_open_angle; i > mid_close_angle; i--) {
      midServo.write(i);
      delay(10);
    }



    bottles_counter++;
    tot_points = tot_points + BCpoints[z];
    send_qty_to_nex(bottles_counter);
    Serial.println("tot points:" + String(tot_points));
    send_tpoints_to_nex(tot_points);
    company = BCcompany[z];
    send_comp_to_nex(company);
    //check qty


    Strbuffer = eeprom_read_string(eeprom_storage_address_plastic);
    buttles_in_storage_palstic = Strbuffer.toInt();
    Serial.println("buttles_in_storage_palstic reading:" + String(buttles_in_storage_palstic));

    Strbuffer = eeprom_read_string(eeprom_storage_address_glass);
    buttles_in_storage_glass = Strbuffer.toInt();
    Serial.println("buttles_in_storage_glass reading:" + String(buttles_in_storage_glass));

    Strbuffer = eeprom_read_string(eeprom_storage_address_metal);
    buttles_in_storage_metal = Strbuffer.toInt();
    Serial.println("buttles_in_storage_metal reading:" + String(buttles_in_storage_metal));


    Strbuffer = eeprom_read_string(ID * 8);
    tot_points = Strbuffer.toInt() +  BCpoints[z];
    eeprom_write_string(ID * 8, String(tot_points));
    Strbuffer = eeprom_read_string(ID * 8);
    Serial.println("tot_points from EEPROM:" + Strbuffer);



    Strbuffer = eeprom_read_string(256 + (ID * 8));
    cansQty = Strbuffer.toInt() ;
    cansQty++;
    eeprom_write_string(256 + (ID * 8), String(cansQty));
    Strbuffer = eeprom_read_string(256 + (ID * 8));
    Serial.println("cansQty from EEPROM:" + Strbuffer);

    //*****************************
    if (buttles_in_storage_palstic > max_storage || buttles_in_storage_glass > max_storage || buttles_in_storage_metal > max_storage) {
      Serial.println("overrr qty:");
      NX_go_to_page("5");
      send_NX_str("qty.txt", String(buttles_in_storage_palstic + buttles_in_storage_glass + buttles_in_storage_metal));
      start_flag = 0;
      bottles_counter = 0;
      tot_points = 0;
      company = "";


      sendSMS(buttles_in_storage_palstic + buttles_in_storage_glass + buttles_in_storage_metal);



    }




    //end
Exit:;


  }










  if (NxSerial.available()) {
    NxdataIn = NxSerial.readString();
    Serial.print("NxSerial:");
    Serial.println(NxdataIn);


    int index = NxdataIn.indexOf("start");
    if (index >= 0) {
      cred_flag = 1;





    }
    index = NxdataIn.indexOf("UserName:");
    if (index >= 0) {
      if (cred_flag == 1) {

        inUserName = NxdataIn.substring(NxdataIn.indexOf("UserName:") + 9, NxdataIn.indexOf("###"));
        inUserPass = NxdataIn.substring(NxdataIn.indexOf("pass:") + 5, NxdataIn.indexOf("%^&"));
        Serial.print("inUserName:");
        Serial.println(inUserName);
        Serial.print("inUserPass:");
        Serial.println(inUserPass);




        ID = 0;
        for ( ID ; ID < MAXb; ID++) {
          if (inUserName == userNames[ID] && inUserPass == userPwd[ID]) {

            // break;
            Serial.println(inUserName);
            Serial.print("ID:");
            Serial.println(ID);
            start_flag = 1;
            NX_go_to_page("2");
            topServo.write(open_angle);
            delay(30);
            return;
          }
        }

        Serial.print("ID:");
        Serial.println(ID);

        if (ID == (MAXb)) {

          //error not defined BC
          NX_go_to_page("7");
          delay(2500);



        }









      }
    }
    index = NxdataIn.indexOf("finish");
    if (index >= 0) {
      tot_points = 0;
      Serial.println("finishing");
      topServo.write(close_angle);
    }


    index = NxdataIn.indexOf("open");
    if (index >= 0) {
      digitalWrite(relay_pin, 1);
      Serial.println("opened");

      eeprom_write_string(eeprom_storage_address_plastic, String(0));
      eeprom_write_string(eeprom_storage_address_glass, String(0));
      eeprom_write_string(eeprom_storage_address_metal, String(0));
    }
    index = NxdataIn.indexOf("close");
    if (index >= 0) {
      digitalWrite(relay_pin, 0);
      Serial.println("close");

    }





  }



}






void send_qty_to_nex(int bottles_qty) {
  send_NX_str("qty.txt", String(bottles_qty));
}
void send_tpoints_to_nex(int tot_points_) {
  send_NX_str("points.txt", String(tot_points_));
}

void send_comp_to_nex(String comp_mane) {
  send_NX_str("company.txt", comp_mane);
}

void sendSMS(int buttles_in_storage ) {
  Serial.print('\r');
  delay(1000);
  Serial.print('\r');
  delay(5000);
    Serial.print("AT");
Serial.print('\r');
delay(100);
Serial.print("AT");
Serial.print('\r');
delay(100);
Serial.print("AT");
Serial.print('\r');
delay(100);
Serial.print("AT");
Serial.print('\r');
delay(100);
Serial.print("AT");
Serial.print('\r');
delay(100);
Serial.print("AT");
Serial.print('\r');
delay(100);
Serial.print("AT+CMGF=1");
Serial.print('\r');
delay(500);
Serial.print("AT+CMGF=1");
Serial.print('\r');
delay(500);
/*
Serial.print("ATD0597547853;");
Serial.print('\r');
delay(100);
Serial.print("ATD0597547853;");
Serial.print('\r');
delay(100);
Serial.print("ATD0597547853;");
Serial.print('\r');
delay(100);
Serial.print("ATD0597547853;");
Serial.print('\r');
delay(10);
delay(10000);*/
  
    Serial.print("AT+CMGS=0566450460;\r");    //Start accepting the text for the message
    //to be sent to the number specified.
    //Replace this number with the target mobile number.
    delay(1000);
    Serial.print("over bottles quantity :" + String(int(buttles_in_storage)) + "\nyou should take bottles from the machine\r"); //The text for the message
    delay(1000);
    Serial.write(0x1A);  //Equivalent to sending Ctrl+Z
    delay(100);
    Serial.write(0x1A);
    delay(100);
    Serial.write(0x1A);
  delay(5000);
}

void send_NX_str(String atr, String str) {
  NxSerial.print(atr + "=\"" + str + "\"");
  NxSerial.write(0xff);
  NxSerial.write(0xff);
  NxSerial.write(0xff);

}
void NX_go_to_page(String page) {
  NxSerial.print("page " + page);
  NxSerial.write(0xff);
  NxSerial.write(0xff);
  NxSerial.write(0xff);

}

//__________________________________________ eeprom writeing string _____________________________________________




void eeprom_write_string(int startAddr , String toStore ) {
  int i = 0;
  for (; i < 8; i++) {
    EEPROM.write(startAddr + i, toStore[i]);
    //Serial.println(startAddr + i);
    if ((startAddr + i + 1) % 256 == 0) {
      Serial.println("<><> % 255 <><> new memory page <><>");
      EEPROM.commit();
    }
    if (toStore[i] == '\0') {
      break;
    }
  }
  EEPROM.commit();
}




//__________________________________________ eeprom Reading string _____________________________________________



String eeprom_read_string( int startAddr) {
  int i = 0;
  char  eeprom_data[8];
  for (i; i < 8; i++) {
    eeprom_data[i] = char(EEPROM.read(startAddr + i));
    if (eeprom_data[i] == '\0') {
      break;
    }

  }
  return String(eeprom_data);
}



void clientResponse() {




  Serial.print("WebUserName:");
  Serial.println(idPassword);
  Serial.print("WebUserPass:");
  Serial.println(id);




  IDWeb = 0;
  for ( IDWeb ; IDWeb < MAXusers; IDWeb++) {
    if (id == userNames[IDWeb] && idPassword == userPwd[IDWeb]) {

      // break;
      Serial.println(id);
      Serial.print("IDWeb:");
      Serial.println(IDWeb);

      points = eeprom_read_string(IDWeb * 8);
      Serial.println("web tot_points from EEPROM:" + points);

      cans_qty = eeprom_read_string(256 + (IDWeb * 8));
      Serial.println("web cansQty from EEPROM:" + cans_qty);



      return;
    }
  }

  Serial.print("IDWeb:");
  Serial.println(IDWeb);

  if (IDWeb == (MAXusers)) {
    points = "error";
    cans_qty = "error";

  }
}
void clearEEPROM() {
  for (int i = 0; i < 20; i++) {
    eeprom_write_string(i * 8, String(0));
    eeprom_write_string(256 + (i * 8), String(0));
    eeprom_write_string(eeprom_storage_address_plastic, String(0));
  }
}

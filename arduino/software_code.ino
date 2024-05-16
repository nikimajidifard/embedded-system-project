#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"

#define BUTTON_PIN 21  // GIOP21 pin connected to button
#define BUTTON_PIN_s 19  // GIOP21 pin connected to button
#define BUTTON_PIN_opt_ 23 // GIOP21 pin connected to button
// #define BUTTON_PIN2 19 //
#define BUTTON_PIN_opt 6
int old_zero_time = 0;  
int interval = 0;
bool state = true;  
const int max_interval = 20; 
int i = 0;


////////////
// WiFi
const char *ssid = "xxx"; // Enter your WiFi name
const char *password = "xxxxxxxx";  // Enter WiFi password
bool checkwifi = false; 
HardwareSerial mySoftwareSerial(1);
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic_ = "esp32/test";
const char *water_gas = "esp32/gas-water";
const char *electricity = "esp32/electricity";
const char *dfplayer = "esp32/dfplayer";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;
char dfplayermassage[2];
WiFiClient espClient;
PubSubClient client(espClient);
// Variables will change:
int lastState = LOW;  // the previous state from the input pin
int currentState;     // the current reading from the input pin
int lastState_w = LOW;  // the previous state from the input pin
int currentState_w;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
   // connecting to a WiFi network
 WiFi.begin(ssid, password);
  Serial.println("wifi..");
 while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.println("Connecting to WiFi..");
 }
 Serial.println("---Connected to the WiFi network ---");
 //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 while (!client.connected()) {
     String client_id = "esp32-client-";
     client_id += String(WiFi.macAddress());
     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
         Serial.println("Public emqx mqtt broker connected");
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }
 }
 // publish and subscribe
 client.publish(topic_, "Hi EMQX I'm ESP32 ^^");
 client.subscribe(topic_);

 client.publish(dfplayer, "Hi EMQX I'm ESP32 dfplayer   ^^");
 client.subscribe(dfplayer);
  // publish and subscribe

  // initialize the pushbutton pin as an pull-up input
  // the pull-up input pin will be HIGH when the switch is open and LOW when the switch is closed.
   mySoftwareSerial.begin(9600, SERIAL_8N1, 16, 17);  // speed, type, RX, TX
  //Serial.begin(115200);
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    
    Serial.println(myDFPlayer.readType(),HEX);
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  
  //----Set volume----
  myDFPlayer.volume(10);  //Set volume value (0~30).
  myDFPlayer.volumeUp(); //Volume Up
  myDFPlayer.volumeDown(); //Volume Down
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  int delayms=100;
  //----Mp3 play----

  //----Read imformation----
  Serial.println(F("readState--------------------"));
  Serial.println(myDFPlayer.readState()); //read mp3 state
  Serial.println(F("readVolume--------------------"));
  Serial.println(myDFPlayer.readVolume()); //read current volume
  Serial.println(F("readFileCounts--------------------"));
  Serial.println(myDFPlayer.readFileCounts()); //read all file counts in SD card
  Serial.println(F("readCurrentFileNumber--------------------"));
  Serial.println(myDFPlayer.readCurrentFileNumber()); //read current play file number
  Serial.println(F("readFileCountsInFolder--------------------"));
  Serial.println(myDFPlayer.readFileCountsInFolder(3)); //read fill counts in folder SD:/03
  Serial.println(F("--------------------"));
  //***
  Serial.println(F("myDFPlayer.play(1)"));
  myDFPlayer.play(1);  //Play the first mp3
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN_opt_, INPUT_PULLUP);
  pinMode(BUTTON_PIN_s, INPUT_PULLUP);
}
void callback(char *topic, byte *payload, unsigned int length) {
 Serial.print("Message arrived in topic: ");
 Serial.println(topic);
 Serial.print("Message:");
 for (int i = 0; i < length; i++) {
     Serial.print((char) payload[i]);
      
 }
  
 if(String(topic) =="esp32/dfplayer")
 {
   
    dfplayermassage[0] = (char) payload[0];
    
    // dfplayermassage[1] = (char) payload[1];
 
   
      Serial.println(dfplayermassage[0]);
     Serial.println(F("dfplayerstate--------------------"));
    Serial.println(dfplayermassage);
    if ( dfplayermassage[0] == 'n') {
      Serial.println(F("next--------------------"));
      myDFPlayer.next();
      Serial.println(myDFPlayer.readCurrentFileNumber()); //read current play file number
    } else if  ( dfplayermassage[0] == 'p') {
      Serial.println(F("previous--------------------"));
      myDFPlayer.previous();
      Serial.println(myDFPlayer.readCurrentFileNumber()); //read current play file number
    } else if (dfplayermassage[0] == '+') {
      Serial.println(F("up--------------------"));
      myDFPlayer.volumeUp();
      Serial.println(myDFPlayer.readVolume()); //read current volume
    } else if (dfplayermassage[0] == '-') {
      Serial.println(F("down--------------------"));
      myDFPlayer.volumeDown();
      Serial.println(myDFPlayer.readVolume()); //read current volume
    } else if (dfplayermassage[0] == 's') {
      Serial.println(F("pause--------------------"));
      myDFPlayer.pause();
    } else if (dfplayermassage[0] == '>') {
      Serial.println(F("start--------------------"));
      myDFPlayer.start();
    }
 }
      

 Serial.println();
 Serial.println("------------------xxx-------------------");
 Serial.println(dfplayermassage);
}

void loop() {
  // read the state of the switch/button:
  client.loop();
  currentState = digitalRead(BUTTON_PIN);
  currentState_w = digitalRead(BUTTON_PIN_s);
  if (lastState == HIGH && currentState == LOW)
  {
    Serial.println("The gas is turned of");
   client.publish("esp32/gas-water","gas is turned of");
 }
 else if (lastState == LOW && currentState == HIGH)
  {
    Serial.println("The gas is ok now" );
    client.publish("esp32/gas-water","gas is turned on");
  }
  if (lastState_w == HIGH && currentState_w == LOW)
  {
    Serial.println("The water is turned of");
     client.publish("esp32/gas-water","water is turned of");
  }
  else if (lastState_w == LOW && currentState_w == HIGH)
  {
    Serial.println("The water is ok now" );
     client.publish("esp32/gas-water","water is turned on");
  }
  // save the the last state
lastState = currentState;
lastState_w = currentState_w;  
  if(digitalRead( BUTTON_PIN_opt_) == 0)
  {
  Serial.println("the time start"); 
    old_zero_time = millis(); 
    if(!state)
    {
      Serial.println("everything is ok now");
      state = true; 
    }
  }
  
     interval = millis() - old_zero_time; 
    if (interval > max_interval)
    {
      if(state == true || i <= 10)
      {
        Serial.println("electricity is not working ");
        client.publish( "esp32/electricity", "the electricity is off right now");
      }

      state = false; 
      i++;  
    }
     

 if (myDFPlayer.available()) {
  if (myDFPlayer.readType()==DFPlayerPlayFinished) {
    Serial.println(myDFPlayer.read());
    Serial.println(F("next--------------------"));
     myDFPlayer.next();  //Play next mp3 every 3 second.
    Serial.println(F("readCurrentFileNumber--------------------"));
    Serial.println(myDFPlayer.readCurrentFileNumber()); //read current play file number
    delay(500);
  }
 }   
}


void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

#include <WiFi.h>
#include <PubSubClient.h>
#include <Ultrasonic.h>
/////////////ACTUATOR///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <ESP32Servo.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ACTUATOR_PIN 27

const char* mqtt_server = "192.168.0.100";
const char* pubTopic_status = "automatic_cabinet_door/status";
const char* pubTopic_distance = "automatic_cabinet_door/distance";
const char* subTopic_controls = "automatic_cabinet_door/controls";
const char *wifi_ssid = "SSID";
const char *wifi_password = "PASSWORD";

WiFiClient esp32Client;
PubSubClient client(esp32Client);

Ultrasonic ultrasonic(25, 26); //25 trig; 26 echo
int distance;
bool doorOpen=false;
int zoneTime=0;
int distPubTime=0;
const int delayTime = 100; //1 read every 100 milliseconds
const int distPubWait = 5000;//100; //1 pub every 5000 milliseconds
const int openZone = 130;//130 cm
const int safeZone = 60;//60 cm
int mode = 1; //1-AUTOMATIC_MODE; 2-MANUAL_MODE; 3-OPEN_MODE;
int lastMode = 1;
bool canOpen = false;
/////////////ACTUATOR///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Servo myActuator;
int actuatorPos;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long lastMsg = 0;

void setup_wifi() {

  randomSeed(micros());
  
  Serial.print("\n\r Initializing Wifi \n\r");
  
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(F("\n\r WiFi connected!"));
}

/////////////ACTUATOR///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void actuatorOpen(){  
  client.publish(pubTopic_status, "Door Opening");
  for(actuatorPos = 0; actuatorPos <= 360; actuatorPos+=1){
    myActuator.write(actuatorPos);
    delay(30); //15 ms to reach position
  }
  client.publish(pubTopic_status, "Door Open");
}

void actuatorClose(){
  client.publish(pubTopic_status, "Door Closing");
  for(actuatorPos = 360; actuatorPos >= 0; actuatorPos-=1){
    myActuator.write(actuatorPos);
    delay(30); //15 ms to reach position
  }
  client.publish(pubTopic_status, "Door Closed");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  char payloadChar[length+1];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int i=0;
  for (; i < length; i++) {
    payloadChar[i] = (char)payload[i]; 
    Serial.print(payloadChar[i]);
  }
  payloadChar[i] = '\0';
  Serial.println();

  if (strcmp(payloadChar, "AUTOMATIC_MODE")==0) {
    lastMode = mode;
    mode = 1;
    client.publish(pubTopic_status, "Automatic Mode On");
  } else if(strcmp(payloadChar, "MANUAL_MODE")==0){
    lastMode = mode;
    mode = 2;
    client.publish(pubTopic_status, "Manual Mode On");
  } else if(strcmp(payloadChar, "OPEN_MODE")==0){
    lastMode = mode;
    mode = 3;
    client.publish(pubTopic_status, "Open Mode On");
    actuatorOpen();
  } else if(strcmp(payloadChar, "OPEN")==0 && mode==2 && canOpen){
    Serial.println("openining");
    actuatorOpen();
  } else if(strcmp(payloadChar, "CLOSE")==0 && mode==3){
    Serial.println("closing");
    actuatorClose();
    Serial.println("Closed-----");
    mode = lastMode;
  }
  canOpen = false;
  delay(200);

}

void reconnectMQTT() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);//CHANGED ID HERE!
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(pubTopic_status, "ON");
      // ... and resubscribe
      client.subscribe(subTopic_controls);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {    // Initialize the BUILTIN_LED pin as an output
   // Bridge takes about two seconds to start up
  // it can be helpful to use the on-board LED
  // as an indicator for when it has initialized
/////////////ACTUATOR///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  myActuator.attach(ACTUATOR_PIN);
  actuatorClose();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Serial.print("\n\r Initializing Board \n\r");
  Serial.begin(9600);
  Serial.print("\n\r Wifi Call \n\r");
  setup_wifi();
  Serial.print("\n\r MQTT Call \n\r");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callbackMQTT);
  Serial.print("\n\r End Setup \n\r");
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
 
  distance = ultrasonic.read();  
  
  if(distPubTime > distPubWait){
    Serial.print("\n\r Distance: ");
    Serial.print(distance);
    Serial.print(" cm\n\r");
    char d[5]; String str = String(distance);
    const char * payload = str.c_str();
    client.publish(pubTopic_distance, payload);
    distPubTime=0;
  }
  else
    distPubTime += delayTime;
  
  if(mode == 1 || mode == 2){
    if(doorOpen){
      if(distance > openZone)
        zoneTime += delayTime;
      else
        zoneTime = 0;
      if(zoneTime >= 4000){
        Serial.println("Closing Door");
        doorOpen = false;
        zoneTime = 0;
        if(mode == 2)
          canOpen = false;
/////////////ACTUATOR///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        actuatorClose();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      }
    }
    else{
      if(distance < openZone)
        if(distance > safeZone)
          zoneTime += delayTime;
      else
        zoneTime = 0;
      if(zoneTime >= 1000){
        Serial.println("Opening Door");
        doorOpen = true;
        zoneTime = 0;
        if(mode == 2)
          canOpen = true;
        else{
/////////////ACTUATOR///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          actuatorOpen();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }
      }
    }
  }
  delay(delayTime);
}

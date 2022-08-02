#include "Arduino.h"

#include <Bridge.h>
#include <BridgeClient.h>
#include <MQTT.h>
#include <Process.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

//grupo 1-Sesnsores
int SOIL1 = A4; //topico 1.1.1
int SOIL2 = A5; //topico 1.1.2
int LDR1 = A2; // topico 1.2.1
int LDR2 = A3; //topico 1.2.2
int LM35 = A1; // topico 1.3.1
int DHTPIN = 8; // topico 1.3.2-1.4

float soil_1_1_1 = 0;
float soil_1_1_2 = 0;
float LDR_1_2_1 = 0;
float LDR_1_2_2 = 0;
float temperature1_3_1 = 0.0;
float temperature1_3_2 = 0.0;
float humity_1_4 = 0.0;
String topic = "/buzzer1_topic";
String payload = "";
//grupo atuadores
//conetados ao PI
//grupo 2

#define DHTTYPE  DHT11     // topico 1.3.2
DHT_Unified dht(DHTPIN, DHTTYPE);
BridgeClient net;
MQTTClient client;
String humity;

unsigned long lastMillis = 0;
uint32_t delayMS;

//moisture sensors
float slope = 2.48; // slope from linear fit
float intercept = -0.72; // intercept from linear fit
#define VREF_3_3  3.3

//
#define VREF_PLUS  5
#define VREF_MINUS  0
#define RESOLUTION 1023

void connect() {
	String payload;
	Serial.print("connecting...");
	while (!client.connect("10.20.228.23", "Rafaeljff", "ciic")) {
		Serial.print(".");
		delay(1000);
	}

	Serial.println("\nconnected!");

	client.subscribe(topic);
	client.onMessage(callback);
}

void callback(String &topic, String &payload)
{


	  Serial.println("sssssssssssssssssssssssssssssss");
    Serial.println(topic);
    Serial.println(payload);
}

void setup() {
	Process wifiCheck;
	Bridge.begin();
	Serial.begin(115200);
	char c = wifiCheck.read();
	Serial.print(c);
	// Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
	// by Arduino. You need to set the IP address directly.
	client.begin("10.20.228.23", 1883, net);
//	client.subscribe("/buzzer1_topic",2);
	//client.onMessage(messageReceived);
	connect();
	//analog reference pin

	pinMode(SOIL1, INPUT);
	pinMode(SOIL2, INPUT);
	pinMode(LDR1, INPUT);
	pinMode(LDR2, INPUT);
	pinMode(LM35, INPUT);

	//DHT11 temperature and humity sensors
	dht.begin();
	//analogReference(0); // set the analog reference to 3.3V
	sensor_t sensor;
	dht.temperature().getSensor(&sensor);
	dht.humidity().getSensor(&sensor);
	delayMS = sensor.min_delay / 1000;
}

void loop() {
	Serial.println("New Loop");
	Process wifiCheck;
	String t;
	lastMillis = millis();
	wifiCheck.runShellCommand("/usr/bin/pretty-wifi-info.lua"); //
	char c = wifiCheck.read();
	Serial.print(c);
	client.loop();


	//client.returnCode();

	if (!client.connected()) {
		connect();
	}


	temperature_humity();
	LDR();
	soil_moisture();
	temperature_LM35();
	Serial.print("\nSoil Moisture 1:topic 1.1.1: ");
	Serial.print(soil_1_1_1);

	Serial.print("\nSoil Moisture 2:topic 1.1.2: ");
	Serial.print(soil_1_1_2);
	Serial.print("\nLDR 1:topic 1.2.1: ");
	Serial.print(LDR_1_2_1);
	Serial.print("\nLDR 2:topic 1.2.2: ");
	Serial.print(LDR_1_2_2);
	Serial.print("\nTemperature 1:topic 1.3.1: ");
	Serial.print(temperature1_3_1);
	Serial.print("\nTemperature 2:topic 1.3.2: ");
	Serial.print(temperature1_3_2);
	Serial.print("\nHumity:topic 1.4: ");
	Serial.print(humity_1_4);

	if (wifiCheck.available() > 0) {

		if (client.connected()) {

			String publish = "";
			publish.concat(soil_1_1_1);

			client.publish("topic1/1/1", publish);
			publish = "";
			publish.concat(soil_1_1_2);
			client.publish("topic1/1/2", publish);
			publish = "";
			publish.concat(LDR_1_2_1);
			client.publish("topic1/2/1", publish);
			publish = "";
			publish.concat(LDR_1_2_2);
			client.publish("topic1/2/2", publish);
			publish = "";
			publish.concat(temperature1_3_1);
			client.publish("topic1/3/1", publish);
			publish = "";
			publish.concat(temperature1_3_2);
			client.publish("topic1/3/2", publish);
			publish = "";
			publish.concat(humity_1_4);
			client.publish("topic1/4/0", publish);
client.subscribe(topic);
client.onMessageAdvanced(callback);
		}
	}

	delay(4000);
}

void temperature_humity() {
	temperature1_3_2 = 0;
	humity_1_4 = 0;
	sensor_t sensor;
	sensors_event_t event;
	dht.temperature().getEvent(&event);

	if (isnan(event.temperature)) {
		temperature1_3_2 = 0;

	} else {

		temperature1_3_2 = event.temperature;
	}
	// Get humidity event and print its value.
	dht.humidity().getEvent(&event);
	if (isnan(event.relative_humidity)) {
		humity_1_4 = 0;
	} else {

		humity_1_4 = event.relative_humidity;
	}
	delayMS = sensor.min_delay / 1000;
}

void LDR() {
	LDR_1_2_1 = 0;
	LDR_1_2_2 = 0;
	LDR_1_2_1 = (1023 - float(analogRead(LDR1))) / 10;
	LDR_1_2_2 = (1023 - float(analogRead(LDR2))) / 10;
	;

}

void soil_moisture() {
	soil_1_1_1 = 0;
	soil_1_1_2 = 0;
	float voltage_1;
	float voltage_2;

	voltage_1 = (float(analogRead(SOIL1)) / RESOLUTION) * VREF_3_3;
	voltage_2 = (float(analogRead(SOIL2)) / RESOLUTION) * VREF_3_3;

	soil_1_1_1 = ((1.0 / voltage_1) * slope) + intercept; // calc of theta_v (vol. water content)volumetric water content
	soil_1_1_2 = ((1.0 / voltage_2) * slope) + intercept; // calc of theta_v (vol. water content)

}
void temperature_LM35() {
	temperature1_3_1 = 0;
	temperature1_3_1 = (float(analogRead(LM35)) * VREF_PLUS / (RESOLUTION))
			/ 0.1;
	//temperature1_3_1 = (float(analogRead(LM35)) * 0.1039); // get temp
}

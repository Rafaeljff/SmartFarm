#include "Arduino.h"

#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <math.h>
#include <WiFiUdp.h>
#include <coap-simple.h>
#define ONE_WIRE_BUS 4
#define SOIL 34
float temperature_DHT = 0;
float temperature_ds18B20 = 0;
float temperature_KY028 = 0;
float dht11_t = 0;
float dht11_hum = 0;
float moisture = 0;
DHT_Unified dht11(5, DHT11);
// define the LED pin
int digitalPinKY = 25; // KY-028 digital interface
int analogPinKY = 35; // KY-028 analog interface
int digitalVal; // digital readings
int analogVal; //analog readings
#define server=IPAddress(0x0A,0x14,0xE4,0xF7);
//pinMode(analogPin, OUTPUT);

float slope = 2.48; // slope from linear fit
float intercept = -0.72; // intercept from linear fit

const char *ssid = "labs";
const char *password = "robot1cA!ESTG";
const char *address = "http://10.20.228.80:1880/dht11";
uint32_t delayMS;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

WiFiClient Wificlient;
PubSubClient client(Wificlient);
HTTPClient http;
WiFiUDP udp;
Coap coap(udp);

#define server=IPAddress(0x0A,0x14,0xE4,0xF7);

void setup() {

	WiFi.disconnect();
	Serial.begin(115200);
	dht11.begin();

	//while (!status) {

	delay(1000);
	//}

	pinMode(SOIL, INPUT);
	pinMode(analogPinKY, OUTPUT);
	pinMode(digitalPinKY, INPUT);

	WiFi.begin(ssid, password);
	Serial.println("Connecting to wifi...");
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print("still not connected to wifi");
	}

	Serial.print("Connected to WiFi network with IP Address: ");
	sensor_t sensor11;
	dht11.temperature().getSensor(&sensor11);
	dht11.humidity().getSensor(&sensor11);
	delayMS = sensor11.min_delay / 1000;
	coap.start();
}

// The loop function is called in an endless loop
void loop() {
	/*String sender = "";
	char make_char[10];*/
	BME();
	DS18B20();
	soil_moisture();
	ky028();
	Serial.print("Temperature DHT11: ");
	Serial.println(dht11_t);
	Serial.print("Temperature 2: ");
	Serial.println(temperature_ds18B20);
	Serial.print("Temperature 3: ");
	Serial.println(temperature_KY028);
	Serial.print("Soil moisture: ");
	Serial.print(moisture);
	Serial.println("Humity DHT11: ");
	Serial.print(dht11_hum);

	/*	if (WiFi.status() == WL_CONNECTED) { //falta condicao de selecao

	 http.begin(address);

	 http.POST("MERDA");
	 http.end();

	 }
	 */
	/*sender.concat(dht11_t);
	sender.toCharArray(make_char, 10);
	/*int msg_send = coap.put(IPAddress(0x0A, 0x14, 0xE4, 0xF7), 5683,
			"manager", make_char);*/
	/*
	sender = "";
	sender.concat(temperature_ds18B20);
	sender.toCharArray(make_char, 10);
	int msg_send1 = coap.put(IPAddress(0x0A, 0x14, 0xE4, 0xF7), 5683,
			"manager_temp2", make_char);
	sender = "";
	sender.concat(temperature_KY028);
	sender.toCharArray(make_char, 10);
	int msg_send2 = coap.put(IPAddress(0x0A, 0x14, 0xE4, 0xF7), 5683,
			"manager_temp3", make_char);
	sender = "";
	sender.concat(dht11_hum);
	sender.toCharArray(make_char, 10);
	int msg_send3 = coap.put(IPAddress(0x0A, 0x14, 0xE4, 0xF7), 5683,
			"manager_hum", make_char);
	sender = "";
	sender.concat(moisture);
	sender.toCharArray(make_char, 10);
	int msg_send4 = coap.put(IPAddress(0x0A, 0x14, 0xE4, 0xF7), 5683,
			"manager_soil", make_char);*/
	delay(5000);

}
void BME() {

	dht11_t = 0;
	dht11_hum = 0;
	sensor_t sensor11;
	sensors_event_t event;
	dht11.temperature().getEvent(&event);

	if (isnan(event.temperature)) {
		dht11_t = -1;

	} else {

		dht11_t = event.temperature;
	}
	// Get humidity event and print its value.
	dht11.humidity().getEvent(&event);
	if (isnan(event.relative_humidity)) {
		dht11_hum = -1;
	} else {

		dht11_hum = event.relative_humidity;
	}
	delayMS = sensor11.min_delay / 1000;
}
void DS18B20() {
	sensors.requestTemperatures();
	temperature_ds18B20 = sensors.getTempCByIndex(0);

}

void soil_moisture() {
	moisture = 0;
	float voltage;
	voltage = (float(analogRead(SOIL)) / 1024) * 5;
	moisture = ((1.0 / voltage) * slope) + intercept; // calc of theta_v (vol. water content) volumetric water content

}

void ky028() {
	digitalVal = digitalRead(digitalPinKY);
	analogVal = analogRead(analogPinKY);

	temperature_KY028 = log(((10240000 / analogVal) - 10000));
	temperature_KY028 = 1
			/ (0.001129148 + (0.000234125 * temperature_KY028)
					+ (0.0000000876741 * temperature_KY028 * temperature_KY028
							* temperature_KY028));
	temperature_KY028 = temperature_KY028 - 273.15;

}


#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#ifndef STASSID
#define STASSID "labs"
#define STAPSK  "robot1cA!ESTG"
#endif

const char *http_dht11_t = "http://10.20.228.23:1880/dht11_t";
const char *http_dht22_t = "http://10.20.228.23:1880/dht22_t";
const char *http_dht11_h = "http://10.20.228.23:1880/dht11_h";
const char *http_dht22_h = "http://10.20.228.23:1880/dht22_h";
const char *http_DS18b20 = "http://10.20.228.23:1880/ds18b20";
const char *http_analog = "http://10.20.228.23:1880/analog_t";
const char *http_actuator = "http://10.20.228.23:1880/check";

const char *ssid = STASSID;
const char *password = STAPSK;
const int analogInPin = A0;
#define ONE_WIRE_BUS 14

float dht11_t = 0.0;
float dht22_temp = 0.0;
float ds18b20_temp = 0.0;
float analog_temp = 0.0;
float dht11_hum = 0;
float dht22_hum = 0;
int buzzer = D7;
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);
DHT_Unified dht11(4, DHT11);
DHT_Unified dht22(5, DHT22);
uint32_t delayMS;
uint32_t delayMS_2;
WiFiClient client;
float R1 = 10000; // value of R1 on board
float logR2, R2, T;
float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741; //steinhart-hart coeficients for thermistor
volatile unsigned long ul;
volatile unsigned long millis_counter;

HTTPClient http;

void setup() {
	Serial.begin(115200);

	WiFiClient client;
	HTTPClient http;

	sensors.begin();
	dht11.begin();
	dht22.begin();
	pinMode(buzzer, OUTPUT);
	digitalWrite(buzzer, LOW);
	sensor_t sensor11;
	dht11.temperature().getSensor(&sensor11);
	dht11.humidity().getSensor(&sensor11);
	delayMS = sensor11.min_delay / 1000;

	sensor_t sensor22;
	dht22.temperature().getSensor(&sensor22);
	dht22.humidity().getSensor(&sensor22);
	delayMS_2 = sensor22.min_delay / 1000;

	pinMode(analogInPin, INPUT);

	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print("still not connected to wifi");
	}
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void loop() {

	get_measures();
	send_http();
	http_request();
}

void DS18B20() {
	sensors.requestTemperatures();
	ds18b20_temp = sensors.getTempCByIndex(0);

}
void dht_11_temperature_humity() {
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

void dht_22_temperature_humity() {
	dht22_temp = 0;
	dht22_hum = 0;
	sensor_t sensor22;
	sensors_event_t event;
	dht22.temperature().getEvent(&event);

	if (isnan(event.temperature)) {
		dht22_temp = 0;

	} else {

		dht22_temp = event.temperature;
	}
	// Get humidity event and print its value.
	dht22.humidity().getEvent(&event);
	if (isnan(event.relative_humidity)) {
		dht22_hum = 0;
	} else {

		dht22_hum = event.relative_humidity;
	}
	delayMS_2 = sensor22.min_delay / 1000;
}
void analog_temperature() {
	R2 = R1 * (1023.0 / (float) analogRead(analogInPin) - 1.0); //calculate resistance on thermistor
	logR2 = log(R2);
	T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2)); // temperature in Kelvin
	T = T - 273.15;
	analog_temp = T;

}
void get_measures() {
	DS18B20();
	dht_11_temperature_humity();
	dht_22_temperature_humity();
	analog_temperature();
	Serial.println("\nTemperature of DHT11:");
	Serial.print(dht11_t);
	Serial.println("\nTemperature of DHT22:");
	Serial.print(dht22_temp);
	Serial.println("\nTemperature of DS18B20:");
	Serial.print(ds18b20_temp);
	Serial.println("\nTemperature of analog:");
	Serial.print(analog_temp);

	Serial.println("\nHumity of DHT11:");
	Serial.print(dht11_hum);

	Serial.println("\nHumity of DHT22:");
	Serial.print(dht22_hum);

}
void send_http() {

	/*String httpResponseCode = ("{\"Temperature_DHT11\":\"" + DHT11_TEMP
	 + "\",\"humity_DHT11\":\"" + DHT11_HUM + "\"}");*/

	String DHT11_TEMP = String(dht11_t, 3);
	String DHT11_HUM = String(dht11_hum, 3);
	String DHT22_TEMP = String(dht22_temp, 3);
	String DHT22_HUM = String(dht22_hum, 3);
	String ds18b20 = String(ds18b20_temp, 3);
	String analog = String(analog_temp, 3);

	http.begin(client, http_dht11_t);
	http.POST(DHT11_TEMP);
	http.end();
	millis_counter = millis();
	for (ul = millis_counter; ul < millis_counter + 200; ul = millis())
		;
	http.begin(client, http_dht22_t);
	http.POST(DHT22_TEMP);
	http.end();

	millis_counter = millis();
	for (ul = millis_counter; ul < millis_counter + 200; ul = millis())
		;

	http.begin(client, http_dht11_h); //HTTP
	http.POST(DHT11_HUM);
	http.end();

	millis_counter = millis();
	for (ul = millis_counter; ul < millis_counter + 200; ul = millis())
		;

	http.begin(client, http_DS18b20); //HTTP
	http.POST(ds18b20);
	http.end();

	millis_counter = millis();
	for (ul = millis_counter; ul < millis_counter + 200; ul = millis())
		;

	http.begin(client, http_analog); //HTTP
	http.POST(analog);
	http.end();

	millis_counter = millis();
	for (ul = millis_counter; ul < millis_counter + 200; ul = millis())
		;

	http.begin(client, http_dht22_h); //HTTP
	http.POST(DHT22_HUM);
	http.end();

	millis_counter = millis();
	for (ul = millis_counter; ul < millis_counter + 200; ul = millis())
		;

}

void http_request() {
	http.begin(client, http_actuator); //HTTP
	// httpCode will be negative on error
	int httpcode = http.GET();
	if (httpcode > 0) {
// HTTP header has been send and Server response header has been handled
		Serial.printf("[HTTP] POST... code: %d\n", httpcode);

// file found at server
		if (httpcode == HTTP_CODE_OK) {
			const String &payload = http.getString();
			Serial.println("received payload:\n<<");
			Serial.println(payload);
			Serial.println(">>");
			if (payload == "actuator") {
				http.end();
				digitalWrite(buzzer, HIGH);	// send voice

			} else {

				digitalWrite(buzzer, LOW);	// send voice

			}
		}

	} else {
		Serial.printf("[HTTP] POST... failed, error: %s\n",
				http.errorToString(httpcode).c_str());
	}
	http.end();

}

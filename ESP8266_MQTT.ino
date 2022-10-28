#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <PubSubClient.h>
Adafruit_BMP280 bmp; // use I2C interface
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

#ifndef STASSID
#define STASSID "labs"
#define STAPSK  "robot1cA!ESTG"
/*#define STASSID "NOS-F1A1"
 #define STAPSK  "5FYKEEYY"
 */
/*
#define STASSID "labs"
#define STAPSK  "Om3YMdaf"*/
#define green D5
#define red  D6
#endif

const char *ssid = STASSID;
const char *password = STAPSK;
const char *mqtt_server = "10.20.228.247";

float temperature;
float pressure;
float altitude;
const char *subscription = "mqtt_2_response";
const char *temp_topic = "topic2/1";
const char *pressure_topic = "topic2/5";
const char *altitude_topic = "topic2/6";
WiFiClient client_wifi;
PubSubClient client(client_wifi);
void setup() {
	Serial.begin(115200);
	bmp.begin(0x76);
	if (!bmp.begin(0x76)) {
		Serial.println(
				F("Could not find a valid BME280 sensor, check wiring!"));

		delay(10);
	}
	pinMode(green, OUTPUT);
	pinMode(red, OUTPUT);
	bmp_temp->printSensorDetails();
	bmp_pressure->printSensorDetails();

	//WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print("still not connected to wifi");
	}
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
	client.setServer(mqtt_server, 1883);
	//caso de autentica��o utilizar client.connect
	client.subscribe(subscription);
	client.setCallback(callback);
}

void loop() {

	Serial.print(F("Temperature = "));
	Serial.print(bmp.readTemperature());
	Serial.println("*C");
	temperature = bmp.readTemperature();
	Serial.print(F("Pressure = "));
	Serial.print(bmp.readPressure() / 1000);
	Serial.println("kPa");
	pressure = bmp.readPressure() / 1000;

	Serial.print(F("Approx altitude = "));
	Serial.print(bmp.readAltitude(1015)); /* Adjusted to local forecast! */
	Serial.println("m");
	altitude = bmp.readAltitude(1015);
	Serial.println();

	if (!client.connected()) {
		reconnect();
	}
	if (!client.loop()) {
		client.connect("MQTT_2");
	}
	publish_topics();

	delay(1000);
}
// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte *message, unsigned int length) {
	String payload;
	Serial.print("Message received on: ");
	Serial.print(topic);
	Serial.print(". Message: ");

	for (int i = 0; i < length; i++) {
		Serial.print((char) message[i]);
		payload += (char) message[i];
	}
	Serial.println();
	if (topic == "mqtt_2_response") {

		if (payload == "actuatorON") {
			digitalWrite(red, HIGH);
			digitalWrite(green, LOW);
		} else if (payload == "actuatorOFF") {
			digitalWrite(green, HIGH);
			digitalWrite(red, LOW);
		}
	}
	Serial.println();
}
void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		/*
		 YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
		 To change the ESP device ID, you will have to give a new name to the ESP8266.
		 Here's how it looks:
		 if (client.connect("ESP8266Client")) {
		 You can do it like this:
		 if (client.connect("ESP1_Office")) {
		 Then, for the other ESP:
		 if (client.connect("ESP2_Garage")) {
		 That should solve your MQTT multiple connections problem
		 */
		if (client.connect("MQTT_2")) {
			Serial.println("connected");
			// Subscribe or resubscribe to a topic
			// You can subscribe to more topics (to control more LEDs in this example)
			client.subscribe(subscription);
		} else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}
void publish_topics() {
	char char_temperature[5];
	char char_pressure[5];
	char char_altitude[7];
	String temp_string = String(temperature, 5);
	String pressure_string = String(pressure, 5);
	String altitude_string = String(altitude, 7);
	temp_string.toCharArray(char_temperature, 5);
	pressure_string.toCharArray(char_pressure, 5);
	altitude_string.toCharArray(char_altitude, 7);

	client.publish(temp_topic, char_temperature);
	//New topic 5 of mqtt device 2 for pressure

	client.publish(pressure_topic, char_pressure);

	//New topic 6 of mqtt device 2 for altitude

	client.publish(altitude_topic, char_altitude);
	//humity of mqtt device 2

}

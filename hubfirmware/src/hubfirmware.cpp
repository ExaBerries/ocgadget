#include <Arduino.h>
#include <exaocbot_usb_protocol.h>

constexpr int LED = 17;
int data = 0;
int time = 300;

void setup() noexcept {
	pinMode(LED, OUTPUT);
	Serial.begin(115200);
}

void loop() noexcept {
	if (Serial.available() > 0) {
		data = Serial.read();
		time = data;
	}
	digitalWrite(LED, HIGH);
	delay(time);
	digitalWrite(LED, LOW);
	delay(1600);
	Serial.println("test");
}

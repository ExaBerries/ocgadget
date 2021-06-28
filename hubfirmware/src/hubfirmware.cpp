#include <Arduino.h>
#include <exaocbot_usb_protocol.h>

constexpr int LED = 17;
int delay = 10;
int data = 0;

void setup() noexcept {
	pinMode(LED, OUTPUT);
	Serial.begin(9600);
}

void loop() noexcept {
	if (Serial.avaliable() > 0) {
		data = Serial.read();
		Serial.print(data, DEC);
	}
	digitalWrite(LED, HIGH);
	delay(1600);
	digitalWrite(LED, LOW);
	delay(60);
}

const int MOISTURE_PIN = A0;

void setup() {
    Serial.begin(9600);
    Serial.println(F("Calibration -- reading every second"));
}

void loop() {
    Serial.print(F("Raw ADC: "));
    Serial.println(analogRead(MOISTURE_PIN));
    delay(1000);
}

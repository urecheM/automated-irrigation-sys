/*
 * Automated Irrigation System
 * Hardware: Elegoo UNO R3, Capacitive Soil Moisture Sensor,
 *           1-Channel 5V Relay Module, Mini Submersible DC Pump
 *
 * Wiring:
 *   Moisture Sensor  -> A0 (AOUT), 5V (VCC), GND
 *   Relay IN         -> D7
 *   Relay VCC        -> 5V, Relay GND -> GND
 *   Pump +           -> Relay COM, Pump - -> external GND
 *   External 3-4.5V  -> Relay NO (normally-open)
 */

// Pin assignments
const int MOISTURE_PIN  = A0;
const int RELAY_PIN     = 7;

// Calibration
// Read sensor in dry air -> DRY_VALUE; submerged in water -> WET_VALUE.
// Adjust these after running the calibration sketch.
const int DRY_VALUE     = 620;   // raw ADC when sensor is in dry soil/air
const int WET_VALUE     = 280;   // raw ADC when sensor is in water

// Thresholds (0-100%, higher = wetter)
const int PUMP_ON_THRESHOLD  = 30;  // start pump below this moisture %
const int PUMP_OFF_THRESHOLD = 60;  // stop  pump above this moisture %

// Timing
const unsigned long READ_INTERVAL_MS  = 10000UL;  // check moisture every 10 s
const unsigned long MAX_PUMP_ON_MS    = 5000UL;   // safety: never run > 5 s straight
const unsigned long MIN_PUMP_OFF_MS   = 30000UL;  // wait 30 s between pump cycles

// State
enum PumpState { PUMP_IDLE, PUMP_RUNNING };
PumpState pumpState = PUMP_IDLE;

unsigned long lastReadTime    = 0;
unsigned long pumpStartTime   = 0;
unsigned long pumpStopTime    = 0;

// Relay helpers (LOW = relay energised on most modules)
void pumpOn()  { digitalWrite(RELAY_PIN, LOW);  pumpState = PUMP_RUNNING; pumpStartTime = millis(); }
void pumpOff() { digitalWrite(RELAY_PIN, HIGH); pumpState = PUMP_IDLE;   pumpStopTime  = millis(); }

void setup() {
    Serial.begin(9600);
    pinMode(RELAY_PIN, OUTPUT);
    pumpOff();
    Serial.println(F("Automated Irrigation System -- ready"));
    Serial.println(F("Format: [time_ms] moisture=XX% raw=YYYY pump=ON|OFF"));
}

void loop() {
    unsigned long now = millis();

    // Safety cut-off: never run the pump longer than MAX_PUMP_ON_MS
    if (pumpState == PUMP_RUNNING && (now - pumpStartTime >= MAX_PUMP_ON_MS)) {
        Serial.println(F("Safety timeout -- pump off"));
        pumpOff();
    }

    // Read moisture on interval
    if (now - lastReadTime >= READ_INTERVAL_MS) {
        lastReadTime = now;

        int raw      = analogRead(MOISTURE_PIN);
        int moisture = map(raw, DRY_VALUE, WET_VALUE, 0, 100);
        moisture     = constrain(moisture, 0, 100);

        Serial.print(F("["));
        Serial.print(now);
        Serial.print(F("] moisture="));
        Serial.print(moisture);
        Serial.print(F("% raw="));
        Serial.print(raw);
        Serial.print(F(" pump="));
        Serial.println(pumpState == PUMP_RUNNING ? F("ON") : F("OFF"));

        bool cooldownDone = (now - pumpStopTime >= MIN_PUMP_OFF_MS);

        if (pumpState == PUMP_IDLE && moisture < PUMP_ON_THRESHOLD && cooldownDone) {
            Serial.println(F("Soil dry -- starting pump"));
            pumpOn();
        } else if (pumpState == PUMP_RUNNING && moisture >= PUMP_OFF_THRESHOLD) {
            Serial.println(F("Soil wet enough -- stopping pump"));
            pumpOff();
        }
    }
}

#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "your-ssid";
const char *password = "your-password";

const int digitalPinD1 = 5;  // D1 on ESP32

int ledState = LOW;

AsyncWebServer server(80);

int adc_max = 760;
int adc_min = 261;

float volt_multi = 231;
float volt_multi_p;
float volt_multi_n;

void setup() {
  Serial.begin(115200);
  pinMode(digitalPinD1, OUTPUT);

  volt_multi_p = volt_multi * 1.4142;
  volt_multi_n = -volt_multi_p;

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Route to serve HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body>";
    html += "<h1>Analog Values</h1>";
    html += "<p>Voltage: " + String(get_voltage()) + " V</p>";
    html += "<p>Current: " + String(get_current()) + " A</p>";
    html += "<h2>Digital Pin D1</h2>";
    html += "<p>State: " + String(ledState) + "</p>";
    html += "<form action='/toggle' method='post'><input type='submit' value='Toggle'></form>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  // Route to handle toggle button press
  server.on("/toggle", HTTP_POST, [](AsyncWebServerRequest *request){
    ledState = (ledState == LOW) ? HIGH : LOW;
    digitalWrite(digitalPinD1, ledState);
    request->send(200, "text/plain", "Toggle success");
  });

  // Start server
  server.begin();
}

void loop() {
  // Your main loop code here
}

float get_current() {
  unsigned int x = 0;
  float AcsValue = 0.0, Samples = 0.0, AvgAcs = 0.0, AcsValueF = 0.0;

  for (int x = 0; x < 150; x++) { // Get 150 samples
    AcsValue = analogRead(34);    // Read current sensor values
    Samples = Samples + AcsValue; // Add samples together
    delay(3);                     // let ADC settle before next sample 3ms
  }
  AvgAcs = Samples / 150.0;        // Taking Average of Samples

  AcsValueF = (2.5 - (AvgAcs * (5.0 / 1024.0))) / 0.066;

  return AcsValueF;
}

float get_voltage() {
  float adc_sample;
  float volt_inst = 0;
  float sum = 0;
  float volt;
  long init_time = millis();
  int N = 0;

  while ((millis() - init_time) < 500) { // Duration of 0.5 seconds (Approximately 30 cycles of 60Hz)
    adc_sample = analogRead(A0);          // Sensor voltage
    volt_inst = map(adc_sample, adc_min, adc_max, volt_multi_n, volt_multi_p);
    sum += sq(volt_inst);                  // Sum of Squares
    N++;
    delay(1);
  }

  volt = sqrt(sum / N); // RMS equation
  return volt;
}

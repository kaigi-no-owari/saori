#include <ESP8266WiFi.h>
#include <Servo.h>

// Wi-Fiアクセスポイントの情報

const char* ssid     = "SSID is required";
const char* password = "PASS is required";

// APIエンドポイントの情報

const char* host     = "API server hostname is required";
const int port       = 80;
const char* roomid   = "ROOM ID is required";
const String url     = String("/api/meetings/") + roomid + "/pow";
const int pollingIntervalSec = 30;

// サーボモーターの情報
const int servoPin   = 12;
const int servoDelay = 20;
const int repeatCount = 5;
Servo servo;


void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  servo.attach(servoPin);
  servo.write(90);
}

int value = 0;
boolean wasNotified = false;

void loop() {
  delay(pollingIntervalSec * 1000);
  ++value;

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = port;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
    Serial.println(line);
  }

  String line = client.readStringUntil('\n');
  Serial.println();
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");

  if (line.startsWith("{\"notification\":true}")) {
    if (!wasNotified) {
      Serial.println("The meeting will soon be over!");
      for (int angle = 90; angle >= 20; angle--) {
        servo.write(angle);
        delay(servoDelay);
      }
    }
    wasNotified = true;
    // サーボモーターを回転させてハトを出す
    for (int i = 0; i < repeatCount; i++) {
      for (int angle = 20; angle <= 90; angle++) {
        servo.write(angle);
        delay(servoDelay);
      }
      for (int angle = 90; angle >= 20; angle--) {
        servo.write(angle);
        delay(servoDelay);
      }
    }
  } else {
    if (wasNotified) {
      Serial.println("The meeting has been over!");
      wasNotified = false;
      // サーボモーターを回転させてハトを帰す
      for (int angle = 20; angle <= 90; angle++) {
        servo.write(angle);
        delay(servoDelay);
      }
    }
  }

}

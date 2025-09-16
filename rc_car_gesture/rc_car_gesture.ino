#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// WiFi Credentials (ESP32 creates its own network)
const char* ssid = "GestureCar";  // Hotspot name
const char* password = "12345678"; // Password (8+ chars)

// Motor Control Pins (L298N driver)
#define ENA 13  // PWM for Motor A speed
#define IN1 12  // Motor A direction 1
#define IN2 14  // Motor A direction 2
#define IN3 27  // Motor B direction 1
#define IN4 26  // Motor B direction 2
#define ENB 25  // PWM for Motor B speed
int speedy = 100;

// WebSocket Server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");  // WebSocket endpoint

// ================================================
// SETUP: Runs once at startup
// ================================================
void setup() {
  Serial.begin(115200);
  
  // 1. Configure Motor Pins as Outputs
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  stopMotors(); // Initialize motors as stopped

  // 2. Create WiFi Access Point
  WiFi.softAP(ssid, password);
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

  // 3. Configure WebSocket Server
  ws.onEvent(onWebSocketEvent); // Attach event handler
  server.addHandler(&ws);       // Register WebSocket
  
  // 4. Start Web Server
  server.begin();
}

// ================================================
// MAIN LOOP: Runs repeatedly
// ================================================
void loop() {
  ws.cleanupClients(); // Remove disconnected clients
}

// ================================================
// WEBSOCKET EVENT HANDLER
// ================================================
void onWebSocketEvent(AsyncWebSocket *server, 
                     AsyncWebSocketClient *client, 
                     AwsEventType type,
                     void *arg, 
                     uint8_t *data, 
                     size_t len) {
                     
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("Client #%u connected\n", client->id());
      break;
      
    case WS_EVT_DISCONNECT:
      Serial.printf("Client #%u disconnected\n", client->id());
      break;
      
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
      
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

// ================================================
// PROCESS INCOMING WEBSOCKET MESSAGES
// ================================================
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  
  // Only process complete text messages
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0; // Null-terminate the data
    String command = String((char*)data);
    Serial.print("Received: ");
    Serial.println(command);

    // Execute command
    if (command == "forward") moveForward();
    else if (command == "stop") stopMotors();
    else if (command == "left") turnLeft();
    else if (command == "right") turnRight();
    else if (command == "backward") moveBackward();
  }
}

// ================================================
// MOTOR CONTROL FUNCTIONS
// ================================================
void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speedy); // 200/255 speed
  analogWrite(ENB, speedy);
  Serial.println("Moving FORWARD");
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speedy);
  analogWrite(ENB, speedy);
  Serial.println("Moving BACKWARD");
}

void turnRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speedy);
  analogWrite(ENB, speedy);
  Serial.println("Turning LEFT");
}

void turnLeft() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speedy);
  analogWrite(ENB, speedy);
  Serial.println("Turning RIGHT");
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  Serial.println("STOPPED");
}

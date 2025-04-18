char *BLYNK_TEMPLATE_ID = getenv("BLYNK_TEMPLATE_ID");
char *BLYNK_TEMPLATE_NAME = getenv("BLYNK_TEMPLATE_NAME");
char *BLYNK_AUTH_TOKEN = getenv("BLYNK_AUTH_TOKEN");

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <UrlEncode.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

char *ssid = getenv("WIFI_SSID");
char *pass = getenv("WIFI_PASSWORD");

String phoneNumber = getenv("PHONE_NUMBER");
String apiKey = getenv("API_KEY");

#define SS_PIN D8
#define RST_PIN D4
MFRC522 mfrc522(SS_PIN, RST_PIN);
byte authorizedUID[] = {0xC3, 0x9F, 0x20, 0x03};

#define SERVO_PIN D3

Servo gateServo;
int openServo = 180;
int closeServo = 0;
long buttonHoldStart = 0;
long holdDuration = 3000;

int openLED = 3;
int closeLED = D1;

int openBtn = D2;
int closeBtn = D0;

int BlynkOpenBtn = V0;
int BlynkCloseBtn = V1;
int BlynkOpenLED = V2;
int BlynkCloseLED = V3;

bool blynkOpenRequested = false;
bool blynkCloseRequested = false;
long lastOpenPress = 0;
long lastClosePress = 0;
long debounceDelay = 300;

unsigned long lastUnauthorizedAlert = 0;
long alertCooldown = 30000;

void connectToWiFi()
{
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n WiFi connected. IP: " + WiFi.localIP().toString());
}

void sendMessage(String message)
{
  String url = "http://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int response = http.POST(url);
  if (response == 200)
    Serial.println("WhatsApp alert sent!");
  else
    Serial.printf("Failed to send. Code: %d\n", response);
  http.end();
}

void openGate()
{
  gateServo.write(openServo);
  Serial.println("Servo opened!");
  digitalWrite(openLED, HIGH);
  digitalWrite(closeLED, LOW);
  Blynk.virtualWrite(BlynkOpenLED, 255);
  Blynk.virtualWrite(BlynkCloseLED, 0);
  Serial.println("Gate opened!");
}

void closeGate()
{
  gateServo.write(closeServo);
  Serial.println("Servo closed!");
  digitalWrite(openLED, LOW);
  digitalWrite(closeLED, HIGH);
  Blynk.virtualWrite(BlynkOpenLED, 0);
  Blynk.virtualWrite(BlynkCloseLED, 255);
  Serial.println("Gate closed!");
}

bool isAuthorizedCard(byte *uid, byte size)
{
  for (byte i = 0; i < size; i++)
  {
    if (uid[i] != authorizedUID[i])
      return false;
  }
  return true;
}

void checkRFID()
{
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    return;

  Serial.print("Card UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  if (isAuthorizedCard(mfrc522.uid.uidByte, mfrc522.uid.size))
  {
    Serial.println("Authorized card. Opening gate...");
    openGate();
    delay(5000);
    closeGate();
  }
  else
  {
    Serial.println("Unauthorized card detected!");
    unsigned long currentTime = millis();
    if (currentTime - lastUnauthorizedAlert > alertCooldown)
    {
      sendMessage("Unauthorized RFID access attempt at the smart gate!");
      lastUnauthorizedAlert = currentTime;
    }
    else
    {
      Serial.println("Alert suppressed (cooldown active).");
    }
  }

  mfrc522.PICC_HaltA();
}

void checkButtons()
{
  long currentTime = millis();
  if (digitalRead(openBtn) == HIGH && (currentTime - lastOpenPress > debounceDelay))
  {
    Serial.println("openBtn Pressed!");
    openGate();
    lastOpenPress = currentTime;
  }

  if (digitalRead(closeBtn) == HIGH && (currentTime - lastClosePress > debounceDelay))
  {
    Serial.println("closeBtn Pressed!");
    closeGate();
    lastClosePress = currentTime;
  }

  if (blynkOpenRequested)
  {
    openGate();
    blynkOpenRequested = false;
  }

  if (blynkCloseRequested)
  {
    closeGate();
    blynkCloseRequested = false;
  }
}

BLYNK_WRITE(V0)
{
  blynkOpenRequested = param.asInt();
}

BLYNK_WRITE(V1)
{
  blynkCloseRequested = param.asInt();
}

void setup()
{
  Serial.begin(9600);

  pinMode(openLED, OUTPUT);
  pinMode(closeLED, OUTPUT);
  pinMode(openBtn, INPUT);
  pinMode(closeBtn, INPUT);

  digitalWrite(openLED, LOW);
  digitalWrite(closeLED, HIGH);

  gateServo.attach(SERVO_PIN);
  gateServo.write(closeServo);

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID Initialized.");

  connectToWiFi();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.virtualWrite(BlynkOpenLED, 0);
  Blynk.virtualWrite(BlynkCloseLED, 255);
}

void loop()
{
  Blynk.run();
  checkRFID();
  checkButtons();
}

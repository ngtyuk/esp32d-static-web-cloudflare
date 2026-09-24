#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <time.h>

#include "site_html.h"

namespace {
constexpr char kApName[] = "ESP32D-Setup";
constexpr char kApPassword[] = "esp32d-setup";
constexpr char kHostname[] = "esp32d-web";
constexpr uint8_t kLedPin = 2;  // D2 on the ESP32 Dev Module.
constexpr uint8_t kButtonPin = 13;  // D13, wired to GND when pressed.
constexpr uint32_t kBlinkIntervalMs = 250;
constexpr uint32_t kButtonDebounceMs = 40;
constexpr uint32_t kWifiTimeoutMs = 15000;
constexpr long kJstOffsetSeconds = 9 * 60 * 60;
constexpr time_t kMinimumValidEpoch = 1704067200;

enum MoodState : uint8_t {
  kMoodIdle = 0,
  kMoodWaiting = 1,
  kMoodAnswered = 2,
};

WebServer server(80);
Preferences preferences;
bool setupMode = false;
bool ledOn = false;
bool ledBlinking = false;
uint32_t nextBlinkAt = 0;
MoodState moodState = kMoodIdle;
uint64_t askedAtEpoch = 0;
uint64_t answeredAtEpoch = 0;
bool buttonStableState = HIGH;
bool buttonLastReading = HIGH;
uint32_t buttonDebounceAt = 0;

const char kSetupPage[] PROGMEM = R"HTML(<!doctype html><html lang="ja"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>ESP-32D Wi-Fi設定</title><style>body{font-family:system-ui,sans-serif;max-width:520px;margin:40px auto;padding:0 20px;background:#07111f;color:#eef6ff}main{border:1px solid #29415f;border-radius:18px;padding:24px;background:#0e1d31}input{display:block;width:100%;box-sizing:border-box;margin:8px 0 18px;padding:12px;border:1px solid #496685;border-radius:8px;background:#07111f;color:white}button{padding:12px 18px;border:0;border-radius:999px;background:#63e6f5;color:#07111f;font-weight:700}</style><main><h1>ESP-32D Wi-Fi設定</h1><p>接続先のWi-Fi情報を入力してください。保存後、ESP-32Dが再起動します。</p><form method="post" action="/save"><label>Wi-Fi SSID<input name="ssid" required autocomplete="off"></label><label>パスワード<input name="password" type="password" autocomplete="off"></label><button type="submit">保存して接続</button></form></main></html>)HTML";

void sendAsset(const char* contentType, const char* body) {
  server.sendHeader("Cache-Control", "no-store, max-age=0");
  server.send_P(200, contentType, body);
}

void handleHealth() {
  String json = "{\"ok\":true,\"hostname\":\"";
  json += kHostname;
  json += "\",\"ip\":\"";
  json += WiFi.localIP().toString();
  json += "\",\"uptime\":";
  json += String(millis() / 1000);
  json += "}";
  server.sendHeader("Cache-Control", "no-store, max-age=0");
  server.send(200, "application/json; charset=utf-8", json);
}

void setLed(bool on) {
  ledOn = on;
  digitalWrite(kLedPin, ledOn ? HIGH : LOW);
}

bool hasValidClock() {
  return time(nullptr) >= kMinimumValidEpoch;
}

uint64_t currentEpoch() {
  return hasValidClock() ? static_cast<uint64_t>(time(nullptr)) : 0;
}

String formatTimestamp(uint64_t epoch) {
  if (epoch == 0) return "";

  const time_t raw = static_cast<time_t>(epoch);
  struct tm localTime;
  localtime_r(&raw, &localTime);
  char buffer[24];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &localTime);
  return String(buffer) + "+09:00";
}

void saveMoodState() {
  preferences.begin("mood", false);
  preferences.putUChar("state", static_cast<uint8_t>(moodState));
  preferences.putULong64("asked", askedAtEpoch);
  preferences.putULong64("answered", answeredAtEpoch);
  preferences.end();
}

void loadMoodState() {
  preferences.begin("mood", true);
  moodState = static_cast<MoodState>(preferences.getUChar("state", kMoodIdle));
  askedAtEpoch = preferences.getULong64("asked", 0);
  answeredAtEpoch = preferences.getULong64("answered", 0);
  preferences.end();

  if (moodState == kMoodWaiting) {
    ledBlinking = true;
    setLed(false);
    nextBlinkAt = millis();
  } else {
    moodState = moodState == kMoodAnswered ? kMoodAnswered : kMoodIdle;
  }
}

void configureClock() {
  configTime(kJstOffsetSeconds, 0, "pool.ntp.org", "time.nist.gov", "time.cloudflare.com");
}

void startWaiting() {
  moodState = kMoodWaiting;
  askedAtEpoch = currentEpoch();
  saveMoodState();
  ledBlinking = true;
  setLed(false);
  nextBlinkAt = millis();
}

void stopWaiting() {
  ledBlinking = false;
  setLed(false);
}

void updateBlink() {
  if (!ledBlinking || millis() < nextBlinkAt) return;

  setLed(!ledOn);
  nextBlinkAt = millis() + kBlinkIntervalMs;
}

void answerQuestion() {
  moodState = kMoodAnswered;
  answeredAtEpoch = currentEpoch();
  saveMoodState();
  stopWaiting();
}

void updateButton() {
  const bool reading = digitalRead(kButtonPin);
  if (reading != buttonLastReading) buttonDebounceAt = millis();

  if (millis() - buttonDebounceAt > kButtonDebounceMs && reading != buttonStableState) {
    buttonStableState = reading;
    if (buttonStableState == LOW && moodState == kMoodWaiting) answerQuestion();
  }
  buttonLastReading = reading;
}

void handleStatus() {
  String json = "{\"state\":\"";
  if (moodState == kMoodWaiting) json += "waiting";
  else if (moodState == kMoodAnswered) json += "answered";
  else json += "idle";
  json += "\",\"askedAt\":";
  const String askedAt = formatTimestamp(askedAtEpoch);
  if (askedAt.isEmpty()) json += "null";
  else json += "\"" + askedAt + "\"";
  json += ",\"answeredAt\":";
  const String answeredAt = formatTimestamp(answeredAtEpoch);
  if (answeredAt.isEmpty()) json += "null";
  else json += "\"" + answeredAt + "\"";
  json += "}";
  server.sendHeader("Cache-Control", "no-store, max-age=0");
  server.send(200, "application/json; charset=utf-8", json);
}

void handleAsk() {
  startWaiting();
  server.sendHeader("Cache-Control", "no-store, max-age=0");
  server.send(202, "application/json; charset=utf-8", "{\"waiting\":true}");
}

void handleSave() {
  const String ssid = server.arg("ssid");
  const String password = server.arg("password");
  if (ssid.isEmpty()) {
    server.send(400, "text/plain; charset=utf-8", "SSID is required");
    return;
  }
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();
  server.send(200, "text/html; charset=utf-8", "<meta charset='utf-8'><p>保存しました。ESP-32Dを再起動しています…</p>");
  delay(900);
  ESP.restart();
}

void handleNotFound() {
  server.send(404, "text/plain; charset=utf-8", "Not found");
}

bool connectToSavedWifi() {
  preferences.begin("wifi", true);
  const String ssid = preferences.getString("ssid", "");
  const String password = preferences.getString("password", "");
  preferences.end();
  if (ssid.isEmpty()) return false;

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(kHostname);
  WiFi.begin(ssid.c_str(), password.c_str());
  const uint32_t startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < kWifiTimeoutMs) {
    delay(250);
  }
  return WiFi.status() == WL_CONNECTED;
}

void startSetupAccessPoint() {
  setupMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(kApName, kApPassword);
  Serial.print("Setup AP: http://");
  Serial.println(WiFi.softAPIP());
}

void configureRoutes() {
  server.on("/", HTTP_GET, []() {
    if (setupMode) sendAsset("text/html; charset=utf-8", kSetupPage);
    else sendAsset("text/html; charset=utf-8", kIndexHtml);
  });
  server.on("/styles.css", HTTP_GET, []() { sendAsset("text/css; charset=utf-8", kStylesCss); });
  server.on("/app.js", HTTP_GET, []() { sendAsset("application/javascript; charset=utf-8", kAppJs); });
  server.on("/api/health", HTTP_GET, handleHealth);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/ask", HTTP_POST, handleAsk);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(handleNotFound);
  server.begin();
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nESP-32D static site booting...");
  pinMode(kLedPin, OUTPUT);
  setLed(false);
  pinMode(kButtonPin, INPUT_PULLUP);
  buttonStableState = digitalRead(kButtonPin);
  buttonLastReading = buttonStableState;
  loadMoodState();

  if (!connectToSavedWifi()) {
    startSetupAccessPoint();
  } else {
    configureClock();
    Serial.print("Site: http://");
    Serial.println(WiFi.localIP());
  }
  configureRoutes();
}

void loop() {
  updateButton();
  updateBlink();
  server.handleClient();
  delay(2);
}

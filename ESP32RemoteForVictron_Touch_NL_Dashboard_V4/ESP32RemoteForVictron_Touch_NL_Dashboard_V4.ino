#include "general_settings.h"
#include "secret_settings.h"

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <ArduinoJson.h>
#include <EspMQTTClient.h>
#include <LilyGo_AMOLED.h>
#include <TFT_eSPI.h>
#include <math.h>
#include "boot_animation.h"

#define TFT_WIDTH 536
#define TFT_HEIGHT 240

#define topButtonIfUSBIsOnTheLeft 21
#define bottomButtonIfUSBIsOnTheLeft 0

// -----------------------------------------------------------------------------
// App metadata and hardware globals
// -----------------------------------------------------------------------------
const char *programName = "ESP32 Remote voor Victron";
const char *programVersion = "Touch NL dashboard v4";

LilyGo_Class amoled;
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

EspMQTTClient client(
    SECRET_SETTINGS_WIFI_SSID,
    SECRET_SETTINGS_WIFI_PASSWORD,
    SECRET_SETTINGS_MQTT_Broker,
    SECRET_SETTINGS_MQTT_UserID,
    SECRET_SETTINGS_MQTT_Password,
    SECRET_SETTINGS_MQTT_ClientName,
    SECRET_SETTINGS_MQTT_Port);

bool generalDebugOutput = (GENERAL_SETTINGS_DEBUG_OUTPUT_LEVEL > 0);
bool verboseDebugOutput = (GENERAL_SETTINGS_DEBUG_OUTPUT_LEVEL > 1);

unsigned long lastMQTTUpdateReceived = 0UL;
unsigned long lastDisplayUpdate = 0UL;
unsigned long keepDisplayOnStartTime = 0UL;
unsigned long keepDisplayOnTimeOut = (unsigned long)GENERAL_SETTINGS_DISPLAY_TIMEOUT_MINUTEN * 60000UL;
unsigned long lastKeepAlive = 0UL;
unsigned long lastTouchAccepted = 0UL;
unsigned long lastTimeSyncAttempt = 0UL;
unsigned long lastBrightnessCheck = 0UL;
unsigned long pendingActionStarted = 0UL;
unsigned long toastUntil = 0UL;
unsigned long activeTouchUntil = 0UL;

bool theDisplayIsCurrentlyOn = true;
bool boardHasTouch = false;
bool installationDiscovered = false;
bool multiplusDiscovered = false;
bool subscriptionsReady = false;
bool timeConfigured = false;
bool touchLocked = GENERAL_SETTINGS_TOUCH_VERGRENDELING_BIJ_START;

bool discoveryInstallSubscribed = false;
bool discoveryVebusSubscribed = false;
bool discoverySolarSubscribed = false;
bool solarStateSubscribed = false;

String VictronInstallationID = String(SECRET_SETTING_VICTRON_INSTALLATION_ID);
String MultiplusThreeDigitID = String(SECRET_SETTING_VICTRON_MULTIPLUS_ID);
String SolarChargerThreeDigitID = String(SECRET_SETTING_VICTRON_SOLAR_CHARGER_ID);

float gridInL1Watts = 0.0F;
float gridInL2Watts = 0.0F;
float gridInL3Watts = 0.0F;
float solarWatts = 0.0F;
float batterySOC = 0.0F;
float batteryTTG = 0.0F;
float batteryPower = 0.0F;
float ACOutL1Watts = 0.0F;
float ACOutL2Watts = 0.0F;
float ACOutL3Watts = 0.0F;
String chargingState = "Onbekend";
String toastMessage = "";
String pendingActionLabel = "";

float dispGridWatts = 0.0F;
float dispSolarWatts = 0.0F;
float dispBatterySOC = 0.0F;
float dispBatteryPower = 0.0F;
float dispACLoadWatts = 0.0F;
float dispBatteryTTG = 0.0F;

const int HISTORY_POINTS = 64;
float solarHistory[HISTORY_POINTS] = {0};
float gridHistory[HISTORY_POINTS] = {0};
float loadHistory[HISTORY_POINTS] = {0};
float batteryPowerHistory[HISTORY_POINTS] = {0};
int historyIndex = 0;
bool historyWrapped = false;
unsigned long lastHistorySample = 0UL;

enum HistoryRange
{
  HISTORY_RANGE_1H,
  HISTORY_RANGE_24H,
  HISTORY_RANGE_7D
};

struct HistorySeries
{
  float grid[96] = {0};
  float solar[96] = {0};
  float load[96] = {0};
  float battery[96] = {0};
  int size = 0;
  int index = 0;
  bool wrapped = false;
  unsigned long intervalMs = 60000UL;
  unsigned long lastSample = 0UL;
};

HistorySeries history1h;
HistorySeries history24h;
HistorySeries history7d;
HistoryRange selectedHistoryRange = HISTORY_RANGE_1H;

int topButton = -1;
int bottomButton = -1;
int currentBrightness = GENERAL_SETTINGS_DAG_BRIGHTNESS;
bool nightThemeActive = false;

enum BrightnessMode
{
  BRIGHTNESS_AUTO,
  BRIGHTNESS_DAY,
  BRIGHTNESS_NIGHT
};

BrightnessMode brightnessMode = BRIGHTNESS_AUTO;

String discoveryInstallTopic;
String discoveryVebusTopic;
String discoverySolarTopic;

String subGridL1Topic;
String subGridL2Topic;
String subGridL3Topic;
String subPvTopic;
String subBatterySocTopic;
String subBatteryPowerTopic;
String subBatteryTTGTopic;
String subAcL1Topic;
String subAcL2Topic;
String subAcL3Topic;
String subMultiplusModeTopic;
String subSolarStateTopic;


enum multiplusMode
{
  ChargerOnly,
  InverterOnly,
  On,
  Off,
  Unknown
};

multiplusMode currentMultiplusMode = Unknown;
multiplusMode lastKnownMultiplusMode = Unknown;
multiplusMode expectedMultiplusMode = Unknown;


enum multiplusFunction
{
  Charger,
  Inverter
};


enum Page
{
  PAGE_OVERVIEW,
  PAGE_DETAIL,
  PAGE_SYSTEM
};

Page currentPage = PAGE_OVERVIEW;


enum DialogState
{
  DIALOG_NONE,
  DIALOG_CONFIRM
};

DialogState dialogState = DIALOG_NONE;
multiplusFunction pendingFunction = Charger;

struct TouchBox
{
  int16_t x1, y1, x2, y2;
  bool enabled;
};

TouchBox chargerTouchBox = {0, 0, 0, 0, false};
TouchBox inverterTouchBox = {0, 0, 0, 0, false};
TouchBox pageOverviewBox = {0, 0, 0, 0, false};
TouchBox pageDetailBox = {0, 0, 0, 0, false};
TouchBox pageSystemBox = {0, 0, 0, 0, false};
TouchBox detailRange1hBox = {0, 0, 0, 0, false};
TouchBox detailRange24hBox = {0, 0, 0, 0, false};
TouchBox detailRange7dBox = {0, 0, 0, 0, false};
TouchBox brightnessModeBox = {0, 0, 0, 0, false};
TouchBox lockBox = {0, 0, 0, 0, false};
TouchBox dialogYesBox = {0, 0, 0, 0, false};
TouchBox dialogNoBox = {0, 0, 0, 0, false};
TouchBox dialogCancelBox = {0, 0, 0, 0, false};
TouchBox lastTouchedBox = {0, 0, 0, 0, false};

bool pendingAction = false;
bool displayDirty = true;
unsigned long lastStatusSecondRendered = 0UL;
bool statusSecondInitialized = false;

// -----------------------------------------------------------------------------
// Utility helpers
// -----------------------------------------------------------------------------
void DebugPrint(const String &msg)
{
  if (generalDebugOutput)
    Serial.println(msg);
}

void RefreshDisplay()
{
  amoled.pushColors(0, 0, TFT_WIDTH, TFT_HEIGHT, (uint16_t *)sprite.getPointer());
}

void ResetKeepDisplayOnStartTime()
{
  keepDisplayOnStartTime = millis();
}

void SetKeepDisplayOnTimeOut(unsigned int minutes)
{
  keepDisplayOnTimeOut = (unsigned long)minutes * 60UL * 1000UL;
  ResetKeepDisplayOnStartTime();
}

bool IsKeepDisplayOnTimedOut()
{
  return (millis() - keepDisplayOnStartTime >= keepDisplayOnTimeOut);
}

bool PointInTouchBox(int16_t tx, int16_t ty, const TouchBox &box)
{
  return box.enabled && tx >= box.x1 && tx <= box.x2 && ty >= box.y1 && ty <= box.y2;
}

bool SameTouchBox(const TouchBox &a, const TouchBox &b)
{
  return a.enabled && b.enabled && a.x1 == b.x1 && a.y1 == b.y1 && a.x2 == b.x2 && a.y2 == b.y2;
}

void EnableBox(TouchBox &box, int x1, int y1, int x2, int y2)
{
  box.x1 = x1;
  box.y1 = y1;
  box.x2 = x2;
  box.y2 = y2;
  box.enabled = true;
}

void DisableBox(TouchBox &box)
{
  box.enabled = false;
}

void RememberTouchedBox(const TouchBox &box)
{
  if (!GENERAL_SETTINGS_TOON_TOUCH_FEEDBACK || !box.enabled)
    return;
  lastTouchedBox = box;
  activeTouchUntil = millis() + GENERAL_SETTINGS_TOUCH_FEEDBACK_MS;
}

String ConvertToStringWithAFixedNumberOfDecimalPlaces(float f, int decimals = 1)
{
  char buf[32];
  dtostrf(f, 0, decimals, buf);
  return String(buf);
}

String FormatWatts(float watts)
{
  if (GENERAL_SETTINGS_IF_OVER_1000_WATTS_REPORT_KW && fabs(watts) >= 1000.0F)
    return ConvertToStringWithAFixedNumberOfDecimalPlaces(watts / 1000.0F, GENERAL_SETTINGS_NUMBER_DECIMAL_PLACES_FOR_KW_REPORTING) + " kW";
  return String((int)round(watts)) + " W";
}

String FormatMinutes(float seconds)
{
  if (seconds <= 0.0F)
    return "--";
  long totalMinutes = (long)round(seconds / 60.0F);
  long hours = totalMinutes / 60;
  long minutes = totalMinutes % 60;
  if (hours > 0)
    return String(hours) + "u " + String(minutes) + "m";
  return String(minutes) + " min";
}

String BrightnessModeLabel()
{
  switch (brightnessMode)
  {
  case BRIGHTNESS_DAY: return "Dag";
  case BRIGHTNESS_NIGHT: return "Nacht";
  default: return "Auto";
  }
}

String HistoryRangeLabel(HistoryRange range)
{
  switch (range)
  {
  case HISTORY_RANGE_24H: return "24u";
  case HISTORY_RANGE_7D: return "7d";
  default: return "1u";
  }
}

uint16_t AppBackgroundColor()
{
  return nightThemeActive ? TFT_NAVY : TFT_BLACK;
}

String ModeToDutch(multiplusMode mode)
{
  switch (mode)
  {
  case ChargerOnly: return "alleen lader";
  case InverterOnly: return "alleen omvormer";
  case On: return "aan";
  case Off: return "uit";
  default: return "onbekend";
  }
}

bool ChargerIsOn(multiplusMode mode)
{
  return (mode == On || mode == ChargerOnly);
}

bool InverterIsOn(multiplusMode mode)
{
  return (mode == On || mode == InverterOnly);
}

String ChargerStatus()
{
  if (currentMultiplusMode == Unknown)
    return "?";
  return ChargerIsOn(currentMultiplusMode) ? "aan" : "uit";
}

String InverterStatus()
{
  if (currentMultiplusMode == Unknown)
    return "?";
  return InverterIsOn(currentMultiplusMode) ? "aan" : "uit";
}

uint16_t StatusColor(bool onState, bool unknown)
{
  if (unknown)
    return TFT_ORANGE;
  return onState ? TFT_GREEN : TFT_DARKGREY;
}

void SetToast(const String &msg, unsigned long durationMs = 2000)
{
  toastMessage = msg;
  toastUntil = millis() + durationMs;
  displayDirty = true;
  DebugPrint(msg);
}

bool ParseMqttFloatValue(const String &payload, float &target)
{
  JsonDocument doc;
  if (deserializeJson(doc, payload) != DeserializationError::Ok)
    return false;
  target = doc["value"] | 0.0F;
  return true;
}

float SmoothTowards(float current, float target, float factor = 0.18F)
{
  return current + (target - current) * factor;
}

float ClampFloat(float value, float minValue, float maxValue)
{
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}

float AnimatieFactorSchaal()
{
  if (GENERAL_SETTINGS_ANIMATIE_SNELHEID <= 1) return 0.65F;
  if (GENERAL_SETTINGS_ANIMATIE_SNELHEID >= 3) return 1.45F;
  return 1.0F;
}

unsigned long AnimatieFrameIntervalMs()
{
  if (GENERAL_SETTINGS_ANIMATIE_SNELHEID <= 1) return 180UL;
  if (GENERAL_SETTINGS_ANIMATIE_SNELHEID >= 3) return 80UL;
  return 120UL;
}

String AnimatieSnelheidLabel()
{
  if (GENERAL_SETTINGS_ANIMATIE_SNELHEID <= 1) return "langzaam";
  if (GENERAL_SETTINGS_ANIMATIE_SNELHEID >= 3) return "snel";
  return "normaal";
}

uint16_t BlendColor565(uint16_t c1, uint16_t c2, float ratio)
{
  if (ratio <= 0.0F) return c1;
  if (ratio >= 1.0F) return c2;
  int r1 = (c1 >> 11) & 0x1F, g1 = (c1 >> 5) & 0x3F, b1 = c1 & 0x1F;
  int r2 = (c2 >> 11) & 0x1F, g2 = (c2 >> 5) & 0x3F, b2 = c2 & 0x1F;
  int r = r1 + (int)round((r2 - r1) * ratio);
  int g = g1 + (int)round((g2 - g1) * ratio);
  int b = b1 + (int)round((b2 - b1) * ratio);
  return (uint16_t)((r << 11) | (g << 5) | b);
}

float PulseFactor(unsigned long speedMs = 900UL)
{
  return (sinf((float)(millis() % speedMs) / (float)speedMs * 6.2831853F) + 1.0F) * 0.5F;
}

bool UpdateAnimatedValues()
{
  bool changed = false;
  float totalGridWatts = gridInL1Watts + gridInL2Watts + gridInL3Watts;
  float totalLoadWatts = ACOutL1Watts + ACOutL2Watts + ACOutL3Watts;
  float scale = AnimatieFactorSchaal();
  float nextGrid = SmoothTowards(dispGridWatts, totalGridWatts, ClampFloat(0.18F * scale, 0.02F, 0.55F));
  float nextSolar = SmoothTowards(dispSolarWatts, solarWatts, ClampFloat(0.18F * scale, 0.02F, 0.55F));
  float nextSoc = SmoothTowards(dispBatterySOC, batterySOC, ClampFloat(0.12F * scale, 0.02F, 0.45F));
  float nextBatteryPower = SmoothTowards(dispBatteryPower, batteryPower, ClampFloat(0.18F * scale, 0.02F, 0.55F));
  float nextLoad = SmoothTowards(dispACLoadWatts, totalLoadWatts, ClampFloat(0.18F * scale, 0.02F, 0.55F));
  float nextTTG = SmoothTowards(dispBatteryTTG, batteryTTG, ClampFloat(0.08F * scale, 0.02F, 0.35F));

  changed = changed || fabsf(nextGrid - dispGridWatts) > 0.25F;
  changed = changed || fabsf(nextSolar - dispSolarWatts) > 0.25F;
  changed = changed || fabsf(nextSoc - dispBatterySOC) > 0.05F;
  changed = changed || fabsf(nextBatteryPower - dispBatteryPower) > 0.25F;
  changed = changed || fabsf(nextLoad - dispACLoadWatts) > 0.25F;
  changed = changed || fabsf(nextTTG - dispBatteryTTG) > 1.0F;

  dispGridWatts = nextGrid;
  dispSolarWatts = nextSolar;
  dispBatterySOC = nextSoc;
  dispBatteryPower = nextBatteryPower;
  dispACLoadWatts = nextLoad;
  dispBatteryTTG = nextTTG;
  return changed;
}

void PushHistorySample(HistorySeries &series, float grid, float solar, float load, float battery)
{
  if (series.size <= 0)
    return;
  series.grid[series.index] = grid;
  series.solar[series.index] = solar;
  series.load[series.index] = load;
  series.battery[series.index] = battery;
  series.index++;
  if (series.index >= series.size)
  {
    series.index = 0;
    series.wrapped = true;
  }
}

void SampleSeriesIfNeeded(HistorySeries &series, float grid, float solar, float load, float battery)
{
  if (millis() - series.lastSample < series.intervalMs)
    return;
  series.lastSample = millis();
  PushHistorySample(series, grid, solar, load, battery);
}

void SampleHistoryIfNeeded()
{
  if (millis() - lastHistorySample < 3000UL) return;
  lastHistorySample = millis();
  float gridNow = gridInL1Watts + gridInL2Watts + gridInL3Watts;
  float loadNow = ACOutL1Watts + ACOutL2Watts + ACOutL3Watts;
  solarHistory[historyIndex] = solarWatts;
  gridHistory[historyIndex] = gridNow;
  loadHistory[historyIndex] = loadNow;
  batteryPowerHistory[historyIndex] = batteryPower;
  historyIndex++;
  if (historyIndex >= HISTORY_POINTS) { historyIndex = 0; historyWrapped = true; }

  SampleSeriesIfNeeded(history1h, gridNow, solarWatts, loadNow, batteryPower);
  SampleSeriesIfNeeded(history24h, gridNow, solarWatts, loadNow, batteryPower);
  SampleSeriesIfNeeded(history7d, gridNow, solarWatts, loadNow, batteryPower);
}

void DrawSparkline(const float *arr, uint16_t color, int x, int y, int w, int h, bool symmetric = false, int pointCount = HISTORY_POINTS, int currentIndex = -1, bool wrapped = false)
{
  if (!GENERAL_SETTINGS_ENABLE_SPARKLINES)
    return;
  if (pointCount < 2)
    return;
  if (currentIndex < 0)
    currentIndex = historyIndex;

  float minV = arr[0], maxV = arr[0];
  for (int i = 1; i < pointCount; i++)
  {
    if (arr[i] < minV) minV = arr[i];
    if (arr[i] > maxV) maxV = arr[i];
  }
  if (symmetric)
  {
    float absMax = fabsf(minV);
    if (fabsf(maxV) > absMax) absMax = fabsf(maxV);
    minV = -absMax; maxV = absMax;
    int midY = y + h / 2;
    sprite.drawLine(x, midY, x + w, midY, TFT_DARKGREY);
  }
  if (fabsf(maxV - minV) < 0.001F) { maxV += 1.0F; minV -= 1.0F; }
  int available = wrapped ? pointCount : (currentIndex > 2 ? currentIndex : 2);
  int pointsForScale = wrapped ? (pointCount - 1) : (available - 1);
  if (pointsForScale < 1)
    pointsForScale = 1;

  for (int i = 1; i < available; i++)
  {
    int p0 = wrapped ? (currentIndex + i - 1) % pointCount : (i - 1);
    int p1 = wrapped ? (currentIndex + i) % pointCount : i;
    float v0 = arr[p0], v1 = arr[p1];
    int x0 = x + (i - 1) * w / pointsForScale;
    int x1 = x + i * w / pointsForScale;
    int y0 = y + h - 1 - (int)round((v0 - minV) * (h - 1) / (maxV - minV));
    int y1 = y + h - 1 - (int)round((v1 - minV) * (h - 1) / (maxV - minV));
    sprite.drawLine(x0, y0, x1, y1, color);
  }
}

void DrawMetricCard(int x, int y, int w, int h, const String &title, const String &value, uint16_t accent, bool highlight = false)
{
  uint16_t bg = BlendColor565(TFT_BLACK, accent, highlight ? 0.28F : 0.14F);
  sprite.fillRoundRect(x, y, w, h, 12, bg);
  sprite.drawRoundRect(x, y, w, h, 12, BlendColor565(accent, TFT_WHITE, 0.35F));
  sprite.setTextDatum(TL_DATUM);
  sprite.setTextColor(BlendColor565(TFT_WHITE, accent, 0.12F), bg);
  sprite.setTextFont(2);
  sprite.drawString(title, x + 10, y + 8);
  sprite.setTextColor(TFT_WHITE, bg);
  sprite.setTextFont(4);
  sprite.drawString(value, x + 10, y + 28);
}

void DrawFlowChevron(int cx, int cy, float angle, uint16_t color, int size = 5)
{
  int tipX = cx + (int)roundf(cosf(angle) * size);
  int tipY = cy + (int)roundf(sinf(angle) * size);
  int leftX = cx + (int)roundf(cosf(angle + 2.55F) * (size - 2));
  int leftY = cy + (int)roundf(sinf(angle + 2.55F) * (size - 2));
  int rightX = cx + (int)roundf(cosf(angle - 2.55F) * (size - 2));
  int rightY = cy + (int)roundf(sinf(angle - 2.55F) * (size - 2));
  sprite.fillTriangle(tipX, tipY, leftX, leftY, rightX, rightY, color);
}

void DrawFlowPath(int x1, int y1, int x2, int y2, uint16_t color, float intensity, bool reverse = false)
{
  uint16_t pathColor = BlendColor565(TFT_DARKGREY, color, intensity);
  sprite.drawLine(x1, y1, x2, y2, pathColor);
  if (intensity < 0.2F || !GENERAL_SETTINGS_ENABLE_FLOW_ANIMATIE) return;
  float phase = PulseFactor(950UL);
  float dx = (float)(x2 - x1);
  float dy = (float)(y2 - y1);
  float angle = atan2f(dy, dx);
  if (reverse)
    angle += PI;
  for (int i = 0; i < 4; i++)
  {
    float t = fmodf(phase + (float)i / 4.0F, 1.0F);
    if (reverse) t = 1.0F - t;
    int px = x1 + (int)roundf(dx * t);
    int py = y1 + (int)roundf(dy * t);
    uint16_t chevronColor = BlendColor565(color, TFT_WHITE, 0.30F + ((float)i * 0.12F));
    DrawFlowChevron(px, py, angle, chevronColor, 5);
  }
}

void DrawStatusBadge(int x, int y, int w, const String &label, bool ok, uint16_t onColor)
{
  uint16_t bg = ok ? BlendColor565(TFT_BLACK, onColor, 0.40F) : TFT_DARKGREY;
  sprite.fillRoundRect(x, y, w, 18, 8, bg);
  sprite.drawRoundRect(x, y, w, 18, 8, TFT_WHITE);
  sprite.setTextDatum(MC_DATUM);
  sprite.setTextColor(TFT_WHITE, bg);
  sprite.setTextFont(1);
  sprite.drawString(label, x + w / 2, y + 9);
}

HistorySeries &CurrentHistorySeries()
{
  if (selectedHistoryRange == HISTORY_RANGE_24H)
    return history24h;
  if (selectedHistoryRange == HISTORY_RANGE_7D)
    return history7d;
  return history1h;
}

void DrawEnergyFlowSummary(int y)
{
  int x = 172;
  int w = 192;
  int h = 28;
  sprite.fillRoundRect(x, y, w, h, 8, BlendColor565(AppBackgroundColor(), TFT_CYAN, 0.12F));
  sprite.drawRoundRect(x, y, w, h, 8, TFT_DARKGREY);
  sprite.setTextDatum(MC_DATUM);
  sprite.setTextFont(2);
  sprite.setTextColor(TFT_WHITE, AppBackgroundColor());

  bool pvToBattery = dispSolarWatts > 60.0F && dispBatteryPower > 20.0F;
  bool batteryToLoad = dispBatteryPower < -30.0F && dispACLoadWatts > 50.0F;
  bool gridToLoad = dispGridWatts > 40.0F && dispACLoadWatts > 50.0F;
  bool loadToGrid = dispGridWatts < -40.0F;

  String flowText = "Rust";
  if (pvToBattery) flowText = "Zon -> Accu";
  else if (batteryToLoad) flowText = "Accu -> AC";
  else if (gridToLoad) flowText = "Net -> AC";
  else if (loadToGrid) flowText = "AC -> Net";

  sprite.drawString(flowText, x + w / 2, y + 9);
  sprite.setTextFont(1);
  sprite.setTextColor(TFT_LIGHTGREY, AppBackgroundColor());
  sprite.drawString(String("Net ") + FormatWatts(dispGridWatts) + "  |  Zon " + FormatWatts(dispSolarWatts), x + w / 2, y + 21);
}

void SetDisplayOrientation()
{
  if (GENERAL_SETTINGS_USB_ON_THE_LEFT)
    amoled.setRotation(3);
  else
    amoled.setRotation(1);
}

void SetupTopAndBottomButtons()
{
  pinMode(topButtonIfUSBIsOnTheLeft, INPUT);
  pinMode(bottomButtonIfUSBIsOnTheLeft, INPUT);

  if (GENERAL_SETTINGS_USB_ON_THE_LEFT)
  {
    topButton = bottomButtonIfUSBIsOnTheLeft;
    bottomButton = topButtonIfUSBIsOnTheLeft;
  }
  else
  {
    topButton = topButtonIfUSBIsOnTheLeft;
    bottomButton = bottomButtonIfUSBIsOnTheLeft;
  }
}

void SetTheDisplayOn(bool on)
{
  theDisplayIsCurrentlyOn = on;
  displayDirty = true;
  if (!on)
  {
    amoled.setBrightness(0);
    sprite.fillSprite(TFT_BLACK);
    RefreshDisplay();
  }
  else
  {
    amoled.setBrightness((uint8_t)currentBrightness);
    ResetKeepDisplayOnStartTime();
  }
}

void PublishKeepAlive(bool force = false)
{
  if (!client.isMqttConnected() || VictronInstallationID == "+")
    return;

  if (force || (GENERAL_SETTINGS_SEND_PERIODICAL_KEEP_ALIVE_REQUESTS && millis() - lastKeepAlive >= GENERAL_SETTINGS_SEND_PERIODICAL_KEEP_ALIVE_REQUESTS_INTERVAL))
  {
    client.publish("R/" + VictronInstallationID + "/keepalive", "");
    lastKeepAlive = millis();
  }
}

void SyncTimeIfNeeded()
{
  if (!client.isWifiConnected())
    return;

  if (timeConfigured && millis() - lastTimeSyncAttempt < 12UL * 60UL * 60UL * 1000UL)
    return;

  if (!timeConfigured && millis() - lastTimeSyncAttempt < 30000UL)
    return;

  lastTimeSyncAttempt = millis();
  configTzTime(GENERAL_SETTINGS_TIMEZONE, GENERAL_SETTINGS_NTP_SERVER_1, GENERAL_SETTINGS_NTP_SERVER_2);

  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 3000))
  {
    timeConfigured = true;
    DebugPrint("Tijd gesynchroniseerd");
  }
}

String CurrentTimeString()
{
  if (!timeConfigured)
    return "--:--";
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 50))
    return "--:--";
  char buf[8];
  strftime(buf, sizeof(buf), "%H:%M", &timeinfo);
  return String(buf);
}

void UpdateBrightness()
{
  if (millis() - lastBrightnessCheck < 2000)
    return;
  lastBrightnessCheck = millis();

  bool nightByClock = false;
  if (timeConfigured)
  {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 50))
    {
      int hour = timeinfo.tm_hour;
      nightByClock = (hour >= GENERAL_SETTINGS_NACHT_START_UUR || hour < GENERAL_SETTINGS_DAG_START_UUR);
    }
  }

  int desired = GENERAL_SETTINGS_DAG_BRIGHTNESS;
  if (brightnessMode == BRIGHTNESS_DAY)
  {
    desired = GENERAL_SETTINGS_DAG_BRIGHTNESS;
    nightThemeActive = false;
  }
  else if (brightnessMode == BRIGHTNESS_NIGHT)
  {
    desired = GENERAL_SETTINGS_NACHT_BRIGHTNESS;
    nightThemeActive = true;
  }
  else
  {
    bool lowSolar = (solarWatts < 80.0F);
    nightThemeActive = nightByClock || lowSolar;
    if (nightByClock)
      desired = GENERAL_SETTINGS_NACHT_BRIGHTNESS;
    else if (lowSolar)
      desired = (GENERAL_SETTINGS_DAG_BRIGHTNESS + GENERAL_SETTINGS_NACHT_BRIGHTNESS) / 2;
    else
      desired = GENERAL_SETTINGS_DAG_BRIGHTNESS;
  }

  if (desired != currentBrightness)
  {
    currentBrightness = desired;
    if (theDisplayIsCurrentlyOn)
      amoled.setBrightness((uint8_t)currentBrightness);
    displayDirty = true;
  }
}

multiplusMode ComputeDesiredMode(multiplusFunction option)
{
  multiplusMode sourceMode = currentMultiplusMode;
  if (sourceMode == Unknown)
    sourceMode = lastKnownMultiplusMode;
  if (sourceMode == Unknown)
    return Unknown;

  if (option == Charger)
  {
    switch (sourceMode)
    {
    case ChargerOnly: return Off;
    case InverterOnly: return On;
    case On: return InverterOnly;
    case Off: return ChargerOnly;
    default: return Unknown;
    }
  }

  switch (sourceMode)
  {
  case ChargerOnly: return On;
  case InverterOnly: return Off;
  case On: return ChargerOnly;
  case Off: return InverterOnly;
  default: return Unknown;
  }
}

bool ApplyMultiplusMode(multiplusMode desiredMultiplusMode)
{
  multiplusMode sourceMode = currentMultiplusMode;
  if (sourceMode == Unknown)
    sourceMode = lastKnownMultiplusMode;

  if (sourceMode == Unknown)
  {
    SetToast("Schakelen geblokkeerd: status onbekend", 2500);
    return false;
  }
  if (desiredMultiplusMode == Unknown || desiredMultiplusMode == sourceMode || VictronInstallationID == "+" || MultiplusThreeDigitID == "+")
    return false;

  String modeCodeValue;
  switch (desiredMultiplusMode)
  {
  case ChargerOnly: modeCodeValue = "1"; break;
  case InverterOnly: modeCodeValue = "2"; break;
  case On: modeCodeValue = "3"; break;
  case Off: modeCodeValue = "4"; break;
  default: return false;
  }

  expectedMultiplusMode = desiredMultiplusMode;
  pendingAction = true;
  pendingActionStarted = millis();
  pendingActionLabel = "Schakelen...";
  client.publish("W/" + VictronInstallationID + "/vebus/" + MultiplusThreeDigitID + "/Mode", "{\"value\": " + modeCodeValue + "}");
  SetKeepDisplayOnTimeOut(GENERAL_SETTINGS_DISPLAY_TIMEOUT_MINUTEN);
  return true;
}

void StartToggleRequest(multiplusFunction option)
{
  if (touchLocked)
  {
    SetToast("Touch is vergrendeld", 1500);
    return;
  }

  if (!GENERAL_SETTINGS_ALLOW_CHANGING_INVERTER_AND_CHARGER_MODES)
  {
    SetToast("Bediening uitgeschakeld", 1500);
    return;
  }

  if (currentMultiplusMode == Unknown && lastKnownMultiplusMode == Unknown)
  {
    SetToast("Status onbekend, wacht op MQTT", 2000);
    return;
  }

  pendingFunction = option;

  if (GENERAL_SETTINGS_DIRECT_SCHAKELEN && !GENERAL_SETTINGS_TOON_BEVESTIGING_POPUP)
  {
    ApplyMultiplusMode(ComputeDesiredMode(option));
    return;
  }

  dialogState = DIALOG_CONFIRM;
  displayDirty = true;
}

void CancelDialog()
{
  dialogState = DIALOG_NONE;
  displayDirty = true;
  DisableBox(dialogYesBox);
  DisableBox(dialogNoBox);
  DisableBox(dialogCancelBox);
}

void ConfirmDialog()
{
  dialogState = DIALOG_NONE;
  displayDirty = true;
  ApplyMultiplusMode(ComputeDesiredMode(pendingFunction));
}

void UpdateChargingStateFallback()
{
  if (SolarChargerThreeDigitID != "+")
    return;
  if (batteryPower > 25.0F)
    chargingState = "Laden";
  else if (batteryPower < -25.0F)
    chargingState = "Ontladen";
  else
    chargingState = "Rust";
}

void SubscribeToChargingState()
{
  if (solarStateSubscribed)
    return;

  if (SolarChargerThreeDigitID == "+")
  {
    UpdateChargingStateFallback();
    return;
  }

  String commonTopic = "N/" + VictronInstallationID;
  subSolarStateTopic = commonTopic + "/solarcharger/" + SolarChargerThreeDigitID + "/State";

  client.subscribe(subSolarStateTopic, [](const String &payload)
                   {
                     JsonDocument doc;
                     if (deserializeJson(doc, payload) != DeserializationError::Ok)
                       return;
                     int stateCode = doc["value"] | -1;
                     switch (stateCode)
                     {
                     case 0: chargingState = "Uit"; break;
                     case 2: chargingState = "Fout"; break;
                     case 3: chargingState = "Bulk"; break;
                     case 4: chargingState = "Absorptie"; break;
                     case 5: chargingState = "Float"; break;
                     case 6: chargingState = "Opslag"; break;
                     case 7: chargingState = "Equalize"; break;
                     case 252: chargingState = "ESS"; break;
                     default: chargingState = "Onbekend"; break;
                     }
                     lastMQTTUpdateReceived = millis();
                     displayDirty = true;
                   });
  solarStateSubscribed = true;
}

// -----------------------------------------------------------------------------
// Data subscriptions and MQTT discovery
// -----------------------------------------------------------------------------
void SubscribeToCoreTopics()
{
  if (subscriptionsReady || VictronInstallationID == "+" || MultiplusThreeDigitID == "+")
    return;

  String commonTopic = "N/" + VictronInstallationID;
  String system0Topic = commonTopic + "/system/0/";
  subMultiplusModeTopic = commonTopic + "/vebus/" + MultiplusThreeDigitID + "/Mode";

  subGridL1Topic = system0Topic + "Ac/Grid/L1/Power";
  subGridL2Topic = system0Topic + "Ac/Grid/L2/Power";
  subGridL3Topic = system0Topic + "Ac/Grid/L3/Power";
  subPvTopic = system0Topic + "Dc/Pv/Power";
  subBatterySocTopic = system0Topic + "Dc/Battery/Soc";
  subBatteryPowerTopic = system0Topic + "Dc/Battery/Power";
  subBatteryTTGTopic = system0Topic + "Dc/Battery/TimeToGo";
  subAcL1Topic = system0Topic + "Ac/Consumption/L1/Power";
  subAcL2Topic = system0Topic + "Ac/Consumption/L2/Power";
  subAcL3Topic = system0Topic + "Ac/Consumption/L3/Power";

  if (GENERAL_SETTINGS_GRID_IN_L1_IS_USED)
    client.subscribe(subGridL1Topic, [](const String &payload) {
      if (ParseMqttFloatValue(payload, gridInL1Watts))
        lastMQTTUpdateReceived = millis();
    });

  if (GENERAL_SETTINGS_GRID_IN_L2_IS_USED)
    client.subscribe(subGridL2Topic, [](const String &payload) {
      if (ParseMqttFloatValue(payload, gridInL2Watts))
        lastMQTTUpdateReceived = millis();
    });

  if (GENERAL_SETTINGS_GRID_IN_L3_IS_USED)
    client.subscribe(subGridL3Topic, [](const String &payload) {
      if (ParseMqttFloatValue(payload, gridInL3Watts))
        lastMQTTUpdateReceived = millis();
    });

  if (GENERAL_SETTINGS_PV_IS_USED)
    client.subscribe(subPvTopic, [](const String &payload) {
      if (ParseMqttFloatValue(payload, solarWatts))
        lastMQTTUpdateReceived = millis();
    });

  client.subscribe(subBatterySocTopic, [](const String &payload) {
    if (ParseMqttFloatValue(payload, batterySOC))
      lastMQTTUpdateReceived = millis();
  });

  client.subscribe(subBatteryPowerTopic, [](const String &payload) {
    if (ParseMqttFloatValue(payload, batteryPower))
    {
      UpdateChargingStateFallback();
      lastMQTTUpdateReceived = millis();
    }
  });

  client.subscribe(subBatteryTTGTopic, [](const String &payload) {
    if (ParseMqttFloatValue(payload, batteryTTG))
      lastMQTTUpdateReceived = millis();
  });

  if (GENERAL_SETTINGS_AC_OUT_L1_IS_USED)
    client.subscribe(subAcL1Topic, [](const String &payload) {
      if (ParseMqttFloatValue(payload, ACOutL1Watts))
        lastMQTTUpdateReceived = millis();
    });

  if (GENERAL_SETTINGS_AC_OUT_L2_IS_USED)
    client.subscribe(subAcL2Topic, [](const String &payload) {
      if (ParseMqttFloatValue(payload, ACOutL2Watts))
        lastMQTTUpdateReceived = millis();
    });

  if (GENERAL_SETTINGS_AC_OUT_L3_IS_USED)
    client.subscribe(subAcL3Topic, [](const String &payload) {
      if (ParseMqttFloatValue(payload, ACOutL3Watts))
        lastMQTTUpdateReceived = millis();
    });

  client.subscribe(subMultiplusModeTopic, [](const String &payload) {
    JsonDocument doc;
    if (deserializeJson(doc, payload) != DeserializationError::Ok)
      return;
    int workingMode = doc["value"] | 0;
    switch (workingMode)
    {
    case 1: currentMultiplusMode = ChargerOnly; break;
    case 2: currentMultiplusMode = InverterOnly; break;
    case 3: currentMultiplusMode = On; break;
    case 4: currentMultiplusMode = Off; break;
    default: currentMultiplusMode = Unknown; break;
    }
    if (currentMultiplusMode != Unknown)
      lastKnownMultiplusMode = currentMultiplusMode;

    if (pendingAction && currentMultiplusMode != Unknown)
    {
      pendingAction = false;
      pendingActionLabel = "";
      if (expectedMultiplusMode == currentMultiplusMode)
        SetToast("Schakeling bevestigd", 1200);
      else
        SetToast("Modus bijgewerkt", 1200);
    }
    expectedMultiplusMode = Unknown;
    lastMQTTUpdateReceived = millis();
    displayDirty = true;
  });

  SubscribeToChargingState();
  subscriptionsReady = true;
  PublishKeepAlive(true);
  DebugPrint("Abonnementen actief");
}

void TryDiscoverySubscriptions()
{
  if (!client.isMqttConnected())
    return;

  if (!installationDiscovered && !discoveryInstallSubscribed)
  {
    discoveryInstallTopic = "N/+/system/0/Serial";
    client.subscribe(discoveryInstallTopic, [](const String &topic, const String &payload) {
      (void)payload;
      int start = 2;
      int end = topic.indexOf("/system/0/Serial");
      if (end > start)
      {
        VictronInstallationID = topic.substring(start, end);
        installationDiscovered = true;
        DebugPrint("Installatie-ID gevonden: " + VictronInstallationID);
      }
    });
    discoveryInstallSubscribed = true;
  }

  if (installationDiscovered && discoveryInstallSubscribed)
  {
    client.unsubscribe(discoveryInstallTopic);
    discoveryInstallSubscribed = false;
  }

  if (installationDiscovered && !multiplusDiscovered && !discoveryVebusSubscribed)
  {
    discoveryVebusTopic = "N/" + VictronInstallationID + "/vebus/+/Mode";
    client.subscribe(discoveryVebusTopic, [](const String &topic, const String &payload) {
      (void)payload;
      int pos = topic.indexOf("/vebus/");
      if (pos >= 0 && topic.length() >= pos + 10)
      {
        MultiplusThreeDigitID = topic.substring(pos + 7, pos + 10);
        multiplusDiscovered = true;
        DebugPrint("Multiplus-ID gevonden: " + MultiplusThreeDigitID);
      }
    });
    discoveryVebusSubscribed = true;
  }

  if (multiplusDiscovered && discoveryVebusSubscribed)
  {
    client.unsubscribe(discoveryVebusTopic);
    discoveryVebusSubscribed = false;
  }

  if (installationDiscovered && SolarChargerThreeDigitID == "+" && !discoverySolarSubscribed)
  {
    discoverySolarTopic = "N/" + VictronInstallationID + "/solarcharger/+/Mode";
    client.subscribe(discoverySolarTopic, [](const String &topic, const String &payload) {
      (void)payload;
      int pos = topic.indexOf("/solarcharger/");
      if (pos >= 0 && topic.length() >= pos + 17)
      {
        SolarChargerThreeDigitID = topic.substring(pos + 14, pos + 17);
        DebugPrint("Solar charger ID gevonden: " + SolarChargerThreeDigitID);
        if (subscriptionsReady)
          SubscribeToChargingState();
      }
    });
    discoverySolarSubscribed = true;
  }

  if (SolarChargerThreeDigitID != "+" && discoverySolarSubscribed)
  {
    client.unsubscribe(discoverySolarTopic);
    discoverySolarSubscribed = false;
  }

  if (installationDiscovered)
    PublishKeepAlive();

  if (installationDiscovered && multiplusDiscovered)
    SubscribeToCoreTopics();
}

String StatusAgeText()
{
  if (lastMQTTUpdateReceived == 0)
    return "--s";
  unsigned long sec = (millis() - lastMQTTUpdateReceived) / 1000UL;
  return String(sec) + "s";
}

void DrawButtonBox(int x, int y, int w, int h, const String &text, uint16_t fillColor, uint16_t borderColor, bool active, TouchBox *box)
{
  uint16_t actualFill = fillColor;
  uint16_t textColor = TFT_WHITE;
  if (active && GENERAL_SETTINGS_TOON_TOUCH_FEEDBACK)
  {
    actualFill = TFT_WHITE;
    textColor = TFT_BLACK;
  }

  sprite.fillRoundRect(x, y, w, h, 8, actualFill);
  sprite.drawRoundRect(x, y, w, h, 8, borderColor);
  sprite.setTextDatum(MC_DATUM);
  sprite.setTextColor(textColor, actualFill);
  sprite.setTextFont(GENERAL_SETTINGS_GROTE_STATUSREGELS ? 4 : 2);
  sprite.drawString(text, x + w / 2, y + h / 2);
  if (box != nullptr)
    EnableBox(*box, x, y, x + w, y + h);
}

void DrawStatusBar()
{
  DisableBox(pageOverviewBox);
  DisableBox(pageDetailBox);
  DisableBox(pageSystemBox);
  DisableBox(lockBox);
  DisableBox(brightnessModeBox);

  if (!GENERAL_SETTINGS_TOON_STATUSBALK)
    return;

  sprite.fillRect(0, 0, TFT_WIDTH, 24, AppBackgroundColor());
  sprite.drawFastHLine(0, 23, TFT_WIDTH, TFT_DARKGREY);

  int badgeX = 6;
  DrawStatusBadge(badgeX, 3, 34, "WF", client.isWifiConnected(), TFT_GREEN);
  badgeX += 38;
  DrawStatusBadge(badgeX, 3, 36, "MQ", client.isMqttConnected(), TFT_CYAN);
  badgeX += 40;
  DrawStatusBadge(badgeX, 3, 36, touchLocked ? "SL" : "OK", !touchLocked, TFT_ORANGE);
  badgeX += 42;

  sprite.setTextDatum(TL_DATUM);
  sprite.setTextColor(TFT_LIGHTGREY, AppBackgroundColor());
  sprite.setTextFont(2);
  sprite.drawString(CurrentTimeString(), badgeX, 5);
  sprite.drawString(StatusAgeText(), badgeX + 52, 5);

  const int tabW = 46;
  const int gap = 3;
  const int tabY = 3;
  int visibleTabs = 3 + (GENERAL_SETTINGS_TOON_DETAIL_PAGINA ? 1 : 0) + (GENERAL_SETTINGS_TOON_SYSTEEM_PAGINA ? 1 : 0);
  int x = TFT_WIDTH - (visibleTabs * tabW + (visibleTabs - 1) * gap) - 4;

  DrawButtonBox(x, tabY, tabW, 18, "Over", currentPage == PAGE_OVERVIEW ? TFT_BLUE : TFT_BLACK, TFT_WHITE, false, &pageOverviewBox);
  x += tabW + gap;
  if (GENERAL_SETTINGS_TOON_DETAIL_PAGINA)
  {
    DrawButtonBox(x, tabY, tabW, 18, "Det", currentPage == PAGE_DETAIL ? TFT_BLUE : TFT_BLACK, TFT_WHITE, false, &pageDetailBox);
    x += tabW + gap;
  }
  if (GENERAL_SETTINGS_TOON_SYSTEEM_PAGINA)
  {
    DrawButtonBox(x, tabY, tabW, 18, "Sys", currentPage == PAGE_SYSTEM ? TFT_BLUE : TFT_BLACK, TFT_WHITE, false, &pageSystemBox);
    x += tabW + gap;
  }
  DrawButtonBox(x, tabY, tabW, 18, BrightnessModeLabel(), nightThemeActive ? TFT_NAVY : TFT_DARKGREY, TFT_WHITE, false, &brightnessModeBox);
  x += tabW + gap;
  DrawButtonBox(x, tabY, tabW, 18, touchLocked ? "Lock" : "Open", touchLocked ? TFT_RED : TFT_DARKGREY, TFT_WHITE, false, &lockBox);
}

void DrawOverviewPage()
{
  int yOffset = GENERAL_SETTINGS_TOON_STATUSBALK ? 28 : 6;
  int statusTileW = 150;
  int statusTileH = 38;
  int leftColX = 8;
  int rightColX = TFT_WIDTH - 170;
  int centerX = TFT_WIDTH / 2;
  int centerY = yOffset + 105;

  bool modeUnknown = (currentMultiplusMode == Unknown && lastKnownMultiplusMode == Unknown);
  bool chargerOn = ChargerIsOn(currentMultiplusMode == Unknown ? lastKnownMultiplusMode : currentMultiplusMode);
  bool inverterOn = InverterIsOn(currentMultiplusMode == Unknown ? lastKnownMultiplusMode : currentMultiplusMode);

  if (GENERAL_SETTINGS_SHOW_CHARGER_MODE)
    DrawButtonBox(leftColX, yOffset, statusTileW, statusTileH, "Lader " + ChargerStatus(), BlendColor565(StatusColor(chargerOn, modeUnknown), TFT_BLACK, 0.30F + PulseFactor() * (chargerOn ? 0.15F : 0.0F)), TFT_WHITE, SameTouchBox(chargerTouchBox, lastTouchedBox) && millis() < activeTouchUntil, &chargerTouchBox);
  else
    DisableBox(chargerTouchBox);

  if (GENERAL_SETTINGS_SHOW_INVERTER_MODE)
    DrawButtonBox(leftColX, TFT_HEIGHT - 46, statusTileW, statusTileH, "Omvormer " + InverterStatus(), BlendColor565(StatusColor(inverterOn, modeUnknown), TFT_BLACK, 0.30F + PulseFactor(1100UL) * (inverterOn ? 0.15F : 0.0F)), TFT_WHITE, SameTouchBox(inverterTouchBox, lastTouchedBox) && millis() < activeTouchUntil, &inverterTouchBox);
  else
    DisableBox(inverterTouchBox);

  DrawMetricCard(leftColX, yOffset + 50, 156, 58, "Zon", FormatWatts(dispSolarWatts), TFT_YELLOW, dispSolarWatts > 30.0F);
  DrawMetricCard(leftColX, yOffset + 116, 156, 58, "Net", FormatWatts(dispGridWatts), TFT_CYAN, fabsf(dispGridWatts) > 30.0F);
  DrawMetricCard(rightColX, yOffset + 50, 162, 58, "AC-belasting", FormatWatts(dispACLoadWatts), TFT_SILVER, dispACLoadWatts > 50.0F);
  DrawMetricCard(rightColX, yOffset + 116, 162, 58, "Accu verm.", FormatWatts(dispBatteryPower), TFT_ORANGE, fabsf(dispBatteryPower) > 25.0F);

  uint16_t batteryColor = dispBatterySOC <= 20 ? TFT_RED : (dispBatterySOC <= 40 ? TFT_YELLOW : TFT_GREEN);
  int outerRadius = 58;
  int innerRadius = 48;
  int endAngle = int(dispBatterySOC * 3.6F + 180.0F) % 360;
  sprite.drawSmoothArc(centerX, centerY, outerRadius, innerRadius, 180, 360, TFT_DARKGREY, TFT_BLACK);
  sprite.drawSmoothArc(centerX, centerY, outerRadius, innerRadius, 180, endAngle, batteryColor, TFT_BLACK);

  sprite.setTextDatum(MC_DATUM);
  sprite.setTextColor(batteryColor, TFT_BLACK);
  sprite.setTextFont(6);
  sprite.drawString(String((int)round(dispBatterySOC)) + "%", centerX, centerY - 4);
  sprite.setTextFont(GENERAL_SETTINGS_GROTE_NEDERLANDSE_LABELS ? 4 : 2);
  sprite.drawString("Accu", centerX, centerY - 36);

  String extra = "";
  if (GENERAL_SETTINGS_SHOW_ADDITIONAL_INFO)
  {
    if (GENERAL_SETTINGS_ADDITIONAL_INFO == 1) extra = FormatMinutes(dispBatteryTTG);
    else if (GENERAL_SETTINGS_ADDITIONAL_INFO == 4) extra = FormatWatts(dispBatteryPower);
    else extra = chargingState;
  }
  sprite.setTextFont(2);
  sprite.drawString(extra, centerX, centerY + 34);

  if (dispBatteryPower > 25.0F)
    sprite.fillTriangle(centerX - 8, centerY + 64, centerX + 8, centerY + 64, centerX, centerY + 46, batteryColor);
  else if (dispBatteryPower < -25.0F)
    sprite.fillTriangle(centerX - 8, centerY + 46, centerX + 8, centerY + 46, centerX, centerY + 64, batteryColor);

  if (GENERAL_SETTINGS_ENABLE_FLOW_ANIMATIE)
  {
    DrawFlowPath(leftColX + 150, yOffset + 78, centerX - 62, centerY - 18, TFT_YELLOW, dispSolarWatts > 30.0F ? 0.85F : 0.18F, false);
    DrawFlowPath(leftColX + 150, yOffset + 145, centerX - 62, centerY + 18, TFT_CYAN, fabsf(dispGridWatts) > 30.0F ? 0.78F : 0.15F, dispGridWatts < 0.0F);
    DrawFlowPath(centerX + 62, centerY, rightColX, yOffset + 78, TFT_SILVER, dispACLoadWatts > 50.0F ? 0.88F : 0.18F, false);
  }

  if (GENERAL_SETTINGS_ENABLE_SPARKLINES)
  {
    int sparkX = centerX - 70;
    int sparkY = TFT_HEIGHT - 42;
    sprite.fillRoundRect(sparkX, sparkY, 140, 28, 8, BlendColor565(TFT_BLACK, batteryColor, 0.12F));
    sprite.drawRoundRect(sparkX, sparkY, 140, 28, 8, TFT_DARKGREY);
    DrawSparkline(batteryPowerHistory, batteryColor, sparkX + 6, sparkY + 4, 128, 20, true);
  }

  DrawEnergyFlowSummary(TFT_HEIGHT - 74);

  if (pendingAction)
  {
    sprite.setTextColor(TFT_ORANGE, AppBackgroundColor());
    sprite.setTextFont(2);
    sprite.drawString(pendingActionLabel, centerX, TFT_HEIGHT - 52);
  }
}

void DrawDetailPage()
{
  int y = GENERAL_SETTINGS_TOON_STATUSBALK ? 30 : 8;
  DisableBox(detailRange1hBox);
  DisableBox(detailRange24hBox);
  DisableBox(detailRange7dBox);

  HistorySeries &series = CurrentHistorySeries();

  sprite.setTextDatum(TL_DATUM);
  sprite.setTextColor(TFT_WHITE, AppBackgroundColor());
  sprite.setTextFont(2);
  sprite.drawString("Grafieken", 8, y);
  DrawButtonBox(110, y - 2, 52, 18, "1u", selectedHistoryRange == HISTORY_RANGE_1H ? TFT_BLUE : TFT_BLACK, TFT_WHITE, false, &detailRange1hBox);
  DrawButtonBox(166, y - 2, 52, 18, "24u", selectedHistoryRange == HISTORY_RANGE_24H ? TFT_BLUE : TFT_BLACK, TFT_WHITE, false, &detailRange24hBox);
  DrawButtonBox(222, y - 2, 52, 18, "7d", selectedHistoryRange == HISTORY_RANGE_7D ? TFT_BLUE : TFT_BLACK, TFT_WHITE, false, &detailRange7dBox);
  sprite.drawString(String("Range: ") + HistoryRangeLabel(selectedHistoryRange), 282, y);

  DrawMetricCard(8, y + 18, 250, 54, "Net", FormatWatts(dispGridWatts), TFT_CYAN, fabsf(dispGridWatts) > 30.0F);
  DrawSparkline(series.grid, TFT_CYAN, 16, y + 54, 234, 12, false, series.size, series.index, series.wrapped);
  DrawMetricCard(278, y + 18, 250, 54, "Zon", FormatWatts(dispSolarWatts), TFT_YELLOW, dispSolarWatts > 30.0F);
  DrawSparkline(series.solar, TFT_YELLOW, 286, y + 54, 234, 12, false, series.size, series.index, series.wrapped);

  DrawMetricCard(8, y + 84, 250, 54, "AC-belasting", FormatWatts(dispACLoadWatts), TFT_SILVER, dispACLoadWatts > 50.0F);
  DrawSparkline(series.load, TFT_SILVER, 16, y + 120, 234, 12, false, series.size, series.index, series.wrapped);
  DrawMetricCard(278, y + 84, 250, 54, "Accu verm.", FormatWatts(dispBatteryPower), TFT_ORANGE, fabsf(dispBatteryPower) > 25.0F);
  DrawSparkline(series.battery, TFT_ORANGE, 286, y + 120, 234, 12, true, series.size, series.index, series.wrapped);

  sprite.setTextColor(TFT_LIGHTGREY, AppBackgroundColor());
  sprite.setTextFont(2);
  sprite.drawString("Modus: " + ModeToDutch(currentMultiplusMode == Unknown ? lastKnownMultiplusMode : currentMultiplusMode), 10, y + 146);
  sprite.drawString("Laadstatus: " + chargingState + "    Laatste update: " + StatusAgeText(), 10, y + 166);
  sprite.drawString("Accu: " + String((int)round(dispBatterySOC)) + "%    TTG: " + FormatMinutes(dispBatteryTTG), 10, y + 186);
  sprite.drawString("Grid L1/L2/L3: " + FormatWatts(gridInL1Watts) + " | " + FormatWatts(gridInL2Watts) + " | " + FormatWatts(gridInL3Watts), 10, y + 206);
}

void DrawSystemPage()
{
  int y = GENERAL_SETTINGS_TOON_STATUSBALK ? 30 : 8;
  int line = 18;
  sprite.setTextDatum(TL_DATUM);
  sprite.setTextColor(TFT_WHITE, AppBackgroundColor());
  sprite.setTextFont(2);

  sprite.drawString(programName, 8, y); y += line;
  sprite.drawString(programVersion, 8, y); y += line;
  sprite.drawString(String("Touch: ") + (boardHasTouch ? "ja" : "nee"), 8, y); y += line;
  sprite.drawString(String("Touch slot: ") + (touchLocked ? "AAN" : "UIT"), 8, y); y += line;
  sprite.drawString(String("Direct schakelen: ") + (GENERAL_SETTINGS_DIRECT_SCHAKELEN ? "ja" : "nee"), 8, y); y += line;
  sprite.drawString(String("Bevestiging popup: ") + (GENERAL_SETTINGS_TOON_BEVESTIGING_POPUP ? "ja" : "nee"), 8, y); y += line;
  sprite.drawString("Animatiesnelheid: " + AnimatieSnelheidLabel(), 8, y); y += line;
  sprite.drawString("Brightness modus: " + BrightnessModeLabel(), 8, y); y += line;
  sprite.drawString(String("Nachtthema: ") + (nightThemeActive ? "AAN" : "UIT"), 8, y); y += line;
  sprite.drawString("Helderheid: " + String(currentBrightness), 8, y); y += line;
  sprite.drawString("WiFi: " + String(client.isWifiConnected() ? "verbonden" : "niet verbonden"), 8, y); y += line;
  sprite.drawString("MQTT: " + String(client.isMqttConnected() ? "verbonden" : "niet verbonden"), 8, y); y += line;
  sprite.drawString("Installatie ID: " + VictronInstallationID, 8, y); y += line;
  sprite.drawString("Multiplus ID: " + MultiplusThreeDigitID, 8, y); y += line;
  sprite.drawString("Solar charger ID: " + SolarChargerThreeDigitID, 8, y); y += line;
  sprite.drawString("Tijd: " + CurrentTimeString(), 8, y); y += line;
  sprite.drawString("Laatste update: " + StatusAgeText(), 8, y);
}

void DrawDialog()
{
  if (dialogState != DIALOG_CONFIRM)
    return;

  int boxX = 78;
  int boxY = 55;
  int boxW = TFT_WIDTH - 156;
  int boxH = 122;
  sprite.fillRoundRect(boxX, boxY, boxW, boxH, 12, TFT_DARKGREY);
  sprite.drawRoundRect(boxX, boxY, boxW, boxH, 12, TFT_WHITE);
  sprite.setTextDatum(MC_DATUM);
  sprite.setTextColor(TFT_WHITE, TFT_DARKGREY);
  sprite.setTextFont(2);
  String target = (pendingFunction == Charger) ? "Lader" : "Omvormer";
  sprite.drawString("Wil je de " + target + " schakelen?", TFT_WIDTH / 2, boxY + 24);
  sprite.drawString("Huidige modus: " + ModeToDutch(currentMultiplusMode == Unknown ? lastKnownMultiplusMode : currentMultiplusMode), TFT_WIDTH / 2, boxY + 48);
  sprite.drawString("Nieuwe modus: " + ModeToDutch(ComputeDesiredMode(pendingFunction)), TFT_WIDTH / 2, boxY + 68);

  DrawButtonBox(boxX + 16, boxY + 86, 90, 24, "Ja", TFT_GREEN, TFT_WHITE, false, &dialogYesBox);
  DrawButtonBox(boxX + 122, boxY + 86, 90, 24, "Nee", TFT_RED, TFT_WHITE, false, &dialogNoBox);
  DrawButtonBox(boxX + 228, boxY + 86, 120, 24, "Annuleren", TFT_BLACK, TFT_WHITE, false, &dialogCancelBox);
}

void DrawToast()
{
  if (toastUntil == 0 || millis() > toastUntil)
    return;

  int h = 22;
  uint16_t toastBg = nightThemeActive ? TFT_DARKGREY : TFT_NAVY;
  sprite.fillRect(0, TFT_HEIGHT - h, TFT_WIDTH, h, toastBg);
  sprite.setTextColor(TFT_WHITE, toastBg);
  sprite.setTextDatum(MC_DATUM);
  sprite.setTextFont(2);
  sprite.drawString(toastMessage, TFT_WIDTH / 2, TFT_HEIGHT - 11);
}

// -----------------------------------------------------------------------------
// Rendering and input loop
// -----------------------------------------------------------------------------
void UpdateDisplay()
{
  if (!theDisplayIsCurrentlyOn)
    return;

  unsigned long now = millis();
  unsigned long statusAgeSeconds = (lastMQTTUpdateReceived == 0) ? 0UL : ((now - lastMQTTUpdateReceived) / 1000UL);
  bool statusSecondChanged = (!statusSecondInitialized || statusAgeSeconds != lastStatusSecondRendered);
  bool toastActive = (toastUntil != 0 && now <= toastUntil);
  bool touchFeedbackActive = GENERAL_SETTINGS_TOON_TOUCH_FEEDBACK && now < activeTouchUntil;
  bool dialogVisible = (dialogState == DIALOG_CONFIRM);
  bool pendingVisible = pendingAction;
  bool animationActive = (GENERAL_SETTINGS_ENABLE_FLOW_ANIMATIE && currentPage == PAGE_OVERVIEW) || touchFeedbackActive || dialogVisible || pendingVisible || toastActive;
  unsigned long minIntervalMs = animationActive ? AnimatieFrameIntervalMs() : (unsigned long)GENERAL_SETTINGS_SECONDS_BETWEEN_DISPLAY_UPDATES * 1000UL;
  if (!displayDirty && !statusSecondChanged)
    return;

  if (now - lastDisplayUpdate < minIntervalMs)
    return;
  lastDisplayUpdate = now;
  if (statusSecondChanged)
  {
    statusSecondInitialized = true;
    lastStatusSecondRendered = statusAgeSeconds;
  }
  displayDirty = false;

  sprite.fillSprite(AppBackgroundColor());
  DisableBox(chargerTouchBox);
  DisableBox(inverterTouchBox);
  DisableBox(dialogYesBox);
  DisableBox(dialogNoBox);
  DisableBox(dialogCancelBox);
  DisableBox(detailRange1hBox);
  DisableBox(detailRange24hBox);
  DisableBox(detailRange7dBox);

  DrawStatusBar();

  if (!client.isWifiConnected())
  {
    sprite.setTextDatum(MC_DATUM);
    sprite.setTextColor(TFT_WHITE, TFT_BLACK);
    sprite.setTextFont(4);
    sprite.drawString("Wachten op wifi", TFT_WIDTH / 2, TFT_HEIGHT / 2);
    DrawToast();
    RefreshDisplay();
    return;
  }

  if (!client.isMqttConnected())
  {
    sprite.setTextDatum(MC_DATUM);
    sprite.setTextColor(TFT_WHITE, TFT_BLACK);
    sprite.setTextFont(4);
    sprite.drawString("Wachten op MQTT", TFT_WIDTH / 2, TFT_HEIGHT / 2);
    DrawToast();
    RefreshDisplay();
    return;
  }

  if (lastMQTTUpdateReceived != 0 && millis() - lastMQTTUpdateReceived > (unsigned long)GENERAL_SETTINGS_MQTT_DATA_TIMEOUT_SECONDS * 1000UL)
  {
    sprite.setTextDatum(MC_DATUM);
    sprite.setTextColor(TFT_ORANGE, TFT_BLACK);
    sprite.setTextFont(2);
    sprite.drawString("Let op: MQTT data is oud", TFT_WIDTH / 2, 42);
  }

  if (currentPage == PAGE_DETAIL && !GENERAL_SETTINGS_TOON_DETAIL_PAGINA)
    currentPage = PAGE_OVERVIEW;
  if (currentPage == PAGE_SYSTEM && !GENERAL_SETTINGS_TOON_SYSTEEM_PAGINA)
    currentPage = PAGE_OVERVIEW;

  if (currentPage == PAGE_OVERVIEW)
    DrawOverviewPage();
  else if (currentPage == PAGE_DETAIL)
    DrawDetailPage();
  else
    DrawSystemPage();

  DrawDialog();
  DrawToast();
  RefreshDisplay();
}

void HandleButtonAction(multiplusFunction option)
{
  if (!GENERAL_SETTINGS_ALLOW_CHANGING_INVERTER_AND_CHARGER_MODES)
    return;
  if (dialogState == DIALOG_CONFIRM)
    return;

  if (GENERAL_SETTINGS_DIRECT_SCHAKELEN && !GENERAL_SETTINGS_TOON_BEVESTIGING_POPUP)
    ApplyMultiplusMode(ComputeDesiredMode(option));
  else
  {
    pendingFunction = option;
    dialogState = DIALOG_CONFIRM;
  }
}

void CheckButtons()
{
  static unsigned long lastButtonAction = 0UL;

  if (!theDisplayIsCurrentlyOn)
  {
    if (digitalRead(topButton) == 0 || digitalRead(bottomButton) == 0)
    {
      SetTheDisplayOn(true);
      SetKeepDisplayOnTimeOut(GENERAL_SETTINGS_DISPLAY_TIMEOUT_MINUTEN);
    }
    return;
  }

  if (millis() - lastButtonAction < GENERAL_SETTINGS_BUTTON_COOLDOWN_MS)
    return;

  if (digitalRead(topButton) == 0)
  {
    HandleButtonAction(Charger);
    lastButtonAction = millis();
  }
  else if (digitalRead(bottomButton) == 0)
  {
    HandleButtonAction(Inverter);
    lastButtonAction = millis();
  }
}

void ProcessTap(int16_t tx, int16_t ty)
{
  if (millis() - lastTouchAccepted < GENERAL_SETTINGS_TOUCH_COOLDOWN_MS)
    return;
  lastTouchAccepted = millis();

  if (!theDisplayIsCurrentlyOn)
  {
    if (GENERAL_SETTINGS_WAKE_DISPLAY_ON_TOUCH)
    {
      SetTheDisplayOn(true);
      SetKeepDisplayOnTimeOut(GENERAL_SETTINGS_DISPLAY_TIMEOUT_MINUTEN);
    }
    return;
  }

  ResetKeepDisplayOnStartTime();

  if (dialogState == DIALOG_CONFIRM)
  {
    if (PointInTouchBox(tx, ty, dialogYesBox))
    {
      RememberTouchedBox(dialogYesBox);
      ConfirmDialog();
    }
    else if (PointInTouchBox(tx, ty, dialogNoBox) || PointInTouchBox(tx, ty, dialogCancelBox))
    {
      RememberTouchedBox(PointInTouchBox(tx, ty, dialogNoBox) ? dialogNoBox : dialogCancelBox);
      CancelDialog();
    }
    return;
  }

  if (PointInTouchBox(tx, ty, pageOverviewBox))
  {
    RememberTouchedBox(pageOverviewBox);
    currentPage = PAGE_OVERVIEW;
    displayDirty = true;
    return;
  }
  if (PointInTouchBox(tx, ty, pageDetailBox))
  {
    RememberTouchedBox(pageDetailBox);
    currentPage = PAGE_DETAIL;
    displayDirty = true;
    return;
  }
  if (PointInTouchBox(tx, ty, pageSystemBox))
  {
    RememberTouchedBox(pageSystemBox);
    currentPage = PAGE_SYSTEM;
    displayDirty = true;
    return;
  }
  if (PointInTouchBox(tx, ty, lockBox))
  {
    RememberTouchedBox(lockBox);
    touchLocked = !touchLocked;
    SetToast(touchLocked ? "Touch vergrendeld" : "Touch ontgrendeld", 1500);
    displayDirty = true;
    return;
  }
  if (PointInTouchBox(tx, ty, brightnessModeBox))
  {
    RememberTouchedBox(brightnessModeBox);
    if (brightnessMode == BRIGHTNESS_AUTO)
      brightnessMode = BRIGHTNESS_DAY;
    else if (brightnessMode == BRIGHTNESS_DAY)
      brightnessMode = BRIGHTNESS_NIGHT;
    else
      brightnessMode = BRIGHTNESS_AUTO;
    UpdateBrightness();
    SetToast(String("Brightness: ") + BrightnessModeLabel(), 1200);
    displayDirty = true;
    return;
  }

  if (currentPage == PAGE_DETAIL)
  {
    if (PointInTouchBox(tx, ty, detailRange1hBox))
    {
      RememberTouchedBox(detailRange1hBox);
      selectedHistoryRange = HISTORY_RANGE_1H;
      displayDirty = true;
      return;
    }
    if (PointInTouchBox(tx, ty, detailRange24hBox))
    {
      RememberTouchedBox(detailRange24hBox);
      selectedHistoryRange = HISTORY_RANGE_24H;
      displayDirty = true;
      return;
    }
    if (PointInTouchBox(tx, ty, detailRange7dBox))
    {
      RememberTouchedBox(detailRange7dBox);
      selectedHistoryRange = HISTORY_RANGE_7D;
      displayDirty = true;
      return;
    }
  }

  if (touchLocked)
  {
    SetToast("Touch is vergrendeld", 1200);
    return;
  }

  if (currentPage == PAGE_OVERVIEW)
  {
    if (PointInTouchBox(tx, ty, chargerTouchBox))
    {
      RememberTouchedBox(chargerTouchBox);
      StartToggleRequest(Charger);
    }
    else if (PointInTouchBox(tx, ty, inverterTouchBox))
    {
      RememberTouchedBox(inverterTouchBox);
      StartToggleRequest(Inverter);
    }
  }
}

void CheckTouch()
{
  if (!boardHasTouch || !GENERAL_SETTINGS_USE_TOUCH_DISPLAY)
    return;

  static bool touchLatched = false;
  static int16_t lastTx = 0;
  static int16_t lastTy = 0;

  int16_t tx = 0;
  int16_t ty = 0;
  bool touched = amoled.getPoint(&tx, &ty) > 0;

  if (touched)
  {
    lastTx = tx;
    lastTy = ty;
  }

  if (!touched && touchLatched)
    ProcessTap(lastTx, lastTy);

  touchLatched = touched;
}

void onConnectionEstablished()
{
  subscriptionsReady = false;
  solarStateSubscribed = false;
  displayDirty = true;
  if (VictronInstallationID != "+")
    installationDiscovered = true;
  if (MultiplusThreeDigitID != "+")
    multiplusDiscovered = true;
  TryDiscoverySubscriptions();
}

void SetupDisplay()
{
  sprite.createSprite(TFT_WIDTH, TFT_HEIGHT);
  sprite.setSwapBytes(true);

  bool ok = amoled.beginAMOLED_191(true);
  if (!ok)
  {
    DebugPrint("AMOLED init mislukt");
    while (true)
      delay(1000);
  }

  boardHasTouch = amoled.hasTouch();
  SetDisplayOrientation();
  amoled.setBrightness((uint8_t)currentBrightness);
  SetKeepDisplayOnTimeOut(GENERAL_SETTINGS_DISPLAY_TIMEOUT_MINUTEN);
  SetTheDisplayOn(true);
}

void SetupHistoryBuffers()
{
  history1h.size = 60;
  history1h.intervalMs = 60000UL;
  history1h.lastSample = millis();

  history24h.size = 96;
  history24h.intervalMs = 15UL * 60000UL;
  history24h.lastSample = millis();

  history7d.size = 84;
  history7d.intervalMs = 2UL * 60UL * 60000UL;
  history7d.lastSample = millis();
}

void setup()
{
  if (generalDebugOutput)
    Serial.begin(GENERAL_SETTINGS_SERIAL_MONITOR_SPEED);

  SetupTopAndBottomButtons();
  SetupDisplay();
  RunPremiumBootAnimation();
  SetupHistoryBuffers();

  if (VictronInstallationID != "+")
    installationDiscovered = true;
  if (MultiplusThreeDigitID != "+")
    multiplusDiscovered = true;

  DebugPrint(String(programName) + " - " + String(programVersion));
  displayDirty = true;
}

void loop()
{
  bool animatedChanged = false;
  client.loop();
  PublishKeepAlive();
  SyncTimeIfNeeded();
  UpdateBrightness();
  animatedChanged = UpdateAnimatedValues();
  if (animatedChanged)
    displayDirty = true;
  SampleHistoryIfNeeded();

  if (!subscriptionsReady)
    TryDiscoverySubscriptions();

  CheckButtons();
  CheckTouch();

  if (pendingAction && millis() - pendingActionStarted > GENERAL_SETTINGS_PENDING_ACTION_TIMEOUT_MS)
  {
    pendingAction = false;
    pendingActionLabel = "";
    expectedMultiplusMode = Unknown;
    SetToast("Nog geen bevestigde modusupdate", 1800);
  }

  if (GENERAL_SETTINGS_TURN_ON_DISPLAY_AT_SPECIFIC_TIMES_ONLY && IsKeepDisplayOnTimedOut())
    SetTheDisplayOn(false);

  if (toastUntil != 0 && millis() > toastUntil)
  {
    toastUntil = 0;
    toastMessage = "";
    displayDirty = true;
  }

  UpdateDisplay();
}

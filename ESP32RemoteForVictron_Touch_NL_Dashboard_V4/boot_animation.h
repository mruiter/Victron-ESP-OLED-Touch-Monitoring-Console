#pragma once

/*
  Victron ESP OLED Touch Monitoring Console - Premium Boot Animation
  ------------------------------------------------------------------
  Header-only boot animation for TFT_eSPI sprite displays.

  Integration in your .ino:
    1) Add below your includes:
         #include "boot_animation.h"

    2) After sprite.createSprite(...) and display orientation/brightness are ready,
       call:
         RunPremiumBootAnimation();

  This file expects these globals/functions to exist in the main sketch:
    - TFT_eSprite sprite;
    - void RefreshDisplay();
    - const char *programVersion;
    - bool client.isWifiConnected()      [optional via BootStatus]
    - bool client.isMqttConnected()      [optional via BootStatus]

  The animation is asset-free: no bitmap arrays, no external fonts.
  It is designed for 536x240 AMOLED landscape.
*/

#ifndef BOOT_ANIMATION_ENABLED
#define BOOT_ANIMATION_ENABLED true
#endif

#ifndef BOOT_ANIMATION_DURATION_MS
#define BOOT_ANIMATION_DURATION_MS 2800UL
#endif

#ifndef BOOT_ANIMATION_MIN_FRAME_MS
#define BOOT_ANIMATION_MIN_FRAME_MS 26UL
#endif

#ifndef BOOT_ANIMATION_HARD_TIMEOUT_MS
#define BOOT_ANIMATION_HARD_TIMEOUT_MS 5000UL
#endif

#ifndef BOOT_ANIMATION_TITLE
#define BOOT_ANIMATION_TITLE "VICTRON ESP"
#endif

#ifndef BOOT_ANIMATION_SUBTITLE
#define BOOT_ANIMATION_SUBTITLE "Monitoring Console"
#endif

#ifndef BOOT_ANIMATION_ACCENT
#define BOOT_ANIMATION_ACCENT TFT_CYAN
#endif

#ifndef BOOT_ANIMATION_SUCCESS
#define BOOT_ANIMATION_SUCCESS TFT_GREEN
#endif

#ifndef BOOT_ANIMATION_SOLAR
#define BOOT_ANIMATION_SOLAR TFT_YELLOW
#endif

#ifndef BOOT_ANIMATION_GRID
#define BOOT_ANIMATION_GRID TFT_BLUE
#endif

#ifndef BOOT_ANIMATION_BATTERY
#define BOOT_ANIMATION_BATTERY TFT_GREEN
#endif

extern TFT_eSprite sprite;
extern void RefreshDisplay();
extern const char *programVersion;
extern EspMQTTClient client;
extern bool boardHasTouch;
extern void QueueBootCrossfadeFromCurrentFrame();

struct BootStatus
{
  bool wifiReady = false;
  bool mqttReady = false;
  bool touchReady = false;
  bool victronReady = false;
};

static uint16_t BootBlend565(uint16_t c1, uint16_t c2, float ratio)
{
  if (ratio <= 0.0F) return c1;
  if (ratio >= 1.0F) return c2;
  int r1 = (c1 >> 11) & 0x1F, g1 = (c1 >> 5) & 0x3F, b1 = c1 & 0x1F;
  int r2 = (c2 >> 11) & 0x1F, g2 = (c2 >> 5) & 0x3F, b2 = c2 & 0x1F;
  int r = r1 + (int)roundf((r2 - r1) * ratio);
  int g = g1 + (int)roundf((g2 - g1) * ratio);
  int b = b1 + (int)roundf((b2 - b1) * ratio);
  return (uint16_t)((r << 11) | (g << 5) | b);
}

static float BootEaseOutCubic(float x)
{
  if (x < 0.0F) return 0.0F;
  if (x > 1.0F) return 1.0F;
  float t = 1.0F - x;
  return 1.0F - t * t * t;
}

static float BootEaseInOut(float x)
{
  if (x < 0.0F) return 0.0F;
  if (x > 1.0F) return 1.0F;
  return x < 0.5F ? 4.0F * x * x * x : 1.0F - powf(-2.0F * x + 2.0F, 3.0F) / 2.0F;
}

static void BootDrawGlowCircle(int cx, int cy, int r, uint16_t color, float strength)
{
  if (strength <= 0.01F) return;
  for (int i = 5; i >= 1; --i)
  {
    float layer = strength * (float)i / 5.0F;
    uint16_t c = BootBlend565(TFT_BLACK, color, 0.08F + layer * 0.12F);
    sprite.drawCircle(cx, cy, r + i * 3, c);
  }
}

static void BootDrawArcDots(int cx, int cy, int radius, float progress, uint16_t color)
{
  int dots = 48;
  int active = (int)roundf(dots * progress);
  for (int i = 0; i < dots; ++i)
  {
    float angle = -1.5708F + ((float)i / (float)dots) * 6.2831853F;
    int x = cx + (int)roundf(cosf(angle) * radius);
    int y = cy + (int)roundf(sinf(angle) * radius);
    uint16_t c = (i <= active) ? BootBlend565(color, TFT_WHITE, 0.25F) : BootBlend565(TFT_DARKGREY, TFT_BLACK, 0.35F);
    int rr = (i <= active) ? 2 : 1;
    sprite.fillCircle(x, y, rr, c);
  }
}

static void BootDrawEnergyNode(int x, int y, const char *label, uint16_t color, float phase)
{
  float p = (sinf(phase) + 1.0F) * 0.5F;
  uint16_t bg = BootBlend565(TFT_BLACK, color, 0.12F + p * 0.08F);
  sprite.fillRoundRect(x - 33, y - 18, 66, 36, 8, bg);
  sprite.drawRoundRect(x - 33, y - 18, 66, 36, 8, BootBlend565(color, TFT_WHITE, 0.20F));

  sprite.setTextDatum(MC_DATUM);
  sprite.setTextFont(2);
  sprite.setTextColor(TFT_WHITE, bg);
  sprite.drawString(label, x, y);
}

static void BootDrawEnergyLine(int x1, int y1, int x2, int y2, uint16_t color, float phase, float alpha)
{
  uint16_t line = BootBlend565(TFT_DARKGREY, color, alpha);
  sprite.drawLine(x1, y1, x2, y2, line);

  for (int i = 0; i < 3; ++i)
  {
    float t = fmodf(phase + (float)i / 3.0F, 1.0F);
    int x = x1 + (int)roundf((x2 - x1) * t);
    int y = y1 + (int)roundf((y2 - y1) * t);
    sprite.fillCircle(x, y, 2, BootBlend565(color, TFT_WHITE, 0.45F));
  }
}

static void BootDrawStatusPill(int x, int y, const char *label, bool ready, float reveal)
{
  int w = 104;
  int h = 18;
  uint16_t accent = ready ? BOOT_ANIMATION_SUCCESS : TFT_DARKGREY;
  uint16_t bg = BootBlend565(TFT_BLACK, accent, ready ? 0.26F : 0.10F);

  sprite.fillRoundRect(x, y, w, h, 8, bg);
  sprite.drawRoundRect(x, y, w, h, 8, BootBlend565(accent, TFT_WHITE, ready ? 0.28F : 0.08F));

  sprite.setTextDatum(MC_DATUM);
  sprite.setTextFont(1);
  sprite.setTextColor(BootBlend565(TFT_DARKGREY, TFT_WHITE, reveal), bg);
  sprite.drawString(label, x + w / 2, y + h / 2);
}

static void BootDrawLogoMark(int cx, int cy, float t)
{
  float pulse = (sinf(t * 6.2831853F * 2.0F) + 1.0F) * 0.5F;

  BootDrawGlowCircle(cx, cy, 34, BOOT_ANIMATION_ACCENT, 0.45F + pulse * 0.30F);
  sprite.drawCircle(cx, cy, 33, BootBlend565(BOOT_ANIMATION_ACCENT, TFT_WHITE, 0.35F));
  sprite.drawCircle(cx, cy, 25, BootBlend565(TFT_BLUE, BOOT_ANIMATION_ACCENT, 0.55F));

  // Stylized V + energy bolt
  int topY = cy - 15;
  int botY = cy + 17;
  sprite.drawWideLine(cx - 18, topY, cx - 5, botY, 3, BOOT_ANIMATION_ACCENT);
  sprite.drawWideLine(cx + 18, topY, cx + 5, botY, 3, BOOT_ANIMATION_ACCENT);

  uint16_t bolt = BootBlend565(BOOT_ANIMATION_SOLAR, TFT_WHITE, pulse * 0.28F);
  sprite.fillTriangle(cx + 2, cy - 20, cx - 6, cy + 2, cx + 5, cy + 2, bolt);
  sprite.fillTriangle(cx - 2, cy + 20, cx + 6, cy - 2, cx - 5, cy - 2, bolt);

  // Orbiting data dots
  for (int i = 0; i < 4; ++i)
  {
    float a = t * 6.2831853F + i * 1.5708F;
    int x = cx + (int)roundf(cosf(a) * 45);
    int y = cy + (int)roundf(sinf(a) * 45);
    uint16_t c = (i % 2 == 0) ? BOOT_ANIMATION_GRID : BOOT_ANIMATION_SOLAR;
    sprite.fillCircle(x, y, 3, BootBlend565(c, TFT_WHITE, 0.25F));
  }
}

static void DrawPremiumBootFrame(float rawProgress, const BootStatus &status)
{
  float p = BootEaseOutCubic(rawProgress);
  float phase = rawProgress * 2.2F;

  sprite.fillSprite(TFT_BLACK);

  // Subtle background grid / horizon
  for (int x = 0; x < 536; x += 24)
    sprite.drawFastVLine(x, 0, 240, BootBlend565(TFT_BLACK, TFT_DARKGREY, 0.28F));
  for (int y = 0; y < 240; y += 24)
    sprite.drawFastHLine(0, y, 536, BootBlend565(TFT_BLACK, TFT_DARKGREY, 0.22F));

  // Ambient left/right gradients
  for (int i = 0; i < 7; ++i)
  {
    sprite.drawRoundRect(6 + i * 2, 7 + i * 2, 524 - i * 4, 226 - i * 4, 14, BootBlend565(TFT_BLACK, BOOT_ANIMATION_ACCENT, 0.08F - i * 0.007F));
  }

  // Center logo
  int logoX = 118;
  int logoY = 102;
  BootDrawLogoMark(logoX, logoY, phase);
  BootDrawArcDots(logoX, logoY, 58, p, BOOT_ANIMATION_ACCENT);

  // Main title block
  int titleX = 236;
  sprite.setTextDatum(TL_DATUM);
  sprite.setTextFont(4);
  sprite.setTextColor(BootBlend565(TFT_DARKGREY, TFT_WHITE, BootEaseInOut(rawProgress * 1.4F)), TFT_BLACK);
  sprite.drawString(BOOT_ANIMATION_TITLE, titleX, 58);

  sprite.setTextFont(2);
  sprite.setTextColor(BootBlend565(TFT_DARKGREY, BOOT_ANIMATION_ACCENT, BootEaseInOut(rawProgress * 1.6F)), TFT_BLACK);
  sprite.drawString(BOOT_ANIMATION_SUBTITLE, titleX + 2, 91);

  sprite.setTextFont(1);
  sprite.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  sprite.drawString(String("Firmware: ") + String(programVersion), titleX + 2, 113);

  // Animated energy micro-map
  int mapY = 158;
  int netX = 234, battX = 340, pvX = 446;
  BootDrawEnergyLine(netX + 34, mapY, battX - 34, mapY, BOOT_ANIMATION_GRID, phase, 0.85F);
  BootDrawEnergyLine(pvX - 34, mapY, battX + 34, mapY, BOOT_ANIMATION_SOLAR, phase + 0.25F, 0.85F);
  BootDrawEnergyLine(battX, mapY - 18, battX, mapY - 48, BOOT_ANIMATION_BATTERY, phase + 0.5F, 0.70F);

  BootDrawEnergyNode(netX, mapY, "NET", BOOT_ANIMATION_GRID, phase);
  BootDrawEnergyNode(battX, mapY, "ACCU", BOOT_ANIMATION_BATTERY, phase + 1.4F);
  BootDrawEnergyNode(pvX, mapY, "ZON", BOOT_ANIMATION_SOLAR, phase + 2.1F);

  // Status pills
  float reveal = BootEaseInOut((rawProgress - 0.15F) / 0.55F);
  BootDrawStatusPill(236, 198, "WiFi", status.wifiReady, reveal);
  BootDrawStatusPill(346, 198, "MQTT", status.mqttReady, reveal);
  BootDrawStatusPill(456, 198, "Touch", status.touchReady, reveal);

  // Progress bar
  int bx = 236, by = 132, bw = 262, bh = 8;
  sprite.fillRoundRect(bx, by, bw, bh, 4, BootBlend565(TFT_BLACK, TFT_DARKGREY, 0.60F));
  int fillW = (int)roundf((float)bw * p);
  if (fillW > 0)
  {
    uint16_t barColor = BootBlend565(BOOT_ANIMATION_ACCENT, BOOT_ANIMATION_SUCCESS, rawProgress);
    sprite.fillRoundRect(bx, by, fillW, bh, 4, barColor);
    sprite.fillCircle(bx + fillW, by + bh / 2, 4, BootBlend565(barColor, TFT_WHITE, 0.40F));
  }

  // Footer
  sprite.setTextDatum(BR_DATUM);
  sprite.setTextFont(1);
  sprite.setTextColor(BootBlend565(TFT_DARKGREY, TFT_LIGHTGREY, 0.70F), TFT_BLACK);
  sprite.drawString("Victron MQTT Console wordt gestart", 528, 232);
}

static void RunPremiumBootAnimation(const BootStatus &initialStatus = BootStatus())
{
#if BOOT_ANIMATION_ENABLED
  BootStatus status = initialStatus;
  unsigned long start = millis();
  unsigned long lastFrame = 0;
  unsigned long hardStop = start + BOOT_ANIMATION_HARD_TIMEOUT_MS;

  while (true)
  {
    unsigned long now = millis();
    if ((now - start) >= BOOT_ANIMATION_DURATION_MS)
      break;
    if ((long)(now - hardStop) >= 0)
      break;

    if (now - lastFrame < BOOT_ANIMATION_MIN_FRAME_MS)
      continue;
    lastFrame = now;

    client.loop();
    status.wifiReady = client.isWifiConnected();
    status.mqttReady = client.isMqttConnected();
    status.touchReady = boardHasTouch;

    float progress = (float)(now - start) / (float)BOOT_ANIMATION_DURATION_MS;
    if (progress > 1.0F) progress = 1.0F;

    DrawPremiumBootFrame(progress, status);
    RefreshDisplay();
    yield();
  }

  // Final polished settle frame
  status.wifiReady = client.isWifiConnected();
  status.mqttReady = client.isMqttConnected();
  status.touchReady = boardHasTouch;
  DrawPremiumBootFrame(1.0F, status);
  RefreshDisplay();
  QueueBootCrossfadeFromCurrentFrame();
#endif
}

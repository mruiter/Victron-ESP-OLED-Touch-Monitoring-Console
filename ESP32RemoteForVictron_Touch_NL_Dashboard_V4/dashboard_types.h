#pragma once

#include <Arduino.h>

enum HistoryRange : uint8_t
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

enum multiplusMode : uint8_t
{
  ChargerOnly,
  InverterOnly,
  On,
  Off,
  Unknown
};

enum multiplusFunction : uint8_t
{
  Charger,
  Inverter
};

struct TouchBox
{
  int16_t x1, y1, x2, y2;
  bool enabled;
};

#pragma once

#include <Arduino.h>

enum AppLanguage
{
  LANG_ENGLISH = 0,
  LANG_DUTCH,
  LANG_GERMAN,
  LANG_FRENCH,
  LANG_SPANISH,
  LANG_ITALIAN,
  LANG_POLISH,
  LANG_ARABIC,
  LANG_TURKISH,
  LANG_COUNT
};

struct LanguagePack
{
  const char *languageName;
  const char *programName;
  const char *programVersion;

  const char *autoLabel;
  const char *dayLabel;
  const char *nightLabel;
  const char *slowLabel;
  const char *normalLabel;
  const char *fastLabel;

  const char *onLabel;
  const char *offLabel;
  const char *unknownLabel;
  const char *yesLabel;
  const char *noLabel;
  const char *cancelLabel;

  const char *chargerLabel;
  const char *inverterLabel;
  const char *solarLabel;
  const char *gridLabel;
  const char *acLoadLabel;
  const char *batteryLabel;
  const char *batteryPowerLabel;

  const char *chargingLabel;
  const char *dischargingLabel;
  const char *idleLabel;
  const char *errorLabel;
  const char *bulkLabel;
  const char *absorptionLabel;
  const char *floatLabel;
  const char *storageLabel;
  const char *equalizeLabel;

  const char *tabOverview;
  const char *tabDetail;
  const char *tabSystem;
  const char *lockLabel;
  const char *unlockLabel;

  const char *chartsLabel;
  const char *rangeLabel;
  const char *modeLabel;
  const char *chargeStatusLabel;
  const char *lastUpdateLabel;
  const char *touchLabel;
  const char *touchLockLabel;
  const char *directSwitchLabel;
  const char *confirmPopupLabel;
  const char *animationSpeedLabel;
  const char *brightnessModeLabel;
  const char *nightThemeLabel;
  const char *brightnessLabel;
  const char *languageLabel;
  const char *wifiLabel;
  const char *mqttLabel;
  const char *installationIdLabel;
  const char *multiplusIdLabel;
  const char *solarChargerIdLabel;
  const char *timeLabel;

  const char *connectedLabel;
  const char *notConnectedLabel;

  const char *switchQuestionTemplate;
  const char *currentModeLabel;
  const char *newModeLabel;

  const char *waitWifiLabel;
  const char *waitMqttLabel;
  const char *oldMqttDataLabel;

  const char *toastSwitchBlockedUnknown;
  const char *toastSwitching;
  const char *toastTouchLocked;
  const char *toastControlsDisabled;
  const char *toastStatusUnknownWait;
  const char *toastSwitchConfirmed;
  const char *toastModeUpdated;
  const char *toastTouchLockedNow;
  const char *toastTouchUnlockedNow;
  const char *toastBrightnessPrefix;
  const char *toastLanguagePrefix;
  const char *toastNoConfirmedModeUpdate;

  const char *flowSolarToBattery;
  const char *flowBatteryToAc;
  const char *flowGridToAc;
  const char *flowAcToGrid;
  const char *flowIdle;
};

const LanguagePack &GetLanguagePack(AppLanguage lang);
AppLanguage NormalizeLanguageSetting(int languageValue);
AppLanguage NextLanguage(AppLanguage lang);

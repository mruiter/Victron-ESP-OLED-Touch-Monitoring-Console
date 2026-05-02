#define GENERAL_SETTINGS_USB_ON_THE_LEFT                                true
#define GENERAL_SETTINGS_ALLOW_CHANGING_INVERTER_AND_CHARGER_MODES      true
#define GENERAL_SETTINGS_SHOW_CHARGER_MODE                              true
#define GENERAL_SETTINGS_SHOW_INVERTER_MODE                             true
#define GENERAL_SETTINGS_SECONDS_BETWEEN_DISPLAY_UPDATES                1
#define GENERAL_SETTINGS_SEND_PERIODICAL_KEEP_ALIVE_REQUESTS            true
#define GENERAL_SETTINGS_SEND_PERIODICAL_KEEP_ALIVE_REQUESTS_INTERVAL   30000
#define GENERAL_SETTINGS_DEBUG_OUTPUT_LEVEL                             1
#define GENERAL_SETTINGS_SERIAL_MONITOR_SPEED                           115200

#define GENERAL_SETTINGS_USE_TOUCH_DISPLAY                              true
#define GENERAL_SETTINGS_WAKE_DISPLAY_ON_TOUCH                          true
#define GENERAL_SETTINGS_TOUCH_AVAILABLE                                0
// Zet op 1 wanneer jouw TFT_eSPI User_Setup touch ondersteunt (getTouch()).
#define GENERAL_SETTINGS_TOUCH_VERGRENDELING_BIJ_START                  false
#define GENERAL_SETTINGS_TOUCH_COOLDOWN_MS                              500
#define GENERAL_SETTINGS_TOUCH_FEEDBACK_MS                              180
#define GENERAL_SETTINGS_BUTTON_COOLDOWN_MS                             350

#define GENERAL_SETTINGS_SHOW_ADDITIONAL_INFO                           true
#define GENERAL_SETTINGS_DEFAULT_LANGUAGE                             0
// 0 = English
// 1 = Nederlands
// 2 = Deutsch
// 3 = Français
// 4 = Español
// 5 = Italiano
// 6 = Polski
// 7 = العربية
// 8 = Türkçe

#define GENERAL_SETTINGS_GROTE_NEDERLANDSE_LABELS                       true
#define GENERAL_SETTINGS_GROTE_STATUSREGELS                             true

#define GENERAL_SETTINGS_DIRECT_SCHAKELEN                               false
#define GENERAL_SETTINGS_TOON_STATUSBALK                                true
#define GENERAL_SETTINGS_TOON_DETAIL_PAGINA                             true
#define GENERAL_SETTINGS_TOON_SYSTEEM_PAGINA                            true
#define GENERAL_SETTINGS_TOON_TOUCH_FEEDBACK                            true
#define GENERAL_SETTINGS_TOON_BEVESTIGING_POPUP                         true
#define GENERAL_SETTINGS_TOON_FASE_DETAILS                              true

#define GENERAL_SETTINGS_IF_OVER_1000_WATTS_REPORT_KW                   true
#define GENERAL_SETTINGS_NUMBER_DECIMAL_PLACES_FOR_KW_REPORTING         1

#define GENERAL_SETTINGS_ADDITIONAL_INFO                                2
// 1 = time-to-go
// 2 = laadstatus
// 4 = accuvermogen

#define GENERAL_SETTINGS_USE_DAYLIGHT_SAVING                            true
// Default timezone: Europe/Amsterdam. Replace these strings if you want a different timezone.
#define GENERAL_SETTINGS_TIMEZONE_WITH_DST                              "CET-1CEST,M3.5.0/2,M10.5.0/3"  // Europe/Amsterdam (DST on)
#define GENERAL_SETTINGS_TIMEZONE_WITHOUT_DST                           "CET-1"                          // Europe/Amsterdam (DST off)
#if GENERAL_SETTINGS_USE_DAYLIGHT_SAVING
#define GENERAL_SETTINGS_TIMEZONE                                       GENERAL_SETTINGS_TIMEZONE_WITH_DST
#else
#define GENERAL_SETTINGS_TIMEZONE                                       GENERAL_SETTINGS_TIMEZONE_WITHOUT_DST
#endif
#define GENERAL_SETTINGS_NTP_SERVER_1                                  "nl.pool.ntp.org"
#define GENERAL_SETTINGS_NTP_SERVER_2                                  "europe.pool.ntp.org"
#define GENERAL_SETTINGS_DAG_BRIGHTNESS                                180
#define GENERAL_SETTINGS_NACHT_BRIGHTNESS                              40
#define GENERAL_SETTINGS_DAG_START_UUR                                 7
#define GENERAL_SETTINGS_NACHT_START_UUR                               22

#define GENERAL_SETTINGS_DISPLAY_TIMEOUT_MINUTEN                        1
#define GENERAL_SETTINGS_TURN_ON_DISPLAY_AT_SPECIFIC_TIMES_ONLY         false
#define GENERAL_SETTINGS_PENDING_ACTION_TIMEOUT_MS                      7000
#define GENERAL_SETTINGS_MQTT_DATA_TIMEOUT_SECONDS                      75

#define GENERAL_SETTINGS_GRID_IN_L1_IS_USED                             true
#define GENERAL_SETTINGS_GRID_IN_L2_IS_USED                             true
#define GENERAL_SETTINGS_GRID_IN_L3_IS_USED                             true
#define GENERAL_SETTINGS_PV_IS_USED                                     true
#define GENERAL_SETTINGS_AC_OUT_L1_IS_USED                              true
#define GENERAL_SETTINGS_AC_OUT_L2_IS_USED                              true
#define GENERAL_SETTINGS_AC_OUT_L3_IS_USED                              true

#define GENERAL_SETTINGS_ENABLE_DASHBOARD_UI                         true
#define GENERAL_SETTINGS_ENABLE_FLOW_ANIMATIE                        true
#define GENERAL_SETTINGS_ENABLE_SPARKLINES                           true
#define GENERAL_SETTINGS_ANIMATIE_SNELHEID                           2
// 1 = langzaam
// 2 = normaal
// 3 = snel

#define BOOT_ANIMATION_ENABLED                                       true
#define BOOT_ANIMATION_DURATION_MS                                   2800UL
#define BOOT_ANIMATION_MIN_FRAME_MS                                  26UL
#define BOOT_ANIMATION_HARD_TIMEOUT_MS                               5000UL
#define BOOT_ANIMATION_TITLE                                         "VICTRON ESP"
#define BOOT_ANIMATION_SUBTITLE                                      "Monitoring Console"
#define BOOT_ANIMATION_ACCENT                                        TFT_CYAN
#define BOOT_ANIMATION_SUCCESS                                       TFT_GREEN
#define BOOT_ANIMATION_SOLAR                                         TFT_YELLOW
#define BOOT_ANIMATION_GRID                                          TFT_BLUE
#define BOOT_ANIMATION_BATTERY                                       TFT_GREEN

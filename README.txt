ESP32RemoteForVictron Touch NL Dashboard V4

Deze versie is een opgeschoonde dashboard-build voor de LilyGo T-Display S3 AMOLED touch.

Belangrijkste functies:
- Nederlandse dashboard UI
- Overzicht, Detail en Systeem pagina
- Touch-tegels voor Lader en Omvormer
- Bevestigingspopup voor schakelen
- Touch lock
- Statusbalk voor WiFi/MQTT/tijd/update-leeftijd
- Energie-flow animatie
- Mini-grafieken / sparklines
- Vloeiend geanimeerde waarden
- Fallback laadstatus als solar charger ID onbekend is

V4 fixes t.o.v. V3:
- Verwijderd: oud los codeblok buiten functies dat compile errors gaf
- Gefixt: Solar charger discovery substring was 1 positie fout
- Gefixt: Solar state subscribe werkt nu ook als de solar charger pas na de core subscriptions wordt ontdekt
- Gefixt: solar state subscription guard tegen dubbele subscriptions
- Verbeterd: dashboard settings voor flow/sparklines werken nu conditioneel
- Verbeterd: waarschuwing bij oude MQTT data
- Verbeterd: pagina fallback als detail/systeem in settings uit staat

Installatie:
1. Open ESP32RemoteForVictron_Touch_NL_Dashboard_V4.ino in Arduino IDE.
2. Vul secret_settings.h in.
3. Installeer libraries:
   - LilyGo AMOLED Series
   - TFT_eSPI
   - EspMQTTClient
   - ArduinoJson
4. Board: ESP32S3 Dev Module, PSRAM OPI PSRAM, USB CDC On Boot Enabled.

Let op:
Deze build is statisch en logisch gecontroleerd, maar niet op echte hardware gecompileerd of geflasht in deze omgeving.

Snelle codegids (voor makkelijk terugvinden):
- `ESP32RemoteForVictron_Touch_NL_Dashboard_V4.ino`
  - Bovenin: globale status, app metadata, touchboxen en pagina-state.
  - Utility helpers: formatting, animatiehulpen, kleurenmix, touchbox checks.
  - Data-laag: MQTT discovery + topic subscriptions + parsing van Victron berichten.
  - UI-laag: tekenfuncties voor statusbalk, overzicht/detail/systeem pagina, dialogs en toasts.
  - Input-laag: fysieke knoppen, touch processing, toggle flow voor lader/omvormer.
  - Runtime-laag: setup(), loop(), sampling van history, display refresh en timeout-logica.
- `general_settings.h`
  - Alle functionele toggles en UX/performance instellingen (animatie, touch, timeout, layout).
- `secret_settings.h`
  - WiFi, MQTT en Victron IDs (lokaal invullen, niet committen met echte secrets).
- `boot_animation.h`
  - Opstartanimatie en visuele onboarding.

CLI testen in deze omgeving (zonder Arduino IDE):
1. Installeer Arduino CLI + ESP32 core + libraries:
   - `./scripts/setup_arduino_cli.sh`
2. Compile test draaien:
   - `./scripts/test_compile.sh`

Wat dit doet:
- Installeert `arduino-cli` lokaal in `.arduino/bin` (repo-lokaal, geen systeempakket nodig).
- Zet de ESP32 board index aan en installeert `esp32:esp32`.
- Installeert de gebruikte libraries (TFT_eSPI, EspMQTTClient, ArduinoJson en LilyGo AMOLED Series via GitHub).
- Draait een compile check voor `esp32:esp32:esp32s3`.

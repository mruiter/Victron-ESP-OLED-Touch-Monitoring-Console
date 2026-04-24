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

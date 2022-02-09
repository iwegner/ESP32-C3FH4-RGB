# ESP32-C3FH4-RGB 

driver


https://zadig.akeo.ie/




Setup connection to connect to Board ESP32C3 Dev and com port which is assocuiated to USB Serial Device, not jtag (See Device Manager on Win10). Other settings see settings.png.

Upload sketch from Arduino IDE:
Hold down B, press R and release again but keep on pressing B, trigger Arduino IDE to upload sketch,  until Arduino IDE says that its connected, then you can release B

Serial works for me with "CDC enable on boot" set to true

NEO_GRB shows right colors, NEO_GRBW doesn't work as WS2812b don't have white sub leds.


Specs from BG (German):
Genaue Details:
Was ist C3FH4 RGB?

C3FH4 RGB verfügt über ein ultrakompaktes Design mit einem Risc-v 32-Bit-Single-Core-Prozessor bei 160 MHz auf einer Platine, integriertem 400 KB SRAM und 4 MB Flash , integriert mit 2,4-GHz-WLAN und unterstützt Bluetooth 5 ( LE) mit branchenführender HF-Leistung und geringem Stromverbrauch. Durch die Kombination von RSA-3072-basiertem Secure Boot und der AES-128-XTS-basierten Flash-Verschlüsselung bietet es eine sicherere Möglichkeit, Bluetooth-Sicherheitsbedenken zu beantworten.
Mit geringem Stromverbrauch: Es ist ideal für alle platzbeschränkten oder batteriebetriebenen Geräte wie Wearables, medizinische Geräte, Sensoren und andere IoT-Geräte. und vollständig kompatibel mit Arduin, MicroPython, ESP32-IDF und anderen gängigen Entwicklungsplattformen schnell verschiedene Anwendungen erstellen.
HOHE INTEGRATION: C3FH4 RGB Einschließlich 2 Kanäle von 5V -> 3,3 V DC / DC, GPIOx11, programmierbares RGB-Licht x25, Taste x2, fein abgestimmte HF-Schaltung, die eine stabile und zuverlässige drahtlose Kommunikation bietet.

Merkmale:

ESP32-C3FH4 (2,4 GHz Wi-Fi und unterstützt Bluetooth 5 (LE) über lange Distanzen)
eingebauter 400 KB SRAM und 4 MB Flash
USB Type-C (USB unterwegs (OTG))
Inklusive 2 Kanäle 5V - > 3.3V DC / DC
Integriertes 5X5 ws2812b-1515 (GPIO8) und 2 Tasten (GPIO9 und CHIP_EN)
3V Stromversorgung LED und Status LED(GPIO10)
Keramikantenne

Größe Gewicht:
Größe: 21 x 18 mm
Gewicht: 2,3 g

UNTERLAGEN:
https://github.com/01Space/ESP32-C3FH4-RGB

Paket beinhaltet:

1 x LED-Platine
(Hinweis: Die Münze ist nicht im Lieferumfang enthalten)

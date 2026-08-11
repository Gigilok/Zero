# ESP32-DIV — adaptação para WROOM 30 pinos + OLED 0.96"

Este projeto é uma adaptação do [ESP32-DIV](https://github.com/cifertech/ESP32-DIV.git) (v1.7.2) para rodar em uma **placa ESP32 WROOM 30 pinos** com:

- **Display OLED 0.96" SSD1306** (128×64, I2C) — com **fundo branco** para destacar os desenhos
- **4 botões GPIO** (sem PCF8574, sem touchscreen)
- **Memória flash interna** (LittleFS) no lugar do cartão SD
- **nRF24L01** e **CC1101** compartilhando o barramento SPI
- **Buzzer** e **medidor de bateria** nos pinos especificados

> Nenhum recurso do projeto original foi removido. Todos os módulos (WiFi, BLE, SubGHz, RFID, GPS, IR, Ducky, etc.) permanecem presentes e utilizam as camadas de compatibilidade abaixo para funcionar no novo hardware.

---

## 1. Pinagem

| Função | Pino | Cor do fio |
|---|---|---|
| **OLED SDA** | GPIO 21 | — |
| **OLED SCL** | GPIO 22 | — |
| **OLED VDD** | 3.3 V | — |
| **OLED GND** | GND | — |
| **Botão UP** | GPIO 5 | cinza |
| **Botão DOWN** | GPIO 27 | vermelho |
| **Botão SELECT** | GPIO 32 | marrom |
| **Botão 4 (LEFT)** | GPIO 33 | preto |
| **Botões GND** | GND | — |
| **nRF24 CE** | GPIO 26 | amarelo |
| **nRF24 CSN** | GPIO 25 | azul |
| **nRF24 SCK** | GPIO 18 | verde |
| **nRF24 MOSI** | GPIO 23 | roxo |
| **nRF24 MISO** | GPIO 19 | cinza |
| **nRF24 IRQ** | sem conexão | — |
| **CC1101 GND** | GND | cinza |
| **CC1101 3.3V** | 3.3 V | branco |
| **CC1101 GDO0** | GPIO 12 | azul |
| **CC1101 CSN** | GPIO 14 | roxo |
| **CC1101 SCK** | GPIO 18 | amarelo |
| **CC1101 MOSI** | GPIO 23 | verde |
| **CC1101 MISO** | GPIO 19 | vermelho |
| **CC1101 GDO2** | GPIO 13 | laranja |
| **Buzzer** | GPIO 4 | — |
| **Medidor de bateria** | GPIO 34 | — |

> Os botões devem ser ligados entre o pino GPIO e o GND (o shim ativa o pull-up interno; leitura ativa-baixa).

---

## 2. Arquivos adicionados (camadas de compatibilidade)

| Arquivo | Substitui | Função |
|---|---|---|
| `DisplayAdapter.h` / `.cpp` | `<TFT_eSPI.h>` | Shim que mapeia a API do TFT_eSPI (240×320, 16 bits) para o SSD1306 (128×64, 1 bit) com **fundo branco** e escalonamento automático das coordenadas. |
| `SDFlashAdapter.h` / `.cpp` | `<SD.h>` | Shim que mapeia `SD.begin/open/remove/...` para LittleFS, salvando tudo na memória flash do ESP32. |
| `PCF8574Adapter.h` / `.cpp` | `<PCF8574.h>` | Shim que mapeia `pcf.digitalRead(BTN_x)` para leitura direta dos GPIOs 5/27/32/33. |
| `XPT2046Adapter.h` | `<XPT2046_Touchscreen.h>` | Stub que sempre retorna "sem toque" — a OLED não tem touchscreen. |

Os arquivos originais do ESP32-DIV (`utils.cpp`, `wifi.cpp`, `bluetooth.cpp`, `subghz.cpp`, `rfid.cpp`, `gps.cpp`, `ir.cpp`, `ducky.cpp`, `ESP32-DIV.ino`, etc.) foram mantidos praticamente intactos — apenas os `#include` das quatro bibliotecas acima foram redirecionados para os shims, e o `shared.h` ganhou o bloco `BOARD_ESP32_WROOM_OLED` com a pinagem do usuário.

---

## 3. Bibliotecas necessárias (Arduino IDE / PlatformIO)

Instale pela **Library Manager** do Arduino IDE (ou `pio lib install` no PlatformIO):

| Biblioteca | Uso |
|---|---|
| **Adafruit SSD1306** | Driver do OLED (≥ 2.5.0) |
| **Adafruit GFX Library** | Primitivas gráficas usadas pelo DisplayAdapter |
| **LittleFS** (incluída no core ESP32 ≥ 2.0.4) | Sistema de arquivos na flash |
| **RF24** (TMRh20) | Rádio nRF24L01 |
| **SmartRC-CC1101-Driver-Lib** | Módulo CC1101 Sub-GHz |
| **RCSwitch** | Replay/captura Sub-GHz |
| **arduinoFFT** | Análise de espectro Wi-Fi |
| **NimBLE-Arduino** | Stack BLE (substitui o BLE clássico) |
| **Adafruit PN532** | Leitor RFID/NFC (opcional, se for usar) |
| **IRremoteESP8266** | Controle IR |
| **ArduinoJson** (≥ 6.21) | Persistência de configurações |
| **PCF8574_library** | **NÃO é necessária** — substituída pelo PCF8574Adapter |
| **XPT2046_Touchscreen** | **NÃO é necessária** — substituída pelo XPT2046Adapter |
| **TFT_eSPI** | **NÃO é necessária** — substituída pelo DisplayAdapter |

### Placa-alvo no Arduino IDE

- **Board:** "ESP32 Dev Module" (ou "DOIT ESP32 DEVKIT V1")
- **Upload Speed:** 921600
- **CPU Frequency:** 240 MHz
- **Flash Size:** 4 MB (ou o tamanho da sua placa)
- **Partition Scheme:** "Default 4MB with spiffs (1.2MB APP / 1.5MB SPIFFS)" — **importante**: reserve ao menos 1 MB para LittleFS
- **PSRAM:** Disabled (a WROOM 30 pinos não tem PSRAM)

---

## 4. Como o fundo branco funciona

O SSD1306 é um display monocromático (1 bit por pixel). O DisplayAdapter inverte a polaridade das cores para que:

- `tft.fillScreen(TFT_WHITE)` → preenche o buffer com "todos os pixels desligados" → painel mostra **fundo branco**
- `tft.fillScreen(TFT_BLACK)` → preenche o buffer com "todos os pixels ligados" → painel mostra fundo preto (mas **não usamos isso** na inicialização)
- `tft.drawPixel(x, y, TFT_BLACK)` → liga o pixel → ponto **preto** sobre o fundo branco
- `tft.setTextColor(TFT_BLACK, TFT_WHITE)` → texto em **preto** sobre **branco**
- Qualquer cor colorida (vermelho, azul, etc.) é mapeada para **preto**, já que o painel é 1-bit

O escalonamento 240×320 → 128×64 preserva as proporções originais dos desenhos. Itens pequenos (ícones 16×16) podem aparecer reduzidos, mas mantêm o mesmo layout visual.

---

## 5. Compilação

1. Copie a pasta `ESP32-DIV-WROOM-OLED` para `~/Documents/Arduino/` (ou abra diretamente no Arduino IDE).
2. Selecione a placa **ESP32 Dev Module** e a porta COM correta.
3. Selecione o **Partition Scheme** com LittleFS/SPIFFS (1.5 MB mínimo).
4. Clique em **Upload**.
5. Abra o **Serial Monitor** a 115200 baud para acompanhar o boot.

Mensagens esperadas no boot:

```
[boot] start
[boot] I2C ready (SDA=21, SCL=22 @ 400kHz)
[boot] OLED 0.96" ready (white background)
[SDFlashAdapter] LittleFS mounted (flash storage ready)
[sd] LittleFS mounted (flash storage ready)
[PCF8574Adapter] buttons configured on GPIO 5/27/32/33
[boot] ready
```

---

## 6. Uso

- **Botão UP (cinza, GPIO 5)** — navega para cima
- **Botão DOWN (vermelho, GPIO 27)** — navega para baixo
- **Botão SELECT (marrom, GPIO 32)** — confirma/seleciona/volta
- **Botão 4 (preto, GPIO 33)** — funciona como **LEFT** no menu (quando aplicável)

A navegação pelo **touch** não funciona (não há painel tátil). Use somente os botões físicos.

Os arquivos salvos (logs de wardriving, capturas SubGHz, perfis RFID, etc.) são gravados em **LittleFS** sob os diretórios `/logs`, `/captures`, `/config` — criados automaticamente no primeiro boot.

Para acessar esses arquivos a partir do PC, use a ferramenta **ESP32 LittleFS Data Upload** (plugin do Arduino IDE) ou o comando `esptool.py` para extrair a partição.

---

## 7. Limitações conhecidas

1. **Texto pequeno**: o escalonamento 240×320 → 128×64 reduz muito o texto da fonte 1 do TFT_eSPI. A Adafruit_GFX default font (6×8 px em size 1) é usada no lugar; algumas telas com muito texto podem ficar apertadas. Ajuste os tamanhos de fonte nos arquivos `.cpp` se precisar de mais legibilidade.

2. **Cores perdidas**: o display é 1-bit, então todos os realces coloridos do tema Dark/Light são convertidos para preto-e-branco. A legibilidade é preservada pelo uso de fundo branco + tinta preta.

3. **Sem touchscreen**: funcionalidades que dependem exclusivamente de toque (calibração de tela, alguns gestos) não funcionam. A navegação por botões cobre todos os menus principais.

4. **Buzzer**: ativo no GPIO 4. Alguns alertas sonoros do projeto original devem funcionar como esperado.

5. **RFID PN532**: o projeto mantém o suporte ao PN532 via SPI. Caso não use, ele apenas não detectará o módulo — sem impacto nos demais recursos.

6. **BLE Ducky**: desativado automaticamente nesta placa (`FEATURE_BLE_DUCKY = BOARD_HAS_ESP32S3 = 0`). O stack NimBLE ainda é usado para os demais recursos BLE (jammer, spoofer, scanner, etc.).

---

## 8. Solução de problemas

### "SSD1306 allocation failed"
Verifique se a biblioteca **Adafruit SSD1306** está instalada e se os fios SDA/SCL não estão invertidos.

### Tela totalmente preta ou branca
- Confira o endereço I2C do OLED. A maioria é **0x3C**, mas alguns módulos usam **0x3D**. Ajuste `OLED_I2C_ADDR` em `DisplayAdapter.h` se necessário.
- Verifique se SDA=21 e SCL=22 (e não o contrário).

### Botões não respondem
- Cada botão deve ir do **GPIO ao GND** (sem resistor externo — o pull-up é interno).
- Confirme os pinos: 5 (cinza), 27 (vermelho), 32 (marrom), 33 (preto).

### nRF24 ou CC1101 não funcionam
- Ambos compartilham SCK=18, MOSI=23, MISO=19. Os CS são separados (CC1101=14, nRF24=25).
- Alimente os módulos com **3.3V** (não 5V).
- Para o nRF24, deixe o pino IRQ desconectado.

### LittleFS não monta
- No Arduino IDE, selecione **Partition Scheme** com pelo menos 1 MB para SPIFFS/LittleFS (ex.: "Default 4MB with spiffs").
- A primeira montagem formatará a partição — pode levar alguns segundos.

---

## 9. Créditos

- Projeto original: **cifertech / ESP32-DIV** — https://github.com/cifertech/ESP32-DIV
- Versão: v1.7.2
- Adaptação para ESP32 WROOM + OLED 0.96" com fundo branco: este repositório.

Licença original do ESP32-DIV mantida.

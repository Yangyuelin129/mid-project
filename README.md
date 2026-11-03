# ⏱️ STM32G474RE 15-Second LED Progress & Blink Timer

![STM32G474RE](https://img.shields.io/badge/MCU-STM32G474RE-003555?style=flat-square&logo=stmicroelectronics)
![HAL Library](https://img.shields.io/badge/Library-STM32__HAL-blue?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-green.svg?style=flat-square)

本專案基於 **STM32G474RE** 開發板實作一個 15 秒漸進式 LED 計時器。系統採用非阻塞式（Non-blocking）時間戳與狀態機設計，透過單一按鈕（`PC9`）達成計時的**暫停、恢復與結束重置**，並以 3 顆 LED（`PC5`, `PC8`, `PC6`）呈現 5 秒區間累加點亮與結束閃爍提示。

---

## 📌 功能與狀態轉移 (System Features & State Machine)

系統包含三種運作狀態：`RUNNING`（計時中）、`PAUSED`（暫停中）與 `BLINKING`（閃爍結束）。

### 1. 燈號狀態顯示邏輯
- **0 ~ 5 秒 (`0 ~ 5000ms`)**：亮 1 顆 LED (`PC5`)
- **5 ~ 10 秒 (`5000 ~ 10000ms`)**：亮 2 顆 LED (`PC5`, `PC8`)
- **10 ~ 15 秒 (`10000 ~ 15000ms`)**：亮 3 顆 LED (`PC5`, `PC8`, `PC6`)
- **到達 15 秒 (計時結束)**：進入 `BLINKING` 狀態，3 顆 LED 以 300ms 為週期同步閃爍提示。

### 2. 按鈕控制邏輯 (`PC9` 上拉輸入，按下為 Low)
- **計時中 (`RUNNING`)**：按下按鈕即**暫停計時**，進入 `PAUSED` 狀態（若按下時已達 15 秒則直接進入 `BLINKING`）。
- **暫停中 (`PAUSED`)**：按下按鈕補算時間並**繼續計時**，回歸 `RUNNING` 狀態。
- **閃爍提示中 (`BLINKING`)**：按下按鈕**重置計時器**，時間歸零並重新回到 `RUNNING` 狀態。

---

## 🛠️ 硬體腳位與時間參數 (Hardware & Software Config)

### GPIO 配置

| 定義名稱 | 板載 GPIO 腳位 | 腳位模式 | 說明 |
| :--- | :--- | :--- | :--- |
| `LED1_PIN` | `PC5` | GPIO Output (Push-Pull) | 第一階段進度燈 (0~5s 點亮) |
| `LED2_PIN` | `PC8` | GPIO Output (Push-Pull) | 第二階段進度燈 (5~10s 點亮) |
| `LED3_PIN` | `PC6` | GPIO Output (Push-Pull) | 第三階段進度燈 (10~15s 點亮) |
| `BTN_PIN` | `PC9` | GPIO Input (`GPIO_PULLUP`) | 按鈕控制 (按下觸發 Low) |

### 時間巨集設定

```c
#define DEBOUNCE_TIME 50    // 按鈕去彈跳時間 (50ms)
#define BLINK_TIME    300   // 結束閃爍切換週期 (300ms)
#define STAGE_TIME    5000  // 每階段區間時間 (5000ms)
#define TOTAL_TIME    15000 // 總計時時間 (15000ms)# mid project


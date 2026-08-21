# AIR - RoboCupJunior2026 Robot Datas

RoboCupJunior Soccer Lightweight ロボット公開リポジトリ
RoboCupJunior Soccer Lightweight Robot Repository

---

# 概要 / Overview

本リポジトリには、**AIR** が RoboCupJunior Soccer Lightweight 2026 シーズン（Japan Open & World Championship in Incheon）で使用したロボットの機械設計・回路設計・組込みソフトウェアなどの開発データを公開しています。

This repository contains the hardware and software design files used by **AIR** in the RoboCupJunior Soccer Lightweight 2026 season.

---

# 公開内容 / Contents

* **3DCADデータ** / 3D CAD files
* **回路基板データ** / PCB design files
* **組込みソフトウェア** / Embedded software
* **プレゼンポスター** / Presentation poster

---

# 大会実績 / Competition Result

* **RoboCupJunior Japan Open 2026**
  * **競技 1位** / 1st Place in Japan Open

* **RoboCup World Championship 2026 (Incheon, Republic of Korea)**
  * **競技 優勝** / Individual World Champion (1st Place)
  * **総合 優勝** / Overall World Champion (1st Place)
  * **コミュニティアワード** / Community Award

---

# ロボットの特徴 / Features

* **カスタム機械設計** / Custom mechanical design
* **自作多層PCB** / Custom-designed multi-layer PCBs
* **全方向移動機構** / Omnidirectional drive system
* **高速制御システム & マルチマイコン分散処理** / High-speed embedded control & Distributed multi-MCU architecture
* **モジュール化されたソフトウェア構成** / Modular software architecture

---

# リポジトリ構成 / Repository Structure

```text
.
├── CAD/            # 3DCADデータ / CAD files
├── PCB/            # 回路設計データ / PCB files
├── Software/       # ソースコード / Source code
├── Poster/         # プレゼンポスター / Presentation poster
└── Docs/           # ドキュメント / Documents
```

# ハードウェア / Hardware

ジャパンオープン（Japan Open）および世界大会（World Championship）における全ハードウェア構成です。

Complete hardware specifications for both the Japan Open and World Championship models.

## Japan Open モデル構成 / Japan Open Specification

| 部品 / Component | 内容・型番 / Description & Part Number | 詳細 / Details |
| --- | --- | --- |
| Main MCU | Teensy 4.1 (DigiKey) | 600MHz動作、8×UARTポート（内7ポートをサブマイコン等に使用） |
| Ball Sub MCU | ATmega32U4 (JLCPCB SMT) | ICSPブートローダー書き込み、USB Type-C書き込み対応 |
| Line Sub MCU | ATmega2560 (JLCPCB SMT) | ピン数確保、USB Type-C (CH340E) 書き込み |
| Wall/US Sub MCU | Seeed Studio XIAO ESP32-S3 (秋月電子) | FreeRTOS デュアルコア制御による非ブロッキング超音波計測 |
| UI Sub MCU | ESP32 WROOM-32E (秋月電子) | 自動書き込み回路 (CH340K/Type-C)、Bluetoothデバッグ、Type-C VBUSによる挿入検知LED |
| Motor Driver | DRV8432 (JLCPCB SMT) | 1ICで2モータ駆動、モジュール化基板 |
| Kicker Solenoid | CB1037 (タカハ機工様 提供品) | 低抵抗・高出力ソレノイド |
| Kicker MOSFET | Pch (60V15A) / Nch (60V100A) (秋月電子) | ハイサイド充電制御 & Nch大電流ローサイドキック駆動 |
| Kicker Photocoupler | AQW212EHA (DigiKey) | メインマイコンとキッカー回路の光学絶縁 |
| Kicker Boost | XL6009 昇圧モジュール (Amazon) | MAX50V耐圧改造・ルール限界値へ昇圧調整 |
| IMU | BNO055 (秋月電子) | I2C接続、適切なプルアップ抵抗・配線長短縮対策 |
| Level Shifter | BSS138 自作モジュール (秋月電子) | 5V ↔ 3.3V ロジックレベル変換 (ラインマイコン間) |
| Ball Sensor | TSSP4038 (DigiKey) | 赤外線検知 |
| Line Sensor | B19H1LS フォトトランジスタ (DigiKey) | 表面実装、はんだ付け性・波長マッチング向上 |
| Line Comparator | LM393 (DigiKey) | アナログ→デジタル変換、PWM入力＋RCフィルターによる動的閾値調整 |
| Ultrasonic Sensor | RCW-0001 小型超音波センサー (Amazon) ×2 | 壁面距離計測 |
| Power System | LM2576 5V DCDCコンバータ (DigiKey) | 40V高耐圧サージ対策、物理波動スイッチ＋MOSFET駆動回路 |
| UI & Indicator | OLED SSD1306 / NeoPixel WS2812B (秋月電子) | ボール・ライン・ゴール角度表示用UI |

## World Championship モデル構成 / World Championship Specification

| 部品 / Component | 内容・型番 / Description & Part Number | 詳細 / Details |
| --- | --- | --- |
| Main MCU | Teensy 4.1 | 安定動作・メイン処理継続 |
| Main Board Sub MCU | Seeed Studio XIAO ESP32-S3 ×2 | 開発期間短縮のためのマルチボード構成、FreeRTOS処理 |
| Ball Sub MCU | RP2350A (USB Type-C) | 高速処理・最適ピンアサイン |
| Line Sub MCU | RP2350B (USB Type-C) | 処理速度大幅改善、豊富でジャストサイズなピン数 |
| UI / US Sub MCU | Seeed Studio XIAO ESP32-S3 | OLED / スイッチ / NeoPixel 制御、メイン基板へ直付け |
| Motor Driver | DRV8432 | 継続採用 |
| Dribbler (Option) | 超小型 BLDC モーター + 専用 ESC | アリエクスプレス調達、ドリブラー試験搭載 |
| Kicker Solenoid | CB1037 (タカハ機工様 提供品) | AQW212 フォトカプラ絶縁駆動継続 |
| Kicker Boost | XL6009 昇圧モジュール | 昇圧回路をキッカー基板へ直挿し |
| IMU | BNO055 | 継続採用 |
| Ball Sensor | TSSP58038 ×24個 | RP2350Aによる24個直列読み取り |
| Line Sensor | B19H1LS フォトトランジスタ + LM393 | エンジェル32個 + サイドライン4個×2（ワイヤードOR回路統合） |
| Line Dimming | MOSFET 調光機能 | LED光量可変調整回路 |
| Power System | AP64500SP DCDCコンバータ (DigiKey) | 3.3V固定出力 & 7.0V可変出力（メイン基板で5V/3.3V降圧） |
| Power Switch | トグル型ロッカースイッチ | 耐久性向上・大会中故障ゼロ |
| Voltage Measurement | 抵抗分圧回路 | 電源電圧リアルタイム取得機能（ハードウェア実装済み） |
| UI & System | メイン基板統合型UI (OLED / NeoPixel) | 直径15cm超大型メイン基板、世界大会用QRコードWeb/アプリリモートコントロールモジュール対応 |

---

# ソフトウェア / Software

主に C/C++ および Python を用いて開発されています。

Developed primarily using C/C++ and Python.

## 主な機能 / Main Features

* **ボール追跡・位置算出** / Ball tracking and position estimation
* **白線検知・コート内維持** / White line detection and field boundary control
* **超音波・FreeRTOS並行処理による壁面非ブロッキング距離計測** / Non-blocking wall distance measurement using FreeRTOS dual-core
* **PIDモータフィードバック制御** / PID-based motor feedback control
* **状態遷移ベース戦略・ルール適応** / State-machine strategy and rule adaptation system
* **マルチマイコン間UARTメッシュ通信 & センサ統合** / Multi-MCU UART communication & sensor fusion
* **世界大会用コントロールモジュール適応** / World Championship referee module integration via QR code web/app interface

---

# 使用ソフトウェア・設備 / Software & Equipment

## ソフトウェア / Software

| 用途 / Purpose | ソフトウェア / Software |
| --- | --- |
| 3DCAD | Autodesk Fusion |
| EDA (PCB Design) | KiCad |
| Programming | Visual Studio Code / Arduino IDE / MaixPy IDE |
| Poster & Presentation | Canva |

## 設備 / Equipment

| 用途 / Purpose | 設備 / Equipment |
| --- | --- |
| 3D Printer | Bambu Lab A1 mini / Bambu Lab P1S / Flashforge Adventurer 5M |
| PCB Fabrication | JLCPCB (2層・4層基板 / SMTアッセンブリ) |
| Machining / Tools | CNC Machine |

---

# CAD

ロボットの機械部品は3DCAD（Autodesk Fusion）で設計されています。

Mechanical parts of the robot were designed using 3D CAD (Autodesk Fusion).

## 含まれるデータ（一部） / Included Designs (Partial)

* シャーシ / Chassis
* キッカーユニット / Kicker unit
* センサマウント / Sensor mounts
* オムニホイールユニット / Omni wheel modules

---

# PCB

ロボットで使用した自作PCB（KiCad）の設計データも公開しています。全基板はJLCPCBにて製造されています。

Custom PCBs created with KiCad are included. All boards were fabricated by JLCPCB.

## 含まれる基板（一部） / Included Boards (Partial)

* **メイン基板 (`ball unit` / メインボード)** / Main control board
* **ボールセンサー基板** / Ball sensor board
* **ラインセンサー基板 (`linesensor unit`)** / Line sensor board
* **電源基板 (`power unit`)** / Power distribution board
* **MD & 昇圧統合基板** / Motor driver & boost converter board
* **キッカー基板** / Kicker board
* **UI基板** / UI & Debug board

---

# 開発理念 / Development Philosophy

RCJコミュニティや学生ロボティクス開発への貢献を目的として、本リポジトリを公開しています。

We believe open-source robotics accelerates learning and innovation.

このリポジトリが、
* 新規チームの参考になること
* 実践的なロボット工学学習につながること
* RCJコミュニティ全体の発展につながること

を願っています。

We hope this repository helps:
* New RCJ teams get started
* Share practical engineering knowledge
* Improve the RCJ ecosystem

---

# スポンサー / Sponsors

本プロジェクトは、以下の企業・団体の支援を受けています。

This project is supported by the following companies and organizations.

* **JLCPCB**  
  https://jlcpcb.com/  
  （日本語発注システム: https://jlcpcb.com/jp/ ）
* **DigiKey**  
  https://www.digikey.jp/
* **株式会社人機一体**  
  https://www.jinki.jp/
* **株式会社システムアイインターナショナル**  
  http://www.system-i-int.co.jp/
* **maxon**  
  https://maxonjapan.com/
* **タカハ機工株式会社**  
  https://www.takaha.co.jp/
* **ライフ&キャリアコンサルティング LACIQUE**  
  https://lacique.jp/
* **医療法人社団 翠会 水口病院**  
* **JOJISTABLE**  
* **立命館 (Ritsumeikan)**  

---

# チーム情報 / Team

## AIR

RoboCupJunior Soccer Lightweight Team from Japan.

* **X (Twitter)**: https://x.com/Air_3838
* **YouTube**: https://www.youtube.com/@AIR-RCJ
* **note**: https://note.com/air_rcj

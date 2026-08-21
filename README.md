# AIR - RoboCupJunior2026 World Champion Robot Data Repository

RoboCupJunior Soccer Lightweight 2026 シーズンにおいて、ジャパンオープン優勝および世界大会（韓国・仁川）にて**競技優勝・総合優勝・コミュニティ賞の3冠**を達成したロボット「AIR」の完全設計データ公開リポジトリです。

This repository contains the full design, hardware, and software data of **AIR**, the 1st Place Champion in both Japan Open and World Championship (Individual & Overall Champion) in the RoboCupJunior Soccer Lightweight 2026 season.

---

# 概要 / Overview

本リポジトリは、次世代のRCJコミュニティおよび学生ロボティクスエンジニアへの情報共有を目的として公開されています。
2026シーズン内開発で積み上げた回路設計、組み込みソフトウェア（マルチマイコン・FreeRTOS分散処理）、3D CADデータを網羅しています。

We open-source our complete robot design to support the global RCJ community and future student roboticists. It covers custom PCB design, high-speed multi-MCU embedded firmware, and precision 3D CAD models.

---

# 大会実績 / Competition Results

* **RoboCupJunior Japan Open 2026**
  
  * **競技 優勝** / 1st Place in Japan Open
   
* **RoboCup World Championship 2026 (Incheon, Republic of Korea)**
  
  * **競技 優勝** / Individual World Champion (1st Place)
    
  * **総合 優勝** / Overall World Champion (1st Place)
    
  * **コミュニティ アワード** / Community Award

---

# 技術的ハイライト / Technical Highlights

AIRのシステムは、超高速処理と徹底された堅牢性（耐ノイズ・電源安定性）を軸に設計されています。

* **RP2350 & Teensy 4.1 による超高速分散処理**
  メインマイコン(Teensy 4.1 600MHz)の8ch UARTを活用し、サブマイコン群（RP2350A/B, XIAO ESP32-S3）へ処理を分散。
 
* **24基のTSSP58038による高密度ボール検出**  
  RP2350Aを用いて24個の赤外線センサーを直列・高速サンプリングし、死角のない高精度なボール位置・距離の推定を実現。
  
* **FreeRTOSを用いた非ブロッキング超音波壁面計測**  
  `pulseIn` 関数によるブロッキング問題を解消するため、ESP32-S3のデュアルコアとFreeRTOSを活用してバックグラウンドで高周波壁面距離を計測。
  
* **信頼性を極めた電源・回路設計**  
  DCDCコンバータ(AP64500SP)による3.3V/7.0V分離給電、ハイ/ローサイドMOSFET（Nch 60V100A）を用いた大電流キッカー駆動、および全マイコンの自動書き込み・デバッグ機能の一体化。
  
* **世界大会用レフェリーモジュール適応**  
  QRコードを介したWeb/アプリリモートコントロールシステムとの連携機能を搭載。

---

# リポジトリ構成 / Repository Structure

```text
.
├── CAD/            # 3DCADデータ (Autodesk Fusion)
├── PCB/            # 回路設計データ (KiCad) / JLCPCB製造データ
├── Software/       # 組み込みソースコード (C/C++, Python)
├── Poster/         # プレゼンポスター & 世界大会インタビュースライド
└── Docs/           # 回路図補足・システム構成ドキュメント
```

# ハードウェア仕様とシステム進化 / Hardware Architecture & Evolution

AIRのハードウェア構成は、ジャパンオープンから世界大会（Incheon）にかけて大幅な進化を遂げています。

| ユニット / System | Japan Open モデル | World Championship (Incheon) モデル |
| --- | --- | --- |
| **Main MCU** | Teensy 4.1 (600MHz / 8×UART) | Teensy 4.1 (安定動作・メイン制御継続) |
| **Ball Sub MCU & Sensors** | ATmega32U4 + TSSP4038 | **RP2350A + TSSP58038 × 24個** (高密度24方位直列読取) |
| **Line Sub MCU & Sensors** | ATmega2560 + B19H1LS / LM393 | **RP2350B + B19H1LS / LM393** (エンジェル32+サイド8) + MOSFET調光 |
| **Wall/US MCU** | XIAO ESP32-S3 (FreeRTOS超音波制御) | XIAO ESP32-S3 (メイン基板直付け統合) |
| **UI & Remote Control** | ESP32 WROOM-32E + OLED + NeoPixel | **XIAO ESP32-S3 + OLED** + 世界大会公式リモートモジュール対応 |
| **Motor Driver** | DRV8432 (2chモジュール化基板) | DRV8432 (継続採用) + 超小型BLDCドリブラー |
| **Kicker System** | CB1037 + Pch/Nch (60V100A) MOSFET | CB1037 + AQW212光絶縁 + 直挿しXL6009昇圧 |
| **Power Distribution** | LM2576 (5V) + 波動スイッチ | **AP64500SP** (3.3V固定 & 7.0V可変) + トグルロッカースイッチ |

---

# ソフトウェア機能 / Software Architecture

主に C/C++ および Python を用いて開発された、マルチマイコン分散処理による高速制御システムです。

* **アルゴリズム**
  * **Ball Tracking**: 24個のTSSP58038から得られるデジタル&パルスの入力パターンをRP2350Aで高速処理し、死角のない最確方位と距離を即座に算出。新ボールにも対応した、円滑で確実な回り込み軌道を実現。
    
  * **Line Keep**: 40個の表面実装フォトトランジスタ（B19H1LS）とLM393コンパレータを採用。PWM入力＋RCフィルターによる動的閾値調整とワイヤードOR回路の併用で、ライン進入時の超高速レスポンスを実現。
    
  * **Wall Distance**: ESP32-S3のFreeRTOS（デュアルコアタスク）を活用し、`pulseIn` 関数によるマイコンブロッキングを排除。バックグラウンドで壁面距離を高周波サンプリング。
    
  * **Motion Control**: IMU (BNO055) フィードバックを伴うPID全方向移動制御。
    
  * **Strategy State Machine**: 大会中のルール変更や試合状況に即座に適応する柔軟な状態遷移制御。
    
  * **World Championship Remote Control**: 世界大会公式の審判用リモートモジュール（QRコード連携Web/アプリインターフェース）との完全統合。

---

# 開発環境・使用設備 / Development Environment & Equipment

### Software Tools
* **3D CAD**: Autodesk Fusion360
  
* **EDA (PCB Design)**: KiCad
  
* **IDE / Programming**: Visual Studio Code / MaixPy IDE
  
* **Presentation & Media**: Canva

### Hardware & Manufacturing Equipment
* **3D Printers**: Bambu Lab A1 mini / Bambu Lab P1S / Flashforge Adventurer 5M
  
* **PCB Fabrication**: JLCPCB (2層・4層基板 / SMT基板実装アッセンブリサービス活用)
  
* **Machining / Tools**: 小型CNC加工機

---

# CAD設計データ / CAD Models

ロボットを構成するすべてのカスタム機械部品は Autodesk Fusion360 で設計されています。

* **シャーシ構造** / Chassis
  
* **キッカーユニット** / Kicker unit
  
* **センサマウント各種** / Sensor mounts
  
* **オムニホイールモジュール** / Omni wheel modules
  
* **超小型BLDCドリブラー機構** / Micro BLDC Dribbler module

---

# 公開基板データ / PCB Designs

ロボットに使用されている自作基板（KiCad）の全設計ファイルです。すべての基板は JLCPCB 様の製造・SMTアッセンブリ支援を受けて制作されています。

**日本大会機**

1. **メイン基板 (`main_unit`)**: 各種マイコン・IMU・超音波センサー統合基板

2. **ボールセンサー基板 (`ball_unit`)**: TSSP4038×16 ＆ ATmega32U4 搭載基板

3. **ラインセンサー基板 (`line_unit`)**: 特殊クワガタ形状・B19H1LS/LM393/ATmega2560 搭載基板

4. **電源基板 (`power_unit`)**: LM2576採用 電源基板

5. **モータドライバ基板 (`md_unit`)**: DRV8432 & 昇圧 統合モジュール基板

6. **キッカー制御基板 (`kicker_unit`)**: 大電流MOSFET・AQW212光絶縁回路


**世界大会機**

1. **メイン基板 (`main_unit`)**: 各種マイコン・IMU・UI統合型 直径15cm大型基板

2. **ボールセンサー基板 (`ball_unit`)**: TSSP58038×24 ＆ RP2350A 搭載高密度基板

3. **ラインセンサー基板 (`line_unit`)**: サイドバー式採用 B19H1LS/LM393/RP2350B 搭載基板

4. **電源基板 (`power_unit`)**: AP64500SP採用 高効率・高耐圧電源基板

5. **モータドライバ基板 (`md_unit`)**: DRV8432 2chモジュール基板

6. **キッカー制御基板 (`kicker_unit`)**: 昇圧＆大電流MOSFET・AQW212光絶縁回路

---

# スポンサー / Sponsors

本プロジェクトの挑戦と世界大会優勝は、以下の素晴らしいスポンサー企業・団体様のご支援により実現いたしました。心より感謝申し上げます。

* **JLCPCB** (高品質基板製造およびSMTアッセンブリ支援)  
  https://jlcpcb.com/ (日本語発注サイト: https://jlcpcb.com/jp/ )
  
* **DigiKey**  
  https://www.digikey.jp/
  
* **株式会社人機一体**  
  https://www.jinki.jp/
  
* **MAXON**  
  https://maxonjapan.com/
  
* **タカハ機工株式会社** (CB1037ソレノイドご提供)  
  https://www.takaha.co.jp/
  
* **株式会社システムアイインターナショナル**  
  https://www.systemi.com/
  
* **ライフ&キャリアコンサルティング LACIQUE**  
  hhttps://www.lacique.com/
  
* **医療法人社団 翠会 水口病院**  
  https://www.minakuchi-hp.or.jp/

* **JOJISTABLE**
  https://x.com/JojiStable

* **立命館守山高校&大学 (Ritsumeikan)**
  https://www.mrc.ritsumei.ac.jp/
  https://www.ritsumei.ac.jp/

---

# チーム情報・お問い合わせ / Team Information & Contact

## AIR

RoboCupJunior Soccer LightWeight&Infrared Team from Japan  
World Champion 2026

### チーム実績 / Team History
* **2025 シーズン**
  * RoboCupJunior Japan Open 2025 🥉 **3位**
  
  * RoboCup World Championship 2025 (Salvador, Brazil) 🥉 **Individual 3rd Place**
  
* **2026 シーズン**
  * RoboCupJunior Japan Open 2026 🥇 **優勝 (1位)**
  
  * RoboCup World Championship 2026 (Incheon, Republic of Korea) 🏆 **Individual 1st Place** / 🏅 **Community Award**

---

### お問い合わせ・ご質問 / Contact Information

本リポジトリの内容や設計データ、その他技術的なご質問やお問い合わせがありましたら、以下までお気軽にご連絡ください！

* **X (Twitter) DM**: [@Air_Gamu_rcj](https://x.com/Air_Gamu_rcj)

* **Email**: [airkorea2026@gmail.com](mailto:airkorea2026@gmail.com)

### 公式メディア・SNS / Links
* **X (Twitter)**: [@Air_3838](https://x.com/Air_3838)

* **YouTube**: [AIR-RCJ Channel](https://www.youtube.com/@AIR-RCJ)

* **note**: [AIR Official note](https://note.com/air_rcj)

# 動作確認・利用ガイド (Usage & Testing Guide)

本ドキュメントでは、ESP32 WOL / リモートサーボ駆動ファームウェアの設定方法、ビルド・書き込み手順、テスト用WebSocketサーバーの立ち上げ手順、および実機疎通確認手順について解説します。

---

## 1. 開発環境の準備

### 1.1 必要なツール
- **VS Code** または **PlatformIO Core (CLI)**
- **VS Code 拡張機能**: `PlatformIO IDE`
- **テスト用サーバー実行環境**: Python 3.8+ または Node.js 16+
- **USBシリアルドライバ**: CP210x / CH340（ESP32ボードに応じたドライバ）

---

## 2. ファームウェア設定 (`src/config.h`)

書き込み前に `src/config.h` を開き、環境に合わせて設定値を変更します。

```cpp
// =============================================================================
// Wi-Fi 設定
// =============================================================================
#define WIFI_SSID         "YOUR_WIFI_SSID"      // 接続先Wi-FiのSSID (2.4GHz帯)
#define WIFI_PASSWORD     "YOUR_WIFI_PASSWORD"  // Wi-Fiのパスワード

// =============================================================================
// WebSocket サーバー設定
// =============================================================================
#define WS_SERVER_HOST    "192.168.1.100"       // サーバーのIPアドレスまたはホスト名
#define WS_SERVER_PORT    8080                  // サーバーのポート番号
#define WS_SERVER_PATH    "/ws"                 // エンドポイントパス (通常 "/" または "/ws")
#define WS_SERVER_PROTOCOL ""                   // サブプロトコル (不要な場合は "")

// =============================================================================
// サーボモータ設定
// =============================================================================
#define SERVO_PIN            18                 // PWM信号ピン (GPIO18)
#define SERVO_STANDBY_ANGLE  0                  // 待機時の角度 (0度: 押下解除位置)
#define SERVO_TRIGGER_ANGLE  90                 // 押下時の目標角度 (90度)
#define SERVO_HOLD_TIME_MS   500                // ボタン押し込み保持時間 (ミリ秒)
```

---

## 3. ビルドと書き込み (Build & Upload)

### 3.1 VS Code (PlatformIO IDE) を使用する場合
1. VS Codeで本リポジトリのルートフォルダを開きます。
2. 左下のPlatformIOツールバーから以下を実行します：
   - **Build** (チェックマークアイコン `✓`): コンパイル確認
   - **Upload** (右矢印アイコン `→`): ESP32へファームウェア書き込み
   - **Serial Monitor** (プラグアイコン `🔌`): ログ確認 (ボーレート: 115200)

### 3.2 CLI (コマンドライン) を使用する場合
```bash
# プロジェクトディレクトリに移動
cd /path/to/wol-esp

# 1. コンパイル
pio run

# 2. ESP32へファームウェア書き込み
pio run --target upload

# 3. シリアルモニタ起動
pio device monitor -b 115200
```

---

## 4. テスト用WebSocketサーバーの立ち上げ

PC側でESP32からの接続を受け付け、コマンド送受信をテストするための簡易サーバーです。Python版またはNode.js版のいずれかを使用できます。

### 4.1 Python版 テストサーバー (`test_server.py`)

#### インストール & 実行
```bash
# 依存ライブラリのインストール
pip install websockets

# サーバー起動 (ポート 8080 で待受)
python test_server.py
```

#### `test_server.py` のコード例
```python
import asyncio
import json
import websockets

CONNECTED_CLIENTS = set()

async def handler(websocket):
    CONNECTED_CLIENTS.add(websocket)
    remote_addr = websocket.remote_address
    print(f"\n[SERVER] クライアントが接続しました: {remote_addr}")

    try:
        async for message in websocket:
            print(f"[SERVER] 受信データ: {message}")
            try:
                data = json.loads(message)
                event = data.get("event")
                if event == "connected":
                    print(f"  -> ESP32デバイス登録: IP={data.get('ip')}, MAC={data.get('mac')}")
                elif event == "servo_triggered":
                    print(f"  -> サーボトリガー完了: status={data.get('status')}, angle={data.get('target_angle')}")
                elif event == "status":
                    print(f"  -> ステータス取得: free_heap={data.get('free_heap')}, uptime={data.get('uptime_ms')}ms")
            except json.JSONDecodeError:
                pass
    except websockets.exceptions.ConnectionClosed:
        print(f"\n[SERVER] クライアントが切断されました: {remote_addr}")
    finally:
        CONNECTED_CLIENTS.remove(websocket)

async def prompt_commands():
    loop = asyncio.get_event_loop()
    while True:
        cmd = await loop.run_in_executor(None, input, "\n送信コマンドを入力 (1: Trigger, 2: Angle 45, 3: Status, 4: Ping, 直接入力も可) > ")
        if not CONNECTED_CLIENTS:
            print("[SERVER] 警告: 接続中のESP32クライアントが存在しません。")
            continue

        msg = ""
        if cmd.strip() == "1":
            msg = json.dumps({"action": "servo", "angle": 90, "hold_ms": 500})
        elif cmd.strip() == "2":
            msg = json.dumps({"action": "set_angle", "angle": 45})
        elif cmd.strip() == "3":
            msg = json.dumps({"action": "status"})
        elif cmd.strip() == "4":
            msg = json.dumps({"action": "ping"})
        else:
            msg = cmd.strip()

        print(f"[SERVER] 送信中: {msg}")
        for ws in list(CONNECTED_CLIENTS):
            await ws.send(msg)

async def main():
    server = await websockets.serve(handler, "0.0.0.0", 8080)
    print("=" * 60)
    print("  ESP32 WOL WebSocket テストサーバー起動 (Port: 8080)")
    print("  サーバーIPを確認し、src/config.h の WS_SERVER_HOST に設定してください")
    print("=" * 60)
    await asyncio.gather(server.wait_closed(), prompt_commands())

if __name__ == "__main__":
    asyncio.run(main())
```

---

### 4.2 Node.js版 テストサーバー (`test_server.js`)

#### インストール & 実行
```bash
# 依存ライブラリのインストール
npm install ws readline

# サーバー起動
node test_server.js
```

#### `test_server.js` のコード例
```javascript
const WebSocket = require('ws');
const readline = require('readline');

const wss = new WebSocket.Server({ port: 8080 });
console.log('====================================================');
console.log('  ESP32 WOL WebSocket テストサーバー (Node.js) Port: 8080');
console.log('====================================================');

wss.on('connection', (ws, req) => {
    const ip = req.socket.remoteAddress;
    console.log(`\n[SERVER] クライアント接続: ${ip}`);

    ws.on('message', (message) => {
        console.log(`[SERVER] 受信: ${message.toString()}`);
    });

    ws.on('close', () => {
        console.log(`\n[SERVER] クライアント切断: ${ip}`);
    });
});

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

function prompt() {
    rl.question('\n送信コマンド (1: Trigger, 2: Angle 45, 3: Status, 4: Ping, 直接JSON) > ', (answer) => {
        let msg = answer.trim();
        if (msg === '1') msg = JSON.stringify({ action: 'servo', angle: 90, hold_ms: 500 });
        else if (msg === '2') msg = JSON.stringify({ action: 'set_angle', angle: 45 });
        else if (msg === '3') msg = JSON.stringify({ action: 'status' });
        else if (msg === '4') msg = JSON.stringify({ action: 'ping' });

        wss.clients.forEach((client) => {
            if (client.readyState === WebSocket.OPEN) {
                console.log(`[SERVER] 送信: ${msg}`);
                client.send(msg);
            }
        });
        prompt();
    });
}
prompt();
```

---

## 5. コマンドプロトコル仕様

ESP32は以下の形式のメッセージを受け付け、即座に応答を返信します。

### 5.1 サーボトリガー（物理押下）コマンド
- **送信 (JSON)**:
  ```json
  { "action": "servo", "angle": 90, "hold_ms": 500, "return_angle": 0 }
  ```
- **送信 (プレーンテキスト)**:
  ```text
  TRIGGER
  ```
- **ESP32からの返信**:
  ```json
  {
    "event": "servo_triggered",
    "status": "ok",
    "target_angle": 90,
    "hold_ms": 500,
    "return_angle": 0
  }
  ```

### 5.2 角度直接指定コマンド
- **送信 (JSON)**:
  ```json
  { "action": "set_angle", "angle": 45 }
  ```
- **ESP32からの返信**:
  ```json
  { "event": "angle_set", "status": "ok", "angle": 45 }
  ```

### 5.3 デバイス状態確認 (Status) コマンド
- **送信 (JSON / Text)**:
  ```json
  { "action": "status" }
  ```
  または `"STATUS"`
- **ESP32からの返信**:
  ```json
  {
    "event": "status",
    "wifi_connected": true,
    "ip": "192.168.1.50",
    "rssi": -58,
    "mac": "24:6F:28:XX:XX:XX",
    "ws_connected": true,
    "servo_pin": 18,
    "servo_angle": 0,
    "servo_busy": false,
    "servo_attached": false,
    "free_heap": 245312,
    "uptime_ms": 45230
  }
  ```

### 5.4 Ping / Pong 死活確認
- **送信**:
  ```json
  { "action": "ping" }
  ```
  または `"PING"`
- **ESP32からの返信**:
  ```json
  { "event": "pong", "timestamp": 45300 }
  ```

---

## 6. 実機疎通・動作確認手順

1. **配線の確認**:
   - `docs/hardware_spec.md` に従い、サーボ（SG90）の電源（5V）、GND、GPIO18が正しく接続されていることを確認します。
2. **WebSocketサーバーの起動**:
   - PC上で `python test_server.py` を実行し、待機状態にします。
3. **ESP32への給電・書き込み**:
   - USBケーブルを接続し、PlatformIOからファームウェアを書き込みます。
4. **シリアルモニタでの接続確認**:
   - 以下のログ順序を確認します：
     1. `[WiFi] Connecting to SSID: ...`
     2. `[WiFi] Connected successfully! IP Address: 192.168.1.xxx`
     3. `[MAIN] Connecting WebSocket to 192.168.1.100:8080/ws...`
     4. `[WebSocketManager] Connected to server`
     5. `[MAIN] WebSocket server connected!`
5. **テストサーバー側での接続確認**:
   - サーバー側に `[SERVER] クライアントが接続しました` および `event: connected` のJSONが表示されます。
6. **トリガー送信とサーボ動作確認**:
   - サーバープロンプトから `1` (または `{"action": "servo", "angle": 90, "hold_ms": 500}`) を送信します。
   - サーボが 90度 まで回転し、500ms 保持後に 0度 に復帰することを確認します。
   - サーボ復帰後、約500msで自動デタッチされ、ジッターや異音が発生しないことを確認します。
   - サーバー側で `event: servo_triggered`, `status: ok` の返信を受信することを確認します。

---

## 7. トラブルシューティング

| 症状 | 主な原因 | 対処法 |
| :--- | :--- | :--- |
| **Wi-Fiに接続できない (`Connection timeout`)** | SSID/PWの誤り、5GHz帯への接続試行 | `config.h` のSSID/PWを再確認。2.4GHz帯のWi-Fi APを使用してください。 |
| **WebSocketに接続できない (`Connection refused`)** | サーバーIP/ポートの誤り、PC側ファイアウォール | PCのローカルIPを `ipconfig` / `ifconfig` で確認し、Windows Defender等のファイアウォールでポート8080を許可してください。 |
| **サーボ動作時にESP32が再起動する (`Brownout detector`)** | サーボ電源をESP32の3.3Vピンから給電している、電源容量不足 | `docs/hardware_spec.md` を参照し、サーボ電源を5V/VINに接続し、100µF以上の電解コンデンサを並列接続してください。 |
| **サーボが動かない / ピクピク震えるだけ** | 信号ピンの誤り、GNDの未接続 | GPIO18とサーボの信号線（橙/黄）が正しく接続されているか確認。共通GNDを確実に接続してください。 |

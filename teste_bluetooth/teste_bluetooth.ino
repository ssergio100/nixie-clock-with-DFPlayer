#include "sdkconfig.h"

// Pino do LED onboard
#if defined(LED_BUILTIN)
  #define LED_PIN LED_BUILTIN
#else
  #define LED_PIN 2
#endif

// Verificação de suporte a Bluetooth Classic no hardware
#if defined(CONFIG_BT_CLASSIC_ENABLED) && !defined(DISABLE_BT_CLASSIC)
  #define USA_BT_CLASSIC
#endif

// ==========================================
// MODO 1: BLUETOOTH CLASSIC (SPP)
// ==========================================
#ifdef USA_BT_CLASSIC
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// Callback para monitorar eventos do stack Bluetooth SPP
void bluetoothCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("\n[BT Clássico] Conectado! Canal de comunicação SPP aberto.");
  }
  else if (event == ESP_SPP_CLOSE_EVT) {
    Serial.println("\n[BT Clássico] Desconectado!");
  }
}

void setupBluetooth() {
  // Registra o callback antes de inicializar o Bluetooth
  SerialBT.register_callback(bluetoothCallback);

  // Nome do dispositivo Bluetooth que aparecerá no celular
  SerialBT.begin("Nixie_Clock_BT");
  Serial.println("Bluetooth Classic ativado!");
  Serial.println("Nome do dispositivo: 'Nixie_Clock_BT'");
  Serial.println("Conecte-se usando um app como o 'Serial Bluetooth Terminal'.");
}

void loopBluetooth() {
  // Repassa o que recebe da Serial do PC para o Bluetooth
  if (Serial.available()) {
    char c = Serial.read();
    SerialBT.write(c);
  }

  // Repassa o que recebe do Bluetooth para a Serial do PC e processa comandos
  if (SerialBT.available()) {
    char c = SerialBT.read();
    Serial.write(c); // Ecoa no terminal do PC

    // Processa comandos simples
    if (c == '1' || c == 'L' || c == 'l') {
      digitalWrite(LED_PIN, HIGH);
      SerialBT.println("\n[BT] LED Ligado");
    }
    else if (c == '0' || c == 'D' || c == 'd') {
      digitalWrite(LED_PIN, LOW);
      SerialBT.println("\n[BT] LED Desligado");
    }
  }
}

// ==========================================
// MODO 2: BLUETOOTH LOW ENERGY (BLE - UART)
// Usado em ESP32-S3 (ex: Nano ESP32) ou ESP32-C3
// ==========================================
#else
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// UUIDs padrão para o serviço Nordic UART (NUS)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.print("[BLE Recebido]: ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();

        // Processa o primeiro caractere como comando
        char c = rxValue[0];
        if (c == '1' || c == 'L' || c == 'l') {
          digitalWrite(LED_PIN, HIGH);
          // Envia resposta de volta
          uint8_t txBuf[] = "[BLE] LED Ligado\n";
          pTxCharacteristic->setValue(txBuf, sizeof(txBuf) - 1);
          pTxCharacteristic->notify();
        }
        else if (c == '0' || c == 'D' || c == 'd') {
          digitalWrite(LED_PIN, LOW);
          uint8_t txBuf[] = "[BLE] LED Desligado\n";
          pTxCharacteristic->setValue(txBuf, sizeof(txBuf) - 1);
          pTxCharacteristic->notify();
        }
      }
    }
};

void setupBluetooth() {
  // Inicializa o dispositivo BLE
  BLEDevice::init("Nixie_Clock_BLE");

  // Cria o Servidor BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Cria o Serviço BLE (Nordic UART)
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Cria a Característica TX (para enviar dados ao celular/notificar)
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  // Cria a Característica RX (para receber dados do celular)
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Inicia o serviço
  pService->start();

  // Inicia a divulgação (advertising)
  pServer->getAdvertising()->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  
  Serial.println("Bluetooth Low Energy (BLE) ativado!");
  Serial.println("Nome do dispositivo: 'Nixie_Clock_BLE'");
  Serial.println("Conecte-se usando um app como 'Serial Bluetooth Terminal' (aba BLE).");
}

void loopBluetooth() {
  // Repassa o que recebe da Serial do PC para o BLE
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg += "\n";
    if (deviceConnected) {
      pTxCharacteristic->setValue((uint8_t*)msg.c_str(), msg.length());
      pTxCharacteristic->notify();
      Serial.print("[BLE Enviado]: " + msg);
    } else {
      Serial.println("[Aviso] BLE desconectado. Mensagem nao enviada.");
    }
  }

  // Gerenciamento de conexões/desconexões
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Dá um tempo para o stack de BT resetar
    pServer->startAdvertising(); // Reinicia o advertising
    Serial.println("[BLE] Aguardando nova conexao...");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    Serial.println("[BLE] Dispositivo conectado!");
    oldDeviceConnected = deviceConnected;
  }
}
#endif

// ==========================================
// FUNÇÕES PRINCIPAIS DO ARDUINO
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("\n=========================================");
  Serial.println("--- Teste de Bluetooth no ESP32 ---");
  Serial.printf("Modelo do Chip: %s\n", ESP.getChipModel());
  Serial.printf("Revisao do Chip: %d\n", ESP.getChipRevision());
  Serial.println("=========================================\n");

  setupBluetooth();
}

void loop() {
  loopBluetooth();
  delay(10); // Pequeno delay para aliviar a CPU do ESP32
}

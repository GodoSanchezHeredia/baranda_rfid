#include <SPI.h>
#include <MFRC522.h>

// Pines RFID
#define RST_PIN 9
#define SS_PIN 10
MFRC522 rfid(SS_PIN, RST_PIN);

// Pines Ultrasonido
const int trigPin = 37;
const int echoPin = 35;

// Pines Limitadores (Pull-up)
const int inputPin1 = 22;
const int inputPin2 = 23;

// Pines Motor
const int dirPin1 = 4;
const int dirPin2 = 5;
const int pwmPin = 7;
const int standbyPin = 6;

// Pin LED
const int ledPin = 33;

// Variables de estado
bool motorActive = false;
unsigned long noDetectionStart = 0;
const unsigned long noDetectionDelay = 10000; // 10 segundos

void setup() {
  // Configuración de pines
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(inputPin1, INPUT_PULLUP);
  pinMode(inputPin2, INPUT_PULLUP);
  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(standbyPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // Configuración de RFID
  SPI.begin();
  rfid.PCD_Init();

  // Inicialización
  digitalWrite(standbyPin, HIGH); // Activar motor standby
  digitalWrite(ledPin, LOW);      // LED apagado
}

void loop() {
  // Verificar detección RFID
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    unsigned long startNoDetection = 0; // Tiempo sin detección
    bool isObjectDetected = true;      // Bandera para el estado de detección
    bool inReverse = false;            // Bandera para indicar si está en reversa

    while (true) {
      // Verificar detección por ultrasonido
      if (ultrasonicDetectObject()) {
        isObjectDetected = true;       // Reiniciar detección
        inReverse = false;             // Salir de modo reversa
        blinkLED();                    // Indicar que se detectó un objeto

        if (digitalRead(inputPin2) != 0) {
          stopMotor();
        } else {
          activateMotor();
        }

      } else {
        // Si deja de detectar un objeto
        if (isObjectDetected) {
          startNoDetection = millis(); // Iniciar el temporizador
          isObjectDetected = false;   // Marcar que no hay objeto detectado
        }

        // Verificar si han pasado 10 segundos sin detección
        if (millis() - startNoDetection >= 10000) {
          if (!inReverse) {
            reverseMotor(); // Accionar marcha atrás
            inReverse = true; // Activar modo reversa
          }

          // Verificar el estado de inputPin1 mientras está en reversa
          if (digitalRead(inputPin1) != 0) {
            stopMotor();  // Detener el motor
            break;        // Salir del bucle para esperar un nuevo RFID
          }
        }
      }
    }
  }
}



bool ultrasonicDetectObject() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2; // Distancia en cm

  return (distance < 10);
}

void blinkLED() {
  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);
  delay(200);
}

void activateMotor() {
  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, LOW);
  analogWrite(pwmPin, 250); // Velocidad máxima
  motorActive = true;
}

void stopMotor() {
  analogWrite(pwmPin, 0);
  motorActive = false;
}

void reverseMotor() {
  digitalWrite(dirPin1, LOW);
  digitalWrite(dirPin2, HIGH);
  analogWrite(pwmPin, 255); // Velocidad máxima
}

bool waitForNoDetection() {
  noDetectionStart = millis();
  while (millis() - noDetectionStart < noDetectionDelay) {
    if (ultrasonicDetectObject()) {
      noDetectionStart = millis(); // Reiniciar temporizador
    }
  }
  return true;
}

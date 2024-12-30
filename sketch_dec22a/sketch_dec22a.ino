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
const int ledPin = 31;
const int led2pin = 30;
enum State { STOP, FORWARD, REVERSE };
 State currentState = STOP; 
// Variables de estado
bool motorActive = false;
unsigned long noDetectionStart = 0;
const unsigned long noDetectionDelay = 10000; // 10 segundos
void Contigencia(void);
bool ledState = LOW;  
unsigned long previousMillis = 0; // Tiempo anterior

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
    pinMode(led2pin, OUTPUT);

  pinMode(26, INPUT_PULLUP); // Configurar el pin 26 como entrada con pull-up
  pinMode(25, INPUT_PULLUP); // Configurar el pin 26 como entrada con pull-up
  pinMode(27, INPUT_PULLUP); // Configurar el pin 26 como entrada con pull-up
  pinMode(28, INPUT_PULLUP); // Configurar el pin 26 como entrada con pull-up

  // Configuración de RFID
  SPI.begin();
  rfid.PCD_Init();

  // Inicialización
  digitalWrite(standbyPin, HIGH); // Activar motor standby
  digitalWrite(ledPin, LOW);      // LED apagado
}

void loop() {
  // Verificar el estado del botón de emergencia
  Contigencia();

  // Continuar con el flujo normal si se detecta un RFID
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    unsigned long startNoDetection = 0;
    bool isObjectDetected = true;
    bool inReverse = false;

    while (true) {

  Contigencia();
  digitalWrite(ledPin,LOW);
  digitalWrite(led2pin, HIGH);

      // Verificar si el ultrasonido detecta un objeto
      if (ultrasonicDetectObject()) {
        isObjectDetected = true;
        inReverse = false;
         digitalWrite(led2pin, HIGH);


        // Activar o detener el motor según el estado de inputPin2
        if (digitalRead(inputPin2) != 0) {
          stopMotor();
        } else {
          activateMotor();
        }
      } else {
        if (isObjectDetected) {
          startNoDetection = millis();
          isObjectDetected = false;
        }

        // Verificar si no se detecta objeto durante 10 segundos
        if (millis() - startNoDetection >= 10000) {
          if (!inReverse) {
            reverseMotor();
            inReverse = true;
          }

          // Si inputPin1 está activo, detener el motor y salir del bucle
          if (digitalRead(inputPin1) != 0) {
            stopMotor();
            return; // Salir y esperar un nuevo RFID
          }
        }
      }

      // Si el sistema está en reversa, seguir revisando el estado del botón de emergencia
      if (inReverse) {
          digitalWrite(led2pin, HIGH);

        if (digitalRead(26) == LOW || digitalRead(25) == LOW) {
          stopMotor();
              State currentState = STOP;            // Estado inicial
    while (digitalRead(26) == LOW || digitalRead(25) == LOW) {
  digitalWrite(ledPin,HIGH);

      if (digitalRead(25) == LOW) {
                  blinkLED();
         if (digitalRead(27) == LOW) {
        currentState = FORWARD;
      } else if (digitalRead(28) == LOW) {
        currentState = REVERSE;
      } else {
        currentState = STOP;
      }
      switch (currentState) {
        case FORWARD:
        if (digitalRead(inputPin2) == 0){
          activateMotor();
        }else{
          stopMotor(); 
        }
          break;
        case REVERSE:
        if (digitalRead(inputPin1) == 0){
          reverseMotor();
        }else{
          stopMotor(); 
        }
          
          break;
        case STOP:
        default:
          stopMotor();
          break;
      }

      }


      delay(100); // Esperar hasta que se suelte el botón
    }  digitalWrite(ledPin, HIGH);

          // Continuar con el reversa después de que el botón de emergencia se haya liberado
          reverseMotor();
        }
      }
    }
    
  }
    digitalWrite(led2pin, LOW);

}

void Contigencia(void){
  if (digitalRead(26) == LOW || digitalRead(25) == LOW) {
      stopMotor();
      State currentState = STOP;            // Estado inicial
      while (digitalRead(26) == LOW || digitalRead(25) == LOW) {
          digitalWrite(ledPin,HIGH);
        if (digitalRead(25) == LOW) {
          digitalWrite(ledPin,LOW);
          blinkLED();
          if (digitalRead(27) == LOW) {
            currentState = FORWARD;
          } else if (digitalRead(28) == LOW) {
            currentState = REVERSE;
          } else {
            currentState = STOP;
        }
        switch (currentState) {
          case FORWARD:
          if (digitalRead(inputPin2) == 0){
            activateMotor();
          }else{
            stopMotor(); 
          }
            break;
          case REVERSE:
          if (digitalRead(inputPin1) == 0){
            reverseMotor();
          }else{
            stopMotor(); 
          }
            
            break;
          case STOP:
          default:
            stopMotor();
            break;
        }

        }


        delay(100); // Esperar hasta que se suelte el botón
      }
      // Regresar al principio del loop, esperando un nuevo RFID
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
  delay(100);
  digitalWrite(ledPin, LOW);
  delay(100);
}
/* 
void blinkLED(unsigned long interval) {
  unsigned long currentMillis = millis(); // Obtener el tiempo actual

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Actualizar el tiempo anterior

    // Cambiar el estado del LED
    ledState = !ledState;
    digitalWrite(led2pin, ledState);
  }
}*/

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

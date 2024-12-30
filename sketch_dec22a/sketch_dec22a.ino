#include <SPI.h>
#include <MFRC522.h>

// Pines RFID
#define RST_PIN 36
#define SS_PIN 38
MFRC522 rfid(SS_PIN, RST_PIN);
// laser tx - rx
const int txlaser = 32;
const int rxlaser = 34;


// Pines Ultrasonido
const int trigPin = 31;
const int echoPin = 30;
const int trigPin1 = 29;
const int echoPin1 = 28;

// Pines Limitadores (Pull-up)
const int inputPin1 = 44;
const int inputPin2 = 40;

// Pines Motor
const int dirPin1 = 9;
const int dirPin2 = 10;
const int pwmPin = 11;
const int standbyPin = 8;

// Pin LED

const int Output1 = 21;
const int Output2 = 12;
const int Output3 = 20;
const int Output4 = 13;
const int Output5 = 22;

// Pin input

const int Input3 = 23;
const int Input4 = 24;
const int Input5 = 25;
const int Input6 = 26;
const int Input7 = 27;

enum State { STOP, FORWARD, REVERSE };
 State currentState = STOP; 
// Variables de estado
bool motorActive = false;
unsigned long noDetectionStart = 0;
const unsigned long noDetectionDelay = 10000; // 10 segundos
void Contigencia(void);
void SEMAFORO(void);

bool ledState = LOW;  
unsigned long previousMillis = 0; // Tiempo anterior

void setup() {
  // Configuración de pines
    Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);

  pinMode(inputPin1, INPUT_PULLUP);
  pinMode(inputPin2, INPUT_PULLUP);

  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(standbyPin, OUTPUT);

  pinMode(Output1, OUTPUT);
  pinMode(Output2, OUTPUT);
  pinMode(Output3, OUTPUT);
  pinMode(Output4, OUTPUT);
  pinMode(Output5, OUTPUT);

  pinMode(txlaser, OUTPUT);
  pinMode(rxlaser, INPUT); // Configurar el pin 26 como entrada con pull-up


  pinMode(Input3, INPUT_PULLUP); // Configurar el pin 26 como entrada con pull-up
  pinMode(Input4, INPUT_PULLUP); // Configurar el pin 25 como entrada con pull-up
  pinMode(Input5, INPUT_PULLUP); // Configurar el pin 27 como entrada con pull-up
  pinMode(Input6, INPUT_PULLUP); // Configurar el pin 28 como entrada con pull-up
  pinMode(Input7, INPUT_PULLUP); // Configurar el pin 28 como entrada con pull-up

  // Configuración de RFID
  SPI.begin();
  rfid.PCD_Init();

  // Inicialización
  digitalWrite(standbyPin, HIGH); // Activar motor standby
  digitalWrite(Output1, LOW);      // LED apagado
  digitalWrite(Output2, LOW);      // LED apagado
  digitalWrite(Output3, LOW);      // LED apagado
  digitalWrite(Output4, LOW);      // LED apagado
  digitalWrite(Output5, LOW);      // LED apagado

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
  SEMAFORO();
  digitalWrite(Output3,LOW);
  digitalWrite(Output4, HIGH);

      // Verificar si el ultrasonido detecta un objeto
      if (ultrasonicDetectObject()) {
        isObjectDetected = true;
        inReverse = false;
        digitalWrite(Output4, HIGH);
        digitalWrite(txlaser, HIGH);


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
            Serial.print("modo reversa");
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
          digitalWrite(Output4, HIGH);
      SEMAFORO();
        if (digitalRead(Input3) == LOW || digitalRead(Input6) == LOW) {
          stopMotor();
              State currentState = STOP;            // Estado inicial
    while (digitalRead(Input3) == LOW || digitalRead(Input6) == LOW) {
  digitalWrite(Output3,HIGH);

      if (digitalRead(Input6) == LOW) {
                  blinkLED();
         if (digitalRead(Input5) == LOW) {
        currentState = FORWARD;
      } else if (digitalRead(Input7) == LOW) {
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
    }  digitalWrite(Output3, HIGH);
    
          // Continuar con el reversa después de que el botón de emergencia se haya liberado
          reverseMotor();
        }
      }
    }
    
  }
    digitalWrite(Output4, LOW);
    digitalWrite(Output1, LOW);
    digitalWrite(Output2, LOW);
    digitalWrite(txlaser,LOW);

}
void SEMAFORO(void){
if(!digitalRead(rxlaser)){
    digitalWrite(Output1, HIGH);
    digitalWrite(Output2, LOW);

}else{
    digitalWrite(Output1, LOW);
    digitalWrite(Output2, HIGH);

}
}
void Contigencia(void){
  if (digitalRead(Input3) == LOW || digitalRead(Input6) == LOW) {
      stopMotor();
      State currentState = STOP;            // Estado inicial
      while (digitalRead(Input3) == LOW || digitalRead(Input6) == LOW) {
          digitalWrite(Output3,HIGH);
        if (digitalRead(Input6) == LOW) {
          digitalWrite(Output3,LOW);
          blinkLED();
          if (digitalRead(Input5) == LOW) {
            currentState = FORWARD;
          } else if (digitalRead(Input7) == LOW) {
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
   // Serial.print("Distancia: ");
  //Serial.print(distance);
 // Serial.println(" cm");
  return (distance < 10);
}

void blinkLED() {
  digitalWrite(Output3, HIGH);
    digitalWrite(Output5, HIGH);

  delay(100);
  digitalWrite(Output3, LOW);
    digitalWrite(Output5, LOW);

  delay(100);
}
/* 
void blinkLED(unsigned long interval) {
  unsigned long currentMillis = millis(); // Obtener el tiempo actual

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Actualizar el tiempo anterior

    // Cambiar el estado del LED
    ledState = !ledState;
    digitalWrite(Output4, ledState);
  }
}*/

void activateMotor() {
  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, LOW);
  analogWrite(pwmPin, 150); // Velocidad máxima
  motorActive = true;
}

void stopMotor() {
  analogWrite(pwmPin, 0);
  motorActive = false;
}

void reverseMotor() {
  digitalWrite(dirPin1, LOW);
  digitalWrite(dirPin2, HIGH);
  analogWrite(pwmPin, 150); // Velocidad máxima
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

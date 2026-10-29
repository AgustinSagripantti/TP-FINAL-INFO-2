#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN  10
#define RST_PIN  9
#define LED_ROJO 8
#define LED_VERDE 7
#define BUZZER 6
#define MOTOR 5
#define LED_DOOR 3
#define BUTTON 2

MFRC522 rfid(SS_PIN, RST_PIN);

unsigned long tiempoInicio = 0;
bool esperandoPulsador = false;

bool auth = false;
int etapa = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(LED_DOOR, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  pinMode(BUTTON, INPUT);

  SPI.begin();
  rfid.PCD_Init();
}

void loop() {
 
  if (!auth){ 
    digitalWrite(LED_ROJO, HIGH);  
  }

   // Si no hay tarjeta o no se puede leer el serial, no hace nada
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial() || digitalRead(BUTTON)){
    return;
  }

  // Envía únicamente la cadena HEX continua (ej: A30F5B12)
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print(F("0"));
    }
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println(); // El '\n' vital para delimitar en la PC

  // Detiene la tarjeta para evitar ráfagas repetidas
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(200); // Pequeño respiro anti-rebote 

    while(Serial.available() > 0){
    
    if(Serial.read() == '1'){
        auth = true;
        digitalWrite(LED_ROJO, LOW);
        digitalWrite(LED_VERDE, HIGH);
        digitalWrite(BUZZER, HIGH);
        analogWrite(MOTOR, 125);
        tiempoInicio = millis();

     } 
}


    while(auth){
       bool botonPresionado = (digitalRead(BUTTON) == HIGH);
      unsigned long delta = millis() - tiempoInicio;
      // 1. A los 500 ms
      if (delta >= 500 ) {
       // Tu acción acá (ej: prender un buzzer, LED, etc.)
         analogWrite(MOTOR, 0);

    if (botonPresionado) {
      digitalWrite(LED_DOOR, HIGH);
      digitalWrite(BUZZER, LOW);
    } else {
      digitalWrite(LED_DOOR, LOW);
    }   

     }



     // 3. A los 3000 ms (3 segundos desde el inicio) -> REINICIO
     if(delta  >= 3000 && !botonPresionado) {
       digitalWrite(BUZZER, LOW);
       digitalWrite(LED_DOOR, LOW);
       digitalWrite(LED_VERDE, LOW);
       digitalWrite(LED_ROJO, HIGH);
       analogWrite(MOTOR, 125);
       delay(500);
       analogWrite(MOTOR, 0);
       auth = false;
      }  
  }
}

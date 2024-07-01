  // version 7;  movement acknowledgement added every "sak" steps 
  // version 8; added bounce of "bounce steps if movement is counterclockwise; acknowledgement every "sak" steps REMOVED
  // version 9. added calibration procedure "C" and changed "p" command
  //version 11; some user messages (feedback) silenced for MIDAS integration 
  //version 12; Added command "V" for verbouse user messagges toggle
  /*version 13; Add message code: 100#; 
    1000: Microswitch pressed.
    1001:Calib
    1002: TimeOut
    1003: end Calib
  
  */
  //version 14: standbay motor ,added function ,motor_off()
  //version 15: number storable position from 6 to 10
  //version 16:  message  "wait...engine is running" (-1010)  displayed if serial access happen when stepper is running;
  #include <EEPROM.h>
  
  // Parameters
  const int torque = 1;
  const int MAX_SPEED = int(255 / torque);
  const int directionA = 12;
  const int directionB = 13;
  const int brakeA = 9;
  const int brakeB = 8;
  const int speedA = 3;
  const int speedB = 11;
  //const int in2 = A2;
  //const int in3 = A3;
  const int microswitch2Pin = 2; // Pin for the second microswitch

  const bool FORWARD = true;
  const bool BACKWARD = false;
  
  bool verbous = false;

  // volatile bool microswitch1Pressed = false;
  volatile bool microswitch2Pressed = false;
  
  // Variables for the reference position and the current position
 
  const int bounce = 10;
  int origine = 0;
  int posizioneAttuale = 0;
  int moving = -1010;
  const int NUM_POSIZIONI = 10;
  int posizioniDiRiferimento[NUM_POSIZIONI];
  //int posizioneFineCorsa1 = 0;
  //int posizioneFineCorsa2 = 0;
  
// Microswitches
// const int endStop1Pin = 4;
//  const int endStop2Pin = 5;
  
  void setup() {
    // Init Serial USB
    Serial.begin(9600);
   // Serial.println(F("Initialize System"));
  
    // Init Motor Shield
    pinMode(microswitch2Pin, INPUT_PULLUP);
    pinMode(directionA, OUTPUT); // Initiates Motor Channel A pin
    pinMode(brakeA, OUTPUT); // Initiates Brake Channel A pin
    pinMode(directionB, OUTPUT); // Initiates Motor Channel B pin
    pinMode(brakeB, OUTPUT); // Initiates Brake Channel B pin
  
   // Serial.println(F("Initialize interrupt"));
  
    // Interrupt configuration
    // attachInterrupt(digitalPinToInterrupt(microswitch1Pin), microswitch1Interrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(microswitch2Pin), microswitch2Interrupt, FALLING);
  
   // Serial.println("Controllo dello Stepper Motor tramite Serial");
  
    // Read the saved position from EEPROM
    // origine = EEPROM.read(2 + NUM_POSIZIONI) | (EEPROM.read(3 + NUM_POSIZIONI) << 8);
  
    for (int i = 0; i < NUM_POSIZIONI; i++) {
      posizioniDiRiferimento[i] = EEPROM.read(i * 2) | (EEPROM.read((i * 2) + 1) << 8);
    }
  
    // Print the initial position
   // Serial.print("Loaded position from EEPROM: ");
   // Serial.println(origine);
  
     //testStepperMS();
    //calibraPosizione();
    // printMenu(); // Print the initial menu
  }
  
  void loop() {
    if (Serial.available() > 0) {
      char command = Serial.read();
  
      if (command == 'F' || command == 'f') {
        if(verbous) Serial.println("Avanti");
        motorStep(10, FORWARD);
      } 
      else if (command == 'B' || command == 'b') {
        if(verbous) Serial.println("Indietro");
        motorStep(10, BACKWARD);
      } 
      else if (command == 'V' || command == 'v') {
        verbous = !verbous;
        verbous ? Serial.print("Messaggi utente attivati.\n"):Serial.print("Messaggi utente disattivati.\n");
        }
      else if (command == 'S' || command == 's') {
        if(verbous) Serial.println("Inserisci il numero di posizione di riferimento (da 0 a 9):");
        while (Serial.available() <= 0);
        int posizioneScelta = Serial.parseInt();
  
        if (posizioneScelta >= 0 && posizioneScelta < NUM_POSIZIONI) {
          posizioniDiRiferimento[posizioneScelta] = posizioneAttuale;
          EEPROM.write(posizioneScelta * 2, posizioneAttuale & 0xFF);
          EEPROM.write((posizioneScelta * 2) + 1, (posizioneAttuale >> 8) & 0xFF);
          if(verbous) Serial.print("Posizione di riferimento ");
          if(verbous) Serial.print(posizioneScelta);
          if(verbous) Serial.print(" salvata alla posizione ");
          if(verbous) Serial.println(posizioneAttuale);
        } else {
          verbous ? Serial.println("Posizione non valida.") : Serial.print("-1") ;
        }
      }
      else if (command == 'P' || command == 'p') {
       if(verbous) Serial.println("Inserisci il numero di posizione di riferimento da visualizzare (da 0 a 9):");
        while (Serial.available() <= 0);
        int posizioneScelta = Serial.parseInt();
  
        if (posizioneScelta >= 0 && posizioneScelta < NUM_POSIZIONI) {
         // Serial.print("Posizione ");
         // Serial.print(posizioneScelta);
         //Serial.print(": ");
  
          if (posizioniDiRiferimento[posizioneScelta] != -1) {
            Serial.println(posizioniDiRiferimento[posizioneScelta]);
          } else {
            Serial.println("-1");
          }
        } else {
         verbous ? Serial.println("-1") : Serial.print("riferimento non disponibile \n");
        }
      } 
      else if (command == 'G' || command == 'g') {
        // Command to move directly forward or backward
        int passi = 0;
        if(verbous) Serial.println("Inserisci il numero di passi per lo spostamento:");
        while (Serial.available() <= 0);
        passi = Serial.parseInt();
        int direzione = FORWARD;
  
        if (passi < 0) {
          direzione = BACKWARD;
          passi = -passi;
        }
  
        motorStep(passi, direzione);
        if(verbous) Serial.print("Motore spostato di ");
        Serial.print(passi);
        if(verbous) Serial.println(" passi.");
      } 
      else if (command == 'A' || command == 'a') {
        // Command to know the current position
       if(verbous) Serial.print("Posizione attuale: ");
        Serial.println(posizioneAttuale);
      } 
      else if (command == 'O' || command == 'o') {
        // Command to define the current position as the new origin
        origine -= posizioneAttuale;
        posizioneAttuale = 0;
        if(verbous) Serial.println("Posizione attuale definita come nuova origine.");
      } 
      else if (command == 'M' || command == 'm') {
        // Command to show the menu
        printMenu();
      } 
      else if (command == 'R' || command == 'r') {
        //Serial.println("Inserisci il numero di posizione di riferimento a cui tornare (da 0 a 9):");
        while (Serial.available() <= 0);
        int posizioneScelta = Serial.parseInt();
  
        if (posizioneScelta >= 0 && posizioneScelta < NUM_POSIZIONI) {
          int posizioneSalvata = posizioniDiRiferimento[posizioneScelta];
          if (posizioneSalvata != -1) {
            int passiDaMuovere = posizioneSalvata - posizioneAttuale;
  
            if (passiDaMuovere > 0) {
              motorStep(passiDaMuovere, FORWARD);
            } else if (passiDaMuovere < 0) {
              motorStep(-passiDaMuovere, BACKWARD);
            } else {
            if(verbous) Serial.println("Sei già alla posizione di riferimento.");
            }
          } else {
           verbous ? Serial.println("Posizione di riferimento non valida.") : Serial.print ("-1");
          }
        } else {
          verbous ? Serial.println("Posizione non valida") : Serial.print("-1");
        }
      } 
      else if (command == 'C' || command == 'c') {
        calibraPosizione();
      } 
       else if (command == '\n' || command == '\r') {
        //verbous ? Serial.println("è stato premuto enter") : Serial.print("");
      }       
      else {
        verbous ? Serial.println("Comando non valido") : Serial.print("-1");
      }
    }
  }
  
  void testStepperMS() {
    Serial.println("Move stepper 1 step clockwise");
    stpCW(10);
    Serial.println("Move stepper 1 step counter clockwise");
    stpCCW(10);
    delay(1000);
    motor_off();
  }
  /*
  void readSensorMS() {
    Serial.print(F("In2 : "));
    Serial.println(analogRead(in2));
    Serial.print(F("In3 : "));
    Serial.println(analogRead(in3));
  }
  */
  
  void stpCW(int nbstep) {
    for (int i = 0; i < nbstep; i++) {
      digitalWrite(brakeA, LOW);
      digitalWrite(brakeB, HIGH);
      digitalWrite(directionA, HIGH);
      analogWrite(speedA, MAX_SPEED);
      delay(30);
  
      digitalWrite(brakeA, HIGH);
      digitalWrite(brakeB, LOW);
      digitalWrite(directionB, LOW);
      analogWrite(speedB, MAX_SPEED);
      delay(30);
  
      digitalWrite(brakeA, LOW);
      digitalWrite(brakeB, HIGH);
      digitalWrite(directionA, LOW);
      analogWrite(speedA, MAX_SPEED);
      delay(30);
  
      digitalWrite(brakeA, HIGH);
      digitalWrite(brakeB, LOW);
      digitalWrite(directionB, HIGH);
      analogWrite(speedB, MAX_SPEED);
      delay(30);
    }
  }
  
  void stpCCW(int nbstep) {
    for (int i = 0; i < nbstep; i++) {
      digitalWrite(brakeA, LOW);
      digitalWrite(brakeB, HIGH);
      digitalWrite(directionA, HIGH);
      analogWrite(speedA, MAX_SPEED);
      delay(30);
  
      digitalWrite(brakeA, HIGH);
      digitalWrite(brakeB, LOW);
      digitalWrite(directionB, HIGH);
      analogWrite(speedB, MAX_SPEED);
      delay(30);
  
      digitalWrite(brakeA, LOW);
      digitalWrite(brakeB, HIGH);
      digitalWrite(directionA, LOW);
      analogWrite(speedA, MAX_SPEED);
      delay(30);
  
      digitalWrite(brakeA, HIGH);
      digitalWrite(brakeB, LOW);
      digitalWrite(directionB, LOW);
      analogWrite(speedB, MAX_SPEED);
      delay(30);
    }
  }


void motor_off(){

      /*digitalWrite(brakeA, LOW);
      digitalWrite(brakeB, LOW);
      digitalWrite(directionB, LOW);
      digitalWrite(directionA, LOW);
      analogWrite(speedB, 0);
      analogWrite(speedA, 0);      
      */
      
     analogWrite(speedB, 0);
     analogWrite(speedA, 0); 
      
     
      if(verbous) Serial.println("Stepper motor off");
  
}

  
  
  void motorStep(int passi, bool direzione) {
    int steps = passi;
    
    if (direzione == FORWARD) {
      while (steps > 0 && !microswitch2Pressed) {
        stpCW(1);
        steps--;
        if (Serial.available() > 0) {
         Serial.read();
         verbous ? Serial.println("wait...engine is running") : Serial.println(moving);
         Serial.flush();
        }
      }
      
      posizioneAttuale += (passi - steps);
      if (microswitch2Pressed) {
        stpCCW(10);
        delay(1000);
        microswitch2Pressed = false;
        posizioneAttuale -= 10;
      }
    } else {
      steps += bounce;
      while (steps > 0 && !microswitch2Pressed) {
        stpCCW(1);
        steps--;
        if (Serial.available() > 0) {
         Serial.read();
         verbous ? Serial.println("wait...engine is running") : Serial.println(moving);
         Serial.flush();
        }
      }
      posizioneAttuale -= (passi - steps+bounce);
  
      if (microswitch2Pressed) {
        stpCW(10);
        delay(1000);
        microswitch2Pressed = false;
       // posizioneAttuale += (10-bounce);
        posizioneAttuale = 0; //ridefinisce l'origine
      } else {
        stpCW(bounce);
        posizioneAttuale += bounce;
      }
    }
      motor_off();
  }
  
  void printMenu() {
  //readSensorMS();
    Serial.println("********** Menu dei Comandi ***********");
    Serial.println("  F: Avanti 10 steps");
    Serial.println("  B: Indietro 10 steps");
    Serial.println("  S(0-9): Salva la posizione attuale come riferimento");
    Serial.println("  R(0-9): Vai alla posizione di riferimento");
    Serial.println("  P(0-9): Stampa posizione di riferimento");
    Serial.println("  G(+ o -)(n step): Spostamento di n steps clockwise (+) o counterclockwise (-)");
    Serial.println("  A: Stampa la posizione attuale");
    Serial.println("  O: Definisci la posizione attuale come nuova origine");
    Serial.println("  C: Calibra la posizione (definisce l'origine nella posizione centrale)");
    Serial.println("  V: attiva/disattiva messaggi utente (attiva messaggi di feedback testuali");
    Serial.println("  M: Mostra questo menu");
   
  }
  
  void microswitch2Interrupt() {
    verbous ? Serial.println("End stop  is triggered") : Serial.print("-1000");
    microswitch2Pressed = true;
  }
  
  void calibraPosizione() {
  
   microswitch2Pressed = false;
   verbous ? Serial.println("Calibrazione: Muovo il motore all'indietro per raggiungere il microswitch...") : Serial.print("-1001");
   unsigned long startTime = millis();  // Tempo di inizio in millisecondi
   const unsigned long timeout = 100000;  // Timeout di 100 secondi
  
    while (!microswitch2Pressed) {
      stpCCW(1);
       if (Serial.available() > 0) {
         Serial.read();
         verbous ? Serial.println("wait...engine is running") : Serial.println(moving);
         Serial.flush();
        }
      // Controllo del timeout
      if (millis() - startTime > timeout) {
      verbous ? Serial.println("Timeout! La calibrazione ha superato il limite di tempo.") : Serial.print("-1002");
        return;  // Esci dalla funzione in caso di timeout
       }
      }
    
    stpCW(10);
    origine -= posizioneAttuale;
    posizioneAttuale = 0;
    microswitch2Pressed = false;
    verbous ? Serial.print("Calibrazione completata") : Serial.print("-1003");
    motor_off();
  }

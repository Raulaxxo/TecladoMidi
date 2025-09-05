// TEST BÁSICO - DIAGNÓSTICO COMPLETO
#include "MIDIUSB.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configuraciones básicas
#define NUM_ROWS 6
#define NUM_COLS 9
#define START_NOTE 36  // Cambiado a C2 (más seguro)
#define DEBOUNCE_DELAY 50  // Aumentado debounce

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pines
const int rowPins[NUM_ROWS] = {4,5,6,7,8,9};
const int dataPin = 10;
const int latchPin = 11;
const int clockPin = 12;
const int octaveUpPin = 13;
const int octaveDownPin = A1;
const int modeButtonPin = A0;

// Variables básicas
boolean keyPressed[NUM_ROWS][NUM_COLS];
uint8_t keyToMidiMap[NUM_ROWS][NUM_COLS];
unsigned long lastPressTime[NUM_ROWS][NUM_COLS];
byte midiChannel = 0;
byte currentVelocity = 100;
int currentOctaveShift = 0;  // Empezar en 0 para ser más seguro
int lastPlayedNote = -1;

// Shift register patterns
byte bits[NUM_COLS] = {
  B11111110, B11111101, B11111011, B11110111,
  B11101111, B11011111, B10111111, B01111111, B11111111
};

void setup() {
  Serial.begin(115200);
  Serial.println("=== INICIO DIAGNÓSTICO COMPLETO ===");
  
  // Test 1: Configurar pines
  Serial.println("1. Configurando pines...");
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  for(int i = 0; i < NUM_ROWS; i++) {
    pinMode(rowPins[i], INPUT_PULLUP);
  }
  pinMode(octaveUpPin, INPUT_PULLUP);
  pinMode(octaveDownPin, INPUT_PULLUP);
  pinMode(modeButtonPin, INPUT_PULLUP);
  Serial.println("   ✓ Pines configurados");
  
  // Test 2: Inicializar arrays
  Serial.println("2. Inicializando arrays...");
  for(int row = 0; row < NUM_ROWS; row++) {
    for(int col = 0; col < NUM_COLS; col++) {
      keyPressed[row][col] = false;
      lastPressTime[row][col] = 0;
      keyToMidiMap[row][col] = START_NOTE + (row * 12) + col;
      // Limitar a rango seguro
      if(keyToMidiMap[row][col] > 127) keyToMidiMap[row][col] = 127;
    }
  }
  Serial.println("   ✓ Arrays inicializados");
  
  // Test 3: Inicializar OLED
  Serial.println("3. Inicializando OLED...");
  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("   ✗ ERROR: OLED no detectado");
    while(1);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Test Diagnostico");
  display.println("Revisar Serial...");
  display.display();
  Serial.println("   ✓ OLED funcionando");
  
  // Test 4: Mostrar mapeo
  Serial.println("4. Mapeo de teclas (con octave=0):");
  for(int row = 0; row < NUM_ROWS; row++) {
    Serial.print("   Fila ");
    Serial.print(row);
    Serial.print(": ");
    for(int col = 0; col < NUM_COLS; col++) {
      int finalNote = keyToMidiMap[row][col] + currentOctaveShift * 12;
      Serial.print(finalNote);
      Serial.print(" ");
    }
    Serial.println();
  }
  
  // Test 5: Test de shift registers
  Serial.println("5. Test de shift registers...");
  for(int col = 0; col < NUM_COLS; col++) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, B11111111);
    shiftOut(dataPin, clockPin, MSBFIRST, bits[col]);
    digitalWrite(latchPin, HIGH);
    delay(10);
  }
  Serial.println("   ✓ Shift registers testeados");
  
  // Test 6: Test MIDI
  Serial.println("6. Test MIDI básico...");
  midiEventPacket_t testNote = {0x09, 0x90, 60, 100};  // C4
  MidiUSB.sendMIDI(testNote);
  MidiUSB.flush();
  delay(100);
  midiEventPacket_t testOff = {0x08, 0x80, 60, 0};
  MidiUSB.sendMIDI(testOff);
  MidiUSB.flush();
  Serial.println("   ✓ MIDI enviado (C4 test)");
  
  Serial.println("=== DIAGNÓSTICO COMPLETO ===");
  Serial.println("Sistema listo. Presiona teclas...");
  Serial.println("==============================");
}

void loop() {
  // Test simple de botones
  static unsigned long lastButtonTest = 0;
  if(millis() - lastButtonTest > 1000) {
    lastButtonTest = millis();
    Serial.print("Botones - UP:");
    Serial.print(digitalRead(octaveUpPin) ? "0" : "1");
    Serial.print(" DN:");
    Serial.print(digitalRead(octaveDownPin) ? "0" : "1");
    Serial.print(" MODE:");
    Serial.println(digitalRead(modeButtonPin) ? "0" : "1");
  }
  
  // Scan simplificado
  for(int col = 0; col < NUM_COLS; col++) {
    // Activar columna
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, B11111111);
    shiftOut(dataPin, clockPin, MSBFIRST, bits[col]);
    digitalWrite(latchPin, HIGH);
    
    // Leer todas las filas
    for(int row = 0; row < NUM_ROWS; row++) {
      bool currentState = !digitalRead(rowPins[row]);
      unsigned long now = millis();
      
      if(currentState != keyPressed[row][col] && 
         now - lastPressTime[row][col] > DEBOUNCE_DELAY) {
        
        keyPressed[row][col] = currentState;
        lastPressTime[row][col] = now;
        
        if(currentState) {
          // Tecla presionada
          int midiNote = keyToMidiMap[row][col] + currentOctaveShift * 12;
          if(midiNote >= 0 && midiNote <= 127) {
            lastPlayedNote = midiNote;
            
            // Enviar MIDI
            midiEventPacket_t noteOn = {0x09, (uint8_t)(0x90 | midiChannel), (uint8_t)midiNote, currentVelocity};
            MidiUSB.sendMIDI(noteOn);
            MidiUSB.flush();
            
            Serial.print("TECLA PRESIONADA - Row:");
            Serial.print(row);
            Serial.print(" Col:");
            Serial.print(col);
            Serial.print(" Base:");
            Serial.print(keyToMidiMap[row][col]);
            Serial.print(" Final:");
            Serial.print(midiNote);
            Serial.print(" Ch:");
            Serial.print(midiChannel);
            Serial.print(" Vel:");
            Serial.println(currentVelocity);
            
            // Actualizar pantalla
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(10, 20);
            display.print("NOTA: ");
            display.print(midiNote);
            display.setTextSize(1);
            display.setCursor(10, 40);
            display.print("Row:");
            display.print(row);
            display.print(" Col:");
            display.print(col);
            display.display();
            
          } else {
            Serial.print("ERROR: Nota fuera de rango - ");
            Serial.println(midiNote);
          }
        } else {
          // Tecla liberada
          if(lastPlayedNote >= 0) {
            midiEventPacket_t noteOff = {0x08, (uint8_t)(0x80 | midiChannel), (uint8_t)lastPlayedNote, 64};
            MidiUSB.sendMIDI(noteOff);
            MidiUSB.flush();
            Serial.print("TECLA LIBERADA - Nota:");
            Serial.println(lastPlayedNote);
          }
        }
      }
    }
  }
  
  delay(1);
}

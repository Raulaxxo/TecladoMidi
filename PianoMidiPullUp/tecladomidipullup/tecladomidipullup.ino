#include "MIDIUSB.h"

// ---------------- Configuraciones ----------------
#define NUM_ROWS 6
#define NUM_COLS 9
#define START_NOTE 31
#define NOTE_VELOCITY 127
#define SERIAL_RATE 31250
#define DEBOUNCE_DELAY 5 // ms

// Salidas Serial
bool SERIAL_ENABLED = true;      // Activa/desactiva salida Serial normal
bool PLOTTER_ENABLED = true;     // Activa/desactiva salida Serial Plotter

// Canal MIDI (0-15)
byte midiChannel = 0;

// Pins del teclado
const int rowPins[NUM_ROWS] = {2,3,4,5,6,7};
const int dataPin = 11;
const int latchPin = 12;
const int clockPin = 13;

// Pines de cambio de octava
const int octaveUpPin = 8;
const int octaveDownPin = 9;

// Variables
boolean keyPressed[NUM_ROWS][NUM_COLS];
uint8_t keyToMidiMap[NUM_ROWS][NUM_COLS];
unsigned long lastPressTime[NUM_ROWS][NUM_COLS];
int currentOctaveShift = 0;

// Bitmasks para columnas
int bits[] = { 
  B11111110, B11111101, B11111011, B11110111,
  B11101111, B11011111, B10111111, B01111111, B11111111
};

// Nombres de notas con sostenidas ♯
const char* noteNames[12] = { "Do", "Do♯", "Re", "Re♯", "Mi", "Fa", "Fa♯", "Sol", "Sol♯", "La", "La♯", "Si" };

void setup() {
  // Inicializar mapeo de notas
  int note = START_NOTE;
  for(int col=0; col<NUM_COLS; ++col)
    for(int row=0; row<NUM_ROWS; ++row)
      keyToMidiMap[row][col] = note++;

  // Configurar pines
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  for(int i=0; i<NUM_ROWS; i++)
    pinMode(rowPins[i], INPUT_PULLUP);

  pinMode(octaveUpPin, INPUT_PULLUP);
  pinMode(octaveDownPin, INPUT_PULLUP);

  Serial.begin(SERIAL_RATE);

  // Mapa de teclas para referencia
  if(SERIAL_ENABLED || PLOTTER_ENABLED) {
    Serial.println("Mapa de teclas (Serial Plotter):");
    int count = 0;
    for(int row=0; row<NUM_ROWS; row++)
      for(int col=0; col<NUM_COLS; col++) {
        byte midiNote = keyToMidiMap[row][col];
        const char* noteName = noteNames[midiNote % 12];
        int octave = (midiNote / 12) - 1;
        Serial.print("Linea "); Serial.print(count++);
        Serial.print(": "); Serial.print(noteName); Serial.println(octave);
        lastPressTime[row][col] = 0;
      }
  }
}

void loop() {
  // Leer botones de octava
  if(!digitalRead(octaveUpPin))  { currentOctaveShift++; delay(200); }
  if(!digitalRead(octaveDownPin)) { currentOctaveShift--; delay(200); }
  if(currentOctaveShift < -2) currentOctaveShift = -2;
  if(currentOctaveShift > 2) currentOctaveShift = 2;

  // Escanear columnas y actualizar teclas
  for(int col=0; col<NUM_COLS; col++) {
    scanColumn(col);

    int rowValue[NUM_ROWS];
    for(int r=0; r<NUM_ROWS; r++)
      rowValue[r] = !digitalRead(rowPins[r]);

    for(int row=0; row<NUM_ROWS; row++) {
      unsigned long now = millis();
      if(rowValue[row] && !keyPressed[row][col] && now - lastPressTime[row][col] > DEBOUNCE_DELAY) {
        keyPressed[row][col] = true;
        lastPressTime[row][col] = now;
        noteOn(row,col);
      } 
      else if(!rowValue[row] && keyPressed[row][col] && now - lastPressTime[row][col] > DEBOUNCE_DELAY) {
        keyPressed[row][col] = false;
        lastPressTime[row][col] = now;
        noteOff(row,col);
      }
    }
  }

  // Enviar datos al Serial Plotter
  if(PLOTTER_ENABLED) {
    for(int row=0; row<NUM_ROWS; row++)
      for(int col=0; col<NUM_COLS; col++) {
        Serial.print(keyPressed[row][col] ? 1 : 0);
        if(col < NUM_COLS-1 || row < NUM_ROWS-1) Serial.print(",");
      }
    Serial.println();
  }
}

void scanColumn(int colNum) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, B11111111);
  shiftOut(dataPin, clockPin, MSBFIRST, bits[colNum]);
  digitalWrite(latchPin, HIGH);
}

void noteOn(int row, int col) {
  byte midiNote = keyToMidiMap[row][col] + currentOctaveShift * 12;
  const char* noteName = noteNames[midiNote % 12];
  int octave = (midiNote / 12) - 1;

  if(SERIAL_ENABLED) {
    Serial.print("Note ON: "); 
    Serial.print(noteName); Serial.print(octave);
    Serial.print(" ("); Serial.print(midiNote); 
    Serial.print(") - Row: "); Serial.print(row); 
    Serial.print(", Col: "); Serial.println(col);
  }

  noteOnMIDI(midiChannel, midiNote, NOTE_VELOCITY);
  MidiUSB.flush();
}

void noteOff(int row, int col) {
  byte midiNote = keyToMidiMap[row][col] + currentOctaveShift * 12;
  const char* noteName = noteNames[midiNote % 12];
  int octave = (midiNote / 12) - 1;

  if(SERIAL_ENABLED) {
    Serial.print("Note OFF: "); 
    Serial.print(noteName); Serial.print(octave);
    Serial.print(" ("); Serial.print(midiNote); 
    Serial.print(") - Row: "); Serial.print(row); 
    Serial.print(", Col: "); Serial.println(col);
  }

  noteOffMIDI(midiChannel, midiNote, NOTE_VELOCITY);
  MidiUSB.flush();
}

void noteOnMIDI(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, (uint8_t)(0x90 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOffMIDI(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, (uint8_t)(0x80 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, (uint8_t)(0xB0 | channel), control, value};
  MidiUSB.sendMIDI(event);
}

#include "MIDIUSB.h"

// ===== CONFIGURACIÓN DEL TECLADO =====
// Modifica estos valores según tu teclado
#define NUM_ROWS 6
#define NUM_COLS 9

// Configuración de notas MIDI
#define FIRST_MIDI_NOTE 31    // Primera nota MIDI del teclado (nota más grave)

// Pin Definitions - MODIFICA SEGÚN TU HARDWARE
// Row input pins (conecta tus filas a estos pines consecutivos)
const int rowPins[NUM_ROWS] = {4, 5, 6, 7, 8, 9}; // Pines de filas

// 74HC595 pins para shift register
const int dataPin = 10;
const int latchPin = 11;
const int clockPin = 12;

// ===== CONFIGURACIÓN AVANZADA =====
#define NOTE_ON_CMD 0x90
#define NOTE_OFF_CMD 0x80
#define NOTE_VELOCITY 127

//MIDI baud rate
#define SERIAL_RATE 31250

boolean keyPressed[NUM_ROWS][NUM_COLS];
uint8_t keyToMidiMap[NUM_ROWS][NUM_COLS];

// bitmasks for scanning columns - PullUp version (se generan automáticamente)
int bits[NUM_COLS];

// Función para generar bitmasks automáticamente para PullUp
void generateBitmasks() {
  for(int i = 0; i < NUM_COLS && i < 8; i++) {
    bits[i] = ~(1 << i); // B11111110, B11111101, B11111011, etc. (invertidos para PullUp)
  }
  if(NUM_COLS <= 8) {
    bits[NUM_COLS] = B11111111; // Estado inactivo para columnas no usadas
  }
  // Si tienes más de 8 columnas, necesitarás lógica adicional para múltiples shift registers
}

void setup()
{
  Serial.begin(SERIAL_RATE);
  
  // Generar bitmasks automáticamente según configuración PullUp
  generateBitmasks();
  
  // Inicializar mapeo de notas MIDI automáticamente
  int note = FIRST_MIDI_NOTE;

  for(int colCtr = 0; colCtr < NUM_COLS; ++colCtr)
  {
    for(int rowCtr = 0; rowCtr < NUM_ROWS; ++rowCtr)
    {
      keyPressed[rowCtr][colCtr] = false;
      keyToMidiMap[rowCtr][colCtr] = note;
      note++;
    }
  }

  // setup pins output/input mode
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  // Configurar pines de filas automáticamente con pull-up
  for(int i = 0; i < NUM_ROWS; i++) {
    pinMode(rowPins[i], INPUT_PULLUP);
  }

  Serial.print("Sistema iniciado - Teclado PullUp ");
  Serial.print(NUM_ROWS);
  Serial.print("x");
  Serial.print(NUM_COLS);
  Serial.print(" (");
  Serial.print(NUM_ROWS * NUM_COLS);
  Serial.println(" teclas)");
  Serial.print("Rango MIDI: ");
  Serial.print(FIRST_MIDI_NOTE);
  Serial.print(" - ");
  Serial.println(FIRST_MIDI_NOTE + (NUM_ROWS * NUM_COLS) - 1);
}

void loop()
{
  for (int colCtr = 0; colCtr < NUM_COLS; ++colCtr)
  {
    //scan next column
    scanColumn(colCtr);

    //get row values at this column - dinámico según NUM_ROWS
    int rowValue[NUM_ROWS];
    for(int i = 0; i < NUM_ROWS; i++) {
      rowValue[i] = !digitalRead(rowPins[i]); // Invertido para PullUp
    }

    // process keys pressed
    for(int rowCtr=0; rowCtr<NUM_ROWS; ++rowCtr)
    {
      if(rowValue[rowCtr] != 0 && !keyPressed[rowCtr][colCtr])
      {
        keyPressed[rowCtr][colCtr] = true;
        noteOn(rowCtr,colCtr);
      }
    }

    // process keys released
    for(int rowCtr=0; rowCtr<NUM_ROWS; ++rowCtr)
    {
      if(rowValue[rowCtr] == 0 && keyPressed[rowCtr][colCtr])
      {
        keyPressed[rowCtr][colCtr] = false;
        noteOff(rowCtr,colCtr);
      }
    }
  }
}

void scanColumn(int colNum)
{
  digitalWrite(latchPin, LOW);

  // Soporte para hasta 16 columnas usando 2 shift registers - PullUp version
  if(NUM_COLS <= 8) {
    // Configuración simple: un solo shift register
    if(colNum < NUM_COLS) {
      shiftOut(dataPin, clockPin, MSBFIRST, bits[colNum]);
    } else {
      shiftOut(dataPin, clockPin, MSBFIRST, B11111111); // Estado inactivo para PullUp
    }
  } else {
    // Configuración dual: dos shift registers
    if(0 <= colNum && colNum <= 7)
    {
      shiftOut(dataPin, clockPin, MSBFIRST, B11111111); //right sr - inactivo
      shiftOut(dataPin, clockPin, MSBFIRST, bits[colNum]); //left sr
    }
    else if(colNum < NUM_COLS)
    {
      shiftOut(dataPin, clockPin, MSBFIRST, bits[colNum-8]); //right sr
      shiftOut(dataPin, clockPin, MSBFIRST, B11111111); //left sr - inactivo
    }
  }
  
  digitalWrite(latchPin, HIGH);
}

void noteOn(int row, int col)
{
  noteOnMIDI(0, keyToMidiMap[row][col], NOTE_VELOCITY);
  MidiUSB.flush();
}

void noteOff(int row, int col)
{
  noteOffMIDI(0, keyToMidiMap[row][col], NOTE_VELOCITY);
  MidiUSB.flush();
}

void noteOnMIDI(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  
}

void noteOffMIDI(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
 
  
  
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}





#include "MIDIUSB.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define NUM_ROWS 6
#define NUM_COLS 9

#define NOTE_ON_CMD 0x90
#define NOTE_OFF_CMD 0x80
#define NOTE_VELOCITY 127

//MIDI baud rate
#define SERIAL_RATE 31250

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pin Definitions
// Row input pins (teclado conectado de pin 3 al 9)
const int row1Pin = 4;
const int row2Pin = 5;
const int row3Pin = 6;
const int row4Pin = 7;
const int row5Pin = 8;
const int row6Pin = 9;

// 74HC595 pins (data pin al clock pin: 10, 11, 12)
const int dataPin = 10;
const int latchPin = 11;
const int clockPin = 12;

// Botones
const int btn1Pin = 13;  // Botón 1
const int btn2Pin = A5;  // Botón 2  
const int btn3Pin = A1;  // Botón 3

boolean keyPressed[NUM_ROWS][NUM_COLS];
uint8_t keyToMidiMap[NUM_ROWS][NUM_COLS];

// Variables para la pantalla
bool oledWorking = false;
int lastPlayedNote = -1;
int notesPlayedCount = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long noteDisplayTime = 0; // Para mantener la nota visible por un tiempo

// Variables para botones
int currentOctave = 0;  // Octava actual (-2 a +2)
int currentMode = 0;    // Modo actual (0=Piano, 1=Info, 2=Grabar/Reproducir)
unsigned long lastButtonPress[3] = {0, 0, 0}; // Debounce para los 3 botones
#define BUTTON_DEBOUNCE 300

// Variables para grabación y reproducción
#define MAX_RECORDED_NOTES 100
struct RecordedNote {
  uint8_t note;
  unsigned long timestamp;
  bool isNoteOn;
};
RecordedNote recordedSequence[MAX_RECORDED_NOTES];
int recordedCount = 0;
bool isRecording = false;
bool isPlaying = false;
unsigned long recordStartTime = 0;
unsigned long playStartTime = 0;
int currentPlayIndex = 0;

// bitmasks for scanning columns
int bits[] =
{ 
  B11111110,
  B11111101,
  B11111011,
  B11110111,
  B11101111,
  B11011111,
  B10111111,
  B01111111,
  B11111111
};

void setup()
{
  Serial.begin(SERIAL_RATE);
  
  // Inicializar OLED
  Wire.begin();
  delay(100);
  
  if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oledWorking = true;
    Serial.println("OLED inicializado correctamente");
    
    // Pantalla de bienvenida
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(25, 10);
    display.println("MIDI");
    display.setCursor(15, 35);
    display.println("PIANO");
    display.display();
    delay(2000);
  } else {
    oledWorking = false;
    Serial.println("Error inicializando OLED");
  }

  int note = 31;

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

  pinMode(row1Pin, INPUT_PULLUP);
  pinMode(row2Pin, INPUT_PULLUP);
  pinMode(row3Pin, INPUT_PULLUP);
  pinMode(row4Pin, INPUT_PULLUP);
  pinMode(row5Pin, INPUT_PULLUP);
  pinMode(row6Pin, INPUT_PULLUP);
  
  // Configurar botones
  pinMode(btn1Pin, INPUT_PULLUP);
  pinMode(btn2Pin, INPUT_PULLUP);
  pinMode(btn3Pin, INPUT_PULLUP);
  
  Serial.println("Sistema MIDI con OLED listo!");
}

void loop()
{
  for (int colCtr = 0; colCtr < NUM_COLS; ++colCtr)
  {
    //scan next column
    scanColumn(colCtr);

    //get row values at this column
    int rowValue[NUM_ROWS];
    rowValue[0] = !digitalRead(row1Pin);
    rowValue[1] = !digitalRead(row2Pin);
    rowValue[2] = !digitalRead(row3Pin);
    rowValue[3] = !digitalRead(row4Pin);
    rowValue[4] = !digitalRead(row5Pin);
    rowValue[5] = !digitalRead(row6Pin);

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
  
  // Manejar botones
  handleButtons();
  
  // Actualizar pantalla cada 500ms
  unsigned long now = millis();
  if(oledWorking && (now - lastDisplayUpdate > 500)) {
    lastDisplayUpdate = now;
    updateDisplay();
  }
}

void scanColumn(int colNum)
{
  digitalWrite(latchPin, LOW);

  if(0 <= colNum && colNum <= 9)
  {
    shiftOut(dataPin, clockPin, MSBFIRST, B11111111); //right sr
    shiftOut(dataPin, clockPin, MSBFIRST, bits[colNum]); //left sr
  }
  else
  {
    shiftOut(dataPin, clockPin, MSBFIRST, bits[colNum-8]); //right sr
    shiftOut(dataPin, clockPin, MSBFIRST, B11111111); //left sr
  }
  digitalWrite(latchPin, HIGH);
}

void noteOn(int row, int col)
{
  // Aplicar cambio de octava
  int midiNote = keyToMidiMap[row][col] + (currentOctave * 12);
  
  // Limitar rango MIDI válido (0-127)
  if(midiNote < 0) midiNote = 0;
  if(midiNote > 127) midiNote = 127;
  
  Serial.write(NOTE_ON_CMD);
  Serial.write(midiNote);
  Serial.write(NOTE_VELOCITY);
  noteOnMIDI(0, midiNote, NOTE_VELOCITY);
  MidiUSB.flush();
  
  // Actualizar variables para la pantalla
  lastPlayedNote = midiNote;
  noteDisplayTime = millis(); // Marcar cuando se tocó la nota
  notesPlayedCount++;
}

void noteOff(int row, int col)
{
  // Aplicar cambio de octava
  int midiNote = keyToMidiMap[row][col] + (currentOctave * 12);
  
  // Limitar rango MIDI válido (0-127)
  if(midiNote < 0) midiNote = 0;
  if(midiNote > 127) midiNote = 127;
  
  noteOffMIDI(0, midiNote, NOTE_VELOCITY);
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

void handleButtons() {
  unsigned long now = millis();
  
  // Botón 1 (Pin 13) - Subir octava
  if(!digitalRead(btn1Pin) && (now - lastButtonPress[0] > BUTTON_DEBOUNCE)) {
    lastButtonPress[0] = now;
    currentOctave++;
    if(currentOctave > 2) currentOctave = 2; // Límite +2 octavas
    Serial.print("Octava: ");
    Serial.println(currentOctave);
  }
  
  // Botón 2 (Pin A5) - Bajar octava
  if(!digitalRead(btn2Pin) && (now - lastButtonPress[1] > BUTTON_DEBOUNCE)) {
    lastButtonPress[1] = now;
    currentOctave--;
    if(currentOctave < -2) currentOctave = -2; // Límite -2 octavas
    Serial.print("Octava: ");
    Serial.println(currentOctave);
  }
  
  // Botón 3 (Pin A1) - Cambiar modo de display
  if(!digitalRead(btn3Pin) && (now - lastButtonPress[2] > BUTTON_DEBOUNCE)) {
    lastButtonPress[2] = now;
    currentMode++;
    if(currentMode > 1) currentMode = 0; // 2 modos: 0, 1
    Serial.print("Modo display: ");
    Serial.println(currentMode);
  }
}

void updateDisplay() {
  if(!oledWorking) return;
  
  display.clearDisplay();
  
  // Dibujar borde para todos los modos
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  
  // Mostrar información según el modo
  switch(currentMode) {
    case 0: // Modo Piano - Dibujar teclado
      drawPianoKeyboard();
      break;
      
    case 1: // Modo Info
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(3, 3);
      display.println("MODO: INFO");
      display.drawLine(3, 13, 124, 13, SSD1306_WHITE);
      
      display.setCursor(3, 18);
      display.print("Tiempo: ");
      display.print(millis() / 1000);
      display.print("s");
      
      display.setCursor(3, 28);
      display.print("Total notas: ");
      display.println(notesPlayedCount);
      
      display.setCursor(3, 38);
      display.print("MIDI Canal: 1");
      
      display.setCursor(3, 48);
      display.print("Octava: ");
      if(currentOctave > 0) display.print("+");
      display.println(currentOctave);
      
      display.setCursor(3, 58);
      display.print("Estado: ACTIVO");
      break;
      
    case 2: // Modo Config
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(3, 3);
      display.println("MODO: CONFIG");
      display.drawLine(3, 13, 124, 13, SSD1306_WHITE);
      
      display.setCursor(3, 18);
      display.print("Pines teclado: 3-8");
      
      display.setCursor(3, 28);
      display.print("Shift reg: 10-12");
      
      display.setCursor(3, 38);
      display.print("Botones: 13,A5,A1");
      
      display.setCursor(3, 48);
      display.print("OLED: SDA/SCL");
      
      display.setCursor(3, 58);
      display.print("Octava: ");
      display.println(currentOctave);
      break;
  }
  
  display.display();
}

void drawPianoKeyboard() {
  // Título simple (el borde ya se dibuja en updateDisplay)
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(3, 3);
  display.print("PIANO - Oct:");
  if(currentOctave > 0) display.print("+");
  display.print(currentOctave);
  
  // Mostrar nota tocada arriba del teclado
  if(lastPlayedNote >= 0 && (millis() - noteDisplayTime < 2000)) {
    display.setTextSize(2);
    display.setCursor(45, 12);
    display.println(getNoteNameLatin(lastPlayedNote));
  }
  
  // Dibujar teclado (octava completa - 7 teclas blancas)
  int keyWidth = 16;  // Ancho de cada tecla blanca (un poco más pequeño para el borde)
  int keyHeight = 32; // Alto de las teclas (un poco más pequeño)
  int startY = 28;    // Posición Y donde empieza el teclado (más abajo por el borde)
  int startX = 4;     // Posición X donde empieza el teclado (más a la derecha por el borde)
  
  // Teclas blancas (Do, Re, Mi, Fa, Sol, La, Si)
  for(int i = 0; i < 7; i++) {
    int x = startX + (i * keyWidth);
    
    // Dibujar contorno de la tecla
    display.drawRect(x, startY, keyWidth, keyHeight, SSD1306_WHITE);
    
    // Verificar si esta tecla está siendo tocada
    bool isPressed = false;
    if(lastPlayedNote >= 0 && (millis() - noteDisplayTime < 1000)) {
      int noteInOctave = lastPlayedNote % 12;
      // Mapear notas blancas: Do(0), Re(2), Mi(4), Fa(5), Sol(7), La(9), Si(11)
      int whiteNotes[] = {0, 2, 4, 5, 7, 9, 11};
      if(noteInOctave == whiteNotes[i]) {
        isPressed = true;
      }
    }
    
    // Si está presionada, rellenar la tecla
    if(isPressed) {
      display.fillRect(x+1, startY+1, keyWidth-2, keyHeight-2, SSD1306_WHITE);
    }
  }
  
  // Dibujar teclas negras (sostenidos)
  display.setTextColor(SSD1306_WHITE);
  int blackKeyWidth = 10;
  int blackKeyHeight = 20;
  
  // Posiciones de teclas negras ajustadas para el nuevo tamaño (Do#, Re#, Fa#, Sol#, La#)
  int blackKeyPositions[] = {12, 28, 60, 76, 92}; // Posiciones X ajustadas para teclas más pequeñas
  
  for(int i = 0; i < 5; i++) {
    bool isPressed = false;
    if(lastPlayedNote >= 0 && (millis() - noteDisplayTime < 1000)) {
      int noteInOctave = lastPlayedNote % 12;
      // Mapear notas negras: Do#(1), Re#(3), Fa#(6), Sol#(8), La#(10)
      int blackNotes[] = {1, 3, 6, 8, 10};
      if(noteInOctave == blackNotes[i]) {
        isPressed = true;
      }
    }
    
    if(isPressed) {
      // Tecla negra presionada - dibujar en blanco con contorno negro
      display.fillRect(blackKeyPositions[i], startY, blackKeyWidth, blackKeyHeight, SSD1306_WHITE);
      display.drawRect(blackKeyPositions[i], startY, blackKeyWidth, blackKeyHeight, SSD1306_BLACK);
    } else {
      // Tecla negra normal - rellenar en negro con contorno blanco
      display.fillRect(blackKeyPositions[i], startY, blackKeyWidth, blackKeyHeight, SSD1306_BLACK);
      display.drawRect(blackKeyPositions[i], startY, blackKeyWidth, blackKeyHeight, SSD1306_WHITE);
    }
  }
}

String getNoteName(int noteNumber) {
  const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  int octave = (noteNumber / 12) - 1;
  return String(noteNames[noteNumber % 12]) + String(octave);
}

String getNoteNameLatin(int noteNumber) {
  const char* noteNamesLatin[] = {"Do", "Do#", "Re", "Re#", "Mi", "Fa", "Fa#", "Sol", "Sol#", "La", "La#", "Si"};
  int octave = (noteNumber / 12) - 1;
  return String(noteNamesLatin[noteNumber % 12]) + String(octave);
}

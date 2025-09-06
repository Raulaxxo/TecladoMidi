#include "MIDIUSB.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== CONFIGURACIÓN DEL TECLADO =====
// Modifica estos valores según tu teclado
#define NUM_ROWS 6
#define NUM_COLS 9

// Configuración de notas MIDI
#define FIRST_MIDI_NOTE 31    // Primera nota MIDI del teclado (nota más grave)
#define OCTAVE_RANGE_LOW -2   // Octavas hacia abajo permitidas
#define OCTAVE_RANGE_HIGH 1   // Octavas hacia arriba permitidas

// Pin Definitions - MODIFICA SEGÚN TU HARDWARE
// Row input pins (conecta tus filas a estos pines consecutivos)
#define FIRST_ROW_PIN 4       // Primer pin de fila (las demás serán consecutivas)
const int rowPins[NUM_ROWS] = {4, 5, 6, 7, 8, 9}; // Pines de filas

// 74HC595 pins para shift register
const int dataPin = 10;
const int latchPin = 11;
const int clockPin = 12;

// Botones de control
const int btn1Pin = 13;  // Botón 1
const int btn2Pin = A5;  // Botón 2  
const int btn3Pin = A1;  // Botón 3

// Buzzer para metrónomo
const int buzzerPin = A4;  // Buzzer en A4

// ===== CONFIGURACIÓN AVANZADA =====
// Bitmasks para scanning - se generan automáticamente según NUM_COLS
// No modificar a menos que sepas lo que haces

#define NOTE_ON_CMD 0x90
#define NOTE_OFF_CMD 0x80
#define NOTE_VELOCITY 127

//MIDI baud rate
#define SERIAL_RATE 31250

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

boolean keyPressed[NUM_ROWS][NUM_COLS];
uint8_t keyToMidiMap[NUM_ROWS][NUM_COLS];

// Variables para la pantalla
bool oledWorking = false;
int lastPlayedNote = -1;
int notesPlayedCount = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long noteDisplayTime = 0; // Para mantener la nota visible por un tiempo

// Variables para botones
int currentOctave = 0;  // Octava actual (OCTAVE_RANGE_LOW a OCTAVE_RANGE_HIGH)
int currentMode = 0;    // Modo actual (0=Piano, 1=Metrónomo)
unsigned long lastButtonPress[3] = {0, 0, 0}; // Debounce para los 3 botones
#define BUTTON_DEBOUNCE 300

// Variables para metrónomo
int metronomeBPM = 120;  // BPM por defecto
bool metronomeActive = false;
unsigned long lastBeatTime = 0;
bool beatOn = false;
#define MIN_BPM 60
#define MAX_BPM 200

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
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print("PullUp ");
    display.print(NUM_ROWS);
    display.print("x");
    display.print(NUM_COLS);
    display.display();
    delay(2000);
  } else {
    oledWorking = false;
    Serial.println("Error inicializando OLED");
  }

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
  
  // Configurar botones
  pinMode(btn1Pin, INPUT_PULLUP);
  pinMode(btn2Pin, INPUT_PULLUP);
  pinMode(btn3Pin, INPUT_PULLUP);
  
  // Configurar buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  
  Serial.print("Sistema MIDI PullUp con OLED - Teclado ");
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
  
  // Manejar botones
  handleButtons();
  
  // Actualizar metrónomo si está activo
  if(metronomeActive) {
    updateMetronome();
  }
  
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
  // Aplicar cambio de octava
  int midiNote = keyToMidiMap[row][col] + (currentOctave * 12);
  
  // Limitar rango MIDI válido (0-127)
  if(midiNote < 0) midiNote = 0;
  if(midiNote > 127) midiNote = 127;
  
  // Enviar MIDI USB
  noteOnMIDI(0, midiNote, NOTE_VELOCITY);
  
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
  
  // Enviar MIDI USB
  noteOffMIDI(0, midiNote, NOTE_VELOCITY);
}

void noteOnMIDI(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
}

void noteOffMIDI(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void handleButtons() {
  unsigned long now = millis();
  
  if(currentMode == 0) { // Modo Piano - Control de octavas
    // Botón 1 (Pin 13) - Subir octava
    if(!digitalRead(btn1Pin) && (now - lastButtonPress[0] > BUTTON_DEBOUNCE)) {
      lastButtonPress[0] = now;
      currentOctave++;
      if(currentOctave > OCTAVE_RANGE_HIGH) currentOctave = OCTAVE_RANGE_HIGH; // Límite configurable
      Serial.print("Octava: ");
      Serial.println(currentOctave);
    }
    
    // Botón 2 (Pin A5) - Bajar octava
    if(!digitalRead(btn2Pin) && (now - lastButtonPress[1] > BUTTON_DEBOUNCE)) {
      lastButtonPress[1] = now;
      currentOctave--;
      if(currentOctave < OCTAVE_RANGE_LOW) currentOctave = OCTAVE_RANGE_LOW; // Límite configurable
      Serial.print("Octava: ");
      Serial.println(currentOctave);
    }
  } else if(currentMode == 1) { // Modo Metrónomo - Control de BPM y activación
    // Botón 1 (Pin 13) - Subir BPM
    if(!digitalRead(btn1Pin) && (now - lastButtonPress[0] > BUTTON_DEBOUNCE)) {
      lastButtonPress[0] = now;
      metronomeBPM += 5;
      if(metronomeBPM > MAX_BPM) metronomeBPM = MAX_BPM;
      Serial.print("BPM: ");
      Serial.println(metronomeBPM);
    }
    
    // Botón 2 (Pin A5) - Bajar BPM
    if(!digitalRead(btn2Pin) && (now - lastButtonPress[1] > BUTTON_DEBOUNCE)) {
      lastButtonPress[1] = now;
      metronomeBPM -= 5;
      if(metronomeBPM < MIN_BPM) metronomeBPM = MIN_BPM;
      Serial.print("BPM: ");
      Serial.println(metronomeBPM);
    }
    
    // Botón 3 en modo metrónomo - Activar/Desactivar metrónomo (presión larga)
    if(!digitalRead(btn3Pin) && (now - lastButtonPress[2] > BUTTON_DEBOUNCE * 2)) {
      metronomeActive = !metronomeActive;
      lastBeatTime = millis(); // Reiniciar timing
      beatOn = false;
      noTone(buzzerPin);
      lastButtonPress[2] = now;
      Serial.print("Metrónomo: ");
      Serial.println(metronomeActive ? "ACTIVO" : "INACTIVO");
      return; // Salir temprano para no cambiar de modo
    }
  }
  
  // Botón 3 (Pin A1) - Cambiar modo de display (presión corta)
  if(!digitalRead(btn3Pin) && (now - lastButtonPress[2] > BUTTON_DEBOUNCE)) {
    // Solo cambiar modo si no está en modo metrónomo activo
    if(currentMode == 1 && metronomeActive) {
      // En modo metrónomo activo, presión corta no hace nada
      return;
    }
    
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
      
    case 1: // Modo Metrónomo
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(3, 3);
      display.print("METRONOMO");
      display.drawLine(3, 13, 100, 13, SSD1306_WHITE);
      
      // Mostrar BPM grande
      display.setTextSize(2);
      display.setCursor(30, 18);
      display.print(metronomeBPM);
      
      display.setTextSize(1);
      display.setCursor(75, 25);
      display.print("BPM");
      
      // Estado del metrónomo
      display.setCursor(3, 38);
      display.print(metronomeActive ? "ON " : "OFF");
      
      // Indicador visual de beat
      if(metronomeActive && beatOn) {
        display.fillRect(90, 38, 8, 8, SSD1306_WHITE);
      } else if(metronomeActive) {
        display.drawRect(90, 38, 8, 8, SSD1306_WHITE);
      }
      
      // Mostrar última nota tocada
      if(lastPlayedNote >= 0 && (millis() - noteDisplayTime < 2000)) {
        display.setCursor(3, 48);
        display.print("Nota:");
        display.print(getNoteNameLatin(lastPlayedNote));
      }
      
      display.setCursor(3, 58);
      display.print("1/2:BPM 3:ON/OFF");
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

// Función del metrónomo
void updateMetronome() {
  if(!metronomeActive) {
    if(beatOn) {
      noTone(buzzerPin);
      beatOn = false;
    }
    return;
  }
  
  unsigned long currentTime = millis();
  unsigned long beatInterval = 60000 / metronomeBPM; // Intervalo en milisegundos
  
  // Tiempo del beat (sonido corto)
  const unsigned long beatDuration = 30; // 30ms de duración del beep
  
  if(currentTime - lastBeatTime >= beatInterval) {
    // Nuevo beat
    lastBeatTime = currentTime;
    beatOn = true;
    
    // Generar tono en el buzzer
    tone(buzzerPin, 800, beatDuration); // 800Hz por 30ms
    
    Serial.print("Beat! BPM: ");
    Serial.println(metronomeBPM);
  }
  
  // Apagar indicador visual después de un tiempo corto
  if(beatOn && (currentTime - lastBeatTime) > beatDuration) {
    beatOn = false;
  }
}

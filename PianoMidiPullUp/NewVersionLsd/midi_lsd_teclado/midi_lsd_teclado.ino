#include "MIDIUSB.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configuraciones básicas
#define NUM_ROWS 6
#define NUM_COLS 9
#define START_NOTE 31
#define NOTE_VELOCITY 127
#define DEBOUNCE_DELAY 20
#define BOTH_BUTTONS_TIME 500
#define AUTO_SLEEP_TIME 30000  // 30 segundos de inactividad

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
const int modeButtonPin = A0;  // Nuevo botón para cambio de modo

// Modos de funcionamiento
enum DisplayMode {
  MODE_PERFORMANCE = 0,
  MODE_CHORD,
  MODE_SCALE,
  MODE_OCTAVE,
  MODE_RECORDER,  // Nuevo modo de grabación/reproducción
  MODE_SETTINGS,
  MODE_TUNER,  // Nuevo modo
  MODE_COUNT
};

// Variables básicas optimizadas para aprendizaje
byte midiChannel = 0;
byte currentVelocity = 100; // Velocity moderado para aprendizaje
boolean keyPressed[NUM_ROWS][NUM_COLS];
uint8_t keyToMidiMap[NUM_ROWS][NUM_COLS];
unsigned long lastPressTime[NUM_ROWS][NUM_COLS];
unsigned long upButtonPressTime = 0;
unsigned long downButtonPressTime = 0;
unsigned long modeButtonPressTime = 0;  // Nuevo
bool upButtonPressed = false;
bool downButtonPressed = false;
bool modeButtonPressed = false;  // Nuevo
int currentOctaveShift = 1; // Octava +1 para rango medio cómodo
DisplayMode currentMode = MODE_PERFORMANCE;
boolean displayNeedsUpdate = true;
bool displaySleep = false;
unsigned long lastActivityTime = 0;

// Variables para análisis musical reducido
byte activeNotes[12]; // Cambio de int a byte
byte totalActiveNotes = 0;
unsigned long lastNoteTime = 0;
byte noteCount = 0; // Cambio de int a byte
unsigned long noteTimings[5]; // Reducido de 10 a 5
byte timingIndex = 0;

// Variables para afinador simplificado
int lastPlayedNote = -1;

// Variables para el modo grabador/reproductor
#define MAX_RECORDED_NOTES 20  // Máximo de notas que se pueden grabar
struct RecordedNote {
  byte note;
  unsigned long timestamp;
  bool isNoteOn;  // true = note on, false = note off
};
RecordedNote recordedSequence[MAX_RECORDED_NOTES];
byte recordedCount = 0;
bool isRecording = false;
bool isPlaying = false;
unsigned long recordStartTime = 0;
unsigned long playStartTime = 0;
byte playIndex = 0;

// Escalas predefinidas para aprendizaje
const byte scalePatterns[][8] PROGMEM = {
  {0, 2, 4, 5, 7, 9, 11, 12}, // Do Mayor
  {0, 2, 3, 5, 7, 8, 10, 12}, // Do menor
  {0, 2, 4, 7, 9, 12, 0, 0},  // Pentatónica
  {0, 1, 4, 5, 7, 8, 11, 12}  // Blues
};
const char scaleNames[][12] PROGMEM = {"Do Mayor", "Do menor", "Pentatonica", "Blues"};
byte currentScale = 0;

// Bits para shift registers
int bits[] = { 
  B11111110, B11111101, B11111011, B11110111,
  B11101111, B11011111, B10111111, B01111111, B11111111
};

// Nombres de notas y escalas en PROGMEM para ahorrar RAM
const char noteNames0[] PROGMEM = "Do";
const char noteNames1[] PROGMEM = "Do#";
const char noteNames2[] PROGMEM = "Re";
const char noteNames3[] PROGMEM = "Re#";
const char noteNames4[] PROGMEM = "Mi";
const char noteNames5[] PROGMEM = "Fa";
const char noteNames6[] PROGMEM = "Fa#";
const char noteNames7[] PROGMEM = "Sol";
const char noteNames8[] PROGMEM = "Sol#";
const char noteNames9[] PROGMEM = "La";
const char noteNames10[] PROGMEM = "La#";
const char noteNames11[] PROGMEM = "Si";

const char* const noteNames[12] PROGMEM = {
  noteNames0, noteNames1, noteNames2, noteNames3, noteNames4, noteNames5,
  noteNames6, noteNames7, noteNames8, noteNames9, noteNames10, noteNames11
};

const char modeNames0[] PROGMEM = "PERF";
const char modeNames1[] PROGMEM = "ACORD";
const char modeNames2[] PROGMEM = "ESCALA";
const char modeNames3[] PROGMEM = "OCTAV";
const char modeNames4[] PROGMEM = "VEL";
const char modeNames5[] PROGMEM = "CONFIG";
const char modeNames6[] PROGMEM = "DEBUG"; // Cambiado de AFIN a DEBUG

const char* const modeNames[7] PROGMEM = {
  modeNames0, modeNames1, modeNames2, modeNames3, modeNames4, modeNames5, modeNames6
};

void setup() {
  // Inicializar matrices
  for(int row = 0; row < NUM_ROWS; row++) {
    for(int col = 0; col < NUM_COLS; col++) {
      keyPressed[row][col] = false;
      lastPressTime[row][col] = 0;
    }
  }
  
  // Inicializar arrays de notas activas
  for(int i = 0; i < 12; i++) {
    activeNotes[i] = 0;
  }
  for(int i = 0; i < 5; i++) {
    noteTimings[i] = 0;
  }

  // Mapeo de notas mejorado - disposición cromática
  mapNotesToKeys();
  
  // Diagnóstico inicial
  Serial.println("=== DIAGNÓSTICO INICIAL ===");
  Serial.print("NUM_ROWS: ");
  Serial.println(NUM_ROWS);
  Serial.print("NUM_COLS: ");
  Serial.println(NUM_COLS);
  Serial.print("START_NOTE: ");
  Serial.println(START_NOTE);
  Serial.print("Current Octave Shift: ");
  Serial.println(currentOctaveShift);
  Serial.print("Current Velocity: ");
  Serial.println(currentVelocity);
  Serial.print("MIDI Channel: ");
  Serial.println(midiChannel);
  Serial.println("========================");

  // Configurar pines
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  for(int i = 0; i < NUM_ROWS; i++) {
    pinMode(rowPins[i], INPUT_PULLUP);
  }
  pinMode(octaveUpPin, INPUT_PULLUP);
  pinMode(octaveDownPin, INPUT_PULLUP);
  pinMode(modeButtonPin, INPUT_PULLUP);  // Configurar nuevo botón

  Serial.begin(115200); // Cambiado para debug USB
  Wire.begin();

  // Inicializar OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;);
  }
  
  showSplashScreen();
  lastActivityTime = millis();
}

void loop() {
  unsigned long now = millis();
  
  // Manejo de sleep automático
  if(!displaySleep && (now - lastActivityTime > AUTO_SLEEP_TIME)) {
    displaySleep = true;
    display.clearDisplay();
    display.display();
  }
  
  handleButtons();
  
  // Manejo de reproducción de secuencia grabada
  if(isPlaying && playIndex < recordedCount) {
    unsigned long playTime = now - playStartTime;
    if(playTime >= recordedSequence[playIndex].timestamp) {
      byte note = recordedSequence[playIndex].note;
      if(recordedSequence[playIndex].isNoteOn) {
        // Reproducir note on
        midiEventPacket_t noteOn = {0x09, (uint8_t)(0x90 | midiChannel), note, currentVelocity};
        MidiUSB.sendMIDI(noteOn);
        lastPlayedNote = note; // Para mostrar en pantalla
        activeNotes[note % 12] = 1; // Para visualización
      } else {
        // Reproducir note off
        midiEventPacket_t noteOff = {0x08, (uint8_t)(0x80 | midiChannel), note, 64};
        MidiUSB.sendMIDI(noteOff);
        activeNotes[note % 12] = 0; // Para visualización
      }
      MidiUSB.flush();
      playIndex++;
      displayNeedsUpdate = true;
    }
    
    // Fin de reproducción
    if(playIndex >= recordedCount) {
      isPlaying = false;
      displayNeedsUpdate = true;
    }
  }
  
  bool anyKeyPressed = scanKeyboard();
  updateDisplay(anyKeyPressed);
  
  // Pequeña pausa para no saturar
  delay(1);
}

void mapNotesToKeys() {
  // Mapeo mejorado: disposición más musical
  // Filas como octavas, columnas como semitonos
  for(int row = 0; row < NUM_ROWS; row++) {
    for(int col = 0; col < NUM_COLS; col++) {
      keyToMidiMap[row][col] = START_NOTE + (row * 12) + col;
      
      // Asegurar que el mapeo base esté en rango razonable
      // Con octave shift de -3 a +3, necesitamos rango seguro
      if(keyToMidiMap[row][col] > 91) { // 91 + 3*12 = 127 (máximo MIDI)
        keyToMidiMap[row][col] = 91; // Limitar para permitir octave shift
      }
      if(keyToMidiMap[row][col] < 36) { // 36 - 3*12 = 0 (mínimo MIDI)
        keyToMidiMap[row][col] = 36; // Limitar para permitir octave shift
      }
    }
  }
  
  // Debug: Mostrar mapeo completo en Serial
  Serial.println("=== MAPEO DE TECLAS ===");
  for(int row = 0; row < NUM_ROWS; row++) {
    Serial.print("Fila ");
    Serial.print(row);
    Serial.print(": ");
    for(int col = 0; col < NUM_COLS; col++) {
      Serial.print(keyToMidiMap[row][col]);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.print("Octave Shift actual: ");
  Serial.println(currentOctaveShift);
  Serial.println("====================");
}

void showSplashScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(15, 10);
  display.println(F("Raulaxxo"));
  display.setTextSize(1);
  display.setCursor(10, 30);
  display.println(F("Teclado Educativo"));
  display.setCursor(15, 40);
  display.println(F("MIDI v2.1 - Do Re Mi"));
  display.setCursor(5, 50);
  display.println(F("Inicializando MIDI..."));
  display.display();
  
  // Reinicializar MIDI USB múltiples veces
  for(int i = 0; i < 3; i++) {
    MidiUSB.flush();
    delay(100);
  }
  
  // Test MIDI múltiple para forzar detección
  for(int i = 0; i < 5; i++) {
    midiEventPacket_t testNote = {0x09, 0x90, 60, 100};
    MidiUSB.sendMIDI(testNote);
    MidiUSB.flush();
    delay(50);
    
    midiEventPacket_t testOff = {0x08, 0x80, 60, 0};
    MidiUSB.sendMIDI(testOff);
    MidiUSB.flush();
    delay(50);
  }
  
  display.setCursor(20, 60);
  display.println(F("MIDI Listo!"));
  display.display();
  delay(1000);
  displayNeedsUpdate = true;
}

void handleButtons() {
  bool currentUpState = !digitalRead(octaveUpPin);
  bool currentDownState = !digitalRead(octaveDownPin);
  bool currentModeState = !digitalRead(modeButtonPin);  // Nuevo botón
  unsigned long now = millis();
  
  if(displaySleep && (currentUpState || currentDownState || currentModeState)) {
    displaySleep = false;
    lastActivityTime = now;
    displayNeedsUpdate = true;
    return;
  }
  
  // ========== BOTÓN DE MODO (Nuevo y simplificado) ==========
  if(currentModeState && !modeButtonPressed) {
    modeButtonPressed = true;
    modeButtonPressTime = now;
    lastActivityTime = now;
  }
  
  if(!currentModeState && modeButtonPressed) {
    modeButtonPressed = false;
    unsigned long pressDuration = now - modeButtonPressTime;
    
    if(pressDuration > 50 && pressDuration < 2000) {
      // Cambiar al siguiente modo
      currentMode = (DisplayMode)((currentMode + 1) % MODE_COUNT);
      displayNeedsUpdate = true;
    }
    modeButtonPressTime = 0;
  }
  
  // ========== BOTÓN UP (Funciones específicas del modo) ==========
  if(currentUpState && !upButtonPressed) {
    upButtonPressed = true;
    upButtonPressTime = now;
    lastActivityTime = now;
  }
  
  if(!currentUpState && upButtonPressed) {
    upButtonPressed = false;
    unsigned long pressDuration = now - upButtonPressTime;
    
    if(pressDuration > 50 && pressDuration < 2000) {
      handleUpButton();
    }
    upButtonPressTime = 0;
  }
  
  // ========== BOTÓN DOWN (Funciones específicas del modo) ==========
  if(currentDownState && !downButtonPressed) {
    downButtonPressed = true;
    downButtonPressTime = now;
    lastActivityTime = now;
  }
  
  if(!currentDownState && downButtonPressed) {
    downButtonPressed = false;
    unsigned long pressDuration = now - downButtonPressTime;
    
    if(pressDuration > 50 && pressDuration < 2000) {
      handleDownButton();
    }
    downButtonPressTime = 0;
  }
}

void handleUpButton() {
  switch(currentMode) {
    case MODE_PERFORMANCE:
    case MODE_CHORD:
    case MODE_TUNER:
    case MODE_OCTAVE:  // Agregado explícitamente
      currentOctaveShift++;
      if(currentOctaveShift > 3) currentOctaveShift = 3;
      break;
    case MODE_SCALE:
      currentScale = (currentScale + 1) % 4; // Cambiar escala
      break;
    case MODE_RECORDER:
      // UP: Iniciar/Parar grabación
      if(!isRecording && !isPlaying) {
        isRecording = true;
        recordedCount = 0;
        recordStartTime = millis();
      } else if(isRecording) {
        isRecording = false;
      }
      break;
    case MODE_SETTINGS:
      midiChannel++;
      if(midiChannel > 15) midiChannel = 0;
      break;
  }
  displayNeedsUpdate = true;
}

void handleDownButton() {
  switch(currentMode) {
    case MODE_PERFORMANCE:
    case MODE_CHORD:
    case MODE_TUNER:
    case MODE_OCTAVE:  // Agregado explícitamente
      currentOctaveShift--;
      if(currentOctaveShift < -3) currentOctaveShift = -3;
      break;
    case MODE_SCALE:
      currentScale = (currentScale + 3) % 4; // Cambiar escala hacia atrás
      break;
    case MODE_RECORDER:
      // DOWN: Reproducir/Parar reproducción
      if(!isRecording && !isPlaying && recordedCount > 0) {
        isPlaying = true;
        playStartTime = millis();
        playIndex = 0;
      } else if(isPlaying) {
        isPlaying = false;
        // Parar todas las notas
        for(int i = 0; i < 12; i++) {
          if(activeNotes[i] > 0) {
            midiEventPacket_t noteOff = {0x08, 0x80, 60 + i, 0};
            MidiUSB.sendMIDI(noteOff);
            activeNotes[i] = 0;
          }
        }
        MidiUSB.flush();
      }
      break;
    case MODE_SETTINGS:
      if(midiChannel > 0) midiChannel--;
      else midiChannel = 15;
      break;
  }
  displayNeedsUpdate = true;
}

bool scanKeyboard() {
  bool anyKeyPressed = false;

  for(int col = 0; col < NUM_COLS; col++) {
    scanColumn(col);
    
    for(int row = 0; row < NUM_ROWS; row++) {
      bool currentState = !digitalRead(rowPins[row]);
      unsigned long now = millis();
      
      if(currentState != keyPressed[row][col] && 
         now - lastPressTime[row][col] > DEBOUNCE_DELAY) {
        
        keyPressed[row][col] = currentState;
        lastPressTime[row][col] = now;
        displayNeedsUpdate = true;
        
        if(currentState) {
          noteOn(row, col);
          updateActiveNotes(row, col, true);
          lastActivityTime = now;
          recordNoteTiming(now);
          noteCount++;
          lastPlayedNote = keyToMidiMap[row][col] + currentOctaveShift * 12;
        } else {
          noteOff(row, col);
          updateActiveNotes(row, col, false);
        }
      }

      if(keyPressed[row][col]) {
        anyKeyPressed = true;
      }
    }
  }
  
  return anyKeyPressed;
}

void recordNoteTiming(unsigned long noteTime) {
  noteTimings[timingIndex] = noteTime;
  timingIndex = (timingIndex + 1) % 5; // Reducido a 5
  lastNoteTime = noteTime;
}

// Función simplificada de BPM (sin float)
int calculateBPM() {
  if(noteCount < 2) return 0;
  
  unsigned long totalInterval = 0;
  byte validIntervals = 0;
  
  for(byte i = 0; i < 4; i++) {
    byte nextIndex = (i + 1) % 5;
    if(noteTimings[i] > 0 && noteTimings[nextIndex] > 0) {
      unsigned long interval = noteTimings[nextIndex] - noteTimings[i];
      if(interval > 100 && interval < 5000) {
        totalInterval += interval;
        validIntervals++;
      }
    }
  }
  
  if(validIntervals > 0) {
    return 60000 / (totalInterval / validIntervals);
  }
  return 0;
}

void updateActiveNotes(int row, int col, bool pressed) {
  int midiNote = keyToMidiMap[row][col] + currentOctaveShift * 12;
  midiNote = constrain(midiNote, 0, 127);
  int semitone = midiNote % 12;
  
  if(pressed) {
    activeNotes[semitone]++;
    totalActiveNotes++;
  } else {
    if(activeNotes[semitone] > 0) {
      activeNotes[semitone]--;
      totalActiveNotes--;
    }
  }
}

void updateDisplay(bool anyKeyPressed) {
  if(displaySleep) return;
  if(!displayNeedsUpdate && currentMode != MODE_RECORDER) return;
  
  display.clearDisplay();
  
  // Barra superior simplificada
  display.fillRect(0, 0, 128, 10, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(2, 2);
  
  // Usar PROGMEM
  char buffer[8];
  strcpy_P(buffer, (char*)pgm_read_word(&(modeNames[currentMode])));
  display.print(buffer);
  
  display.print(F(" Ch:"));
  display.print(midiChannel + 1);
  
  display.setCursor(80, 2);
  display.print(F("O:"));
  if(currentOctaveShift >= 0) display.print(F("+"));
  display.print(currentOctaveShift);
  
  display.setTextColor(SSD1306_WHITE);
  
  switch(currentMode) {
    case MODE_PERFORMANCE:
      displayPerformanceMode(anyKeyPressed);
      break;
    case MODE_CHORD:
      displayChordMode();
      break;
    case MODE_SCALE:
      displayScaleMode();
      break;
    case MODE_OCTAVE:
      displayOctaveMode();
      break;
    case MODE_RECORDER:
      displayRecorderMode();
      break;
    case MODE_SETTINGS:
      displaySettingsMode();
      break;
    case MODE_TUNER:
      displayDebugMode(); // Cambiado de displayTunerMode a displayDebugMode
      break;
  }
  
  display.display();
  if(currentMode != MODE_RECORDER) {
    displayNeedsUpdate = false;
  }
}

void displayPerformanceMode(bool anyKeyPressed) {
  // Mostrar nota actual en el centro de la pantalla (grande)
  if(lastPlayedNote >= 0) {
    display.setTextSize(3);
    display.setCursor(25, 15);
    char noteName[6];
    strcpy_P(noteName, (char*)pgm_read_word(&(noteNames[lastPlayedNote % 12])));
    display.print(noteName);
  }
  
  // Siempre dibujar la octava completa en la parte inferior (una línea más arriba)
  drawFullOctavePiano(4, 37);
}

void displayDebugMode() {
  display.setTextSize(1);
  display.setCursor(4, 15);
  display.println(F("DEBUG SISTEMA:"));
  
  // Estado de todos los botones en tiempo real
  display.setCursor(4, 25);
  display.print(F("UP: "));
  display.print(!digitalRead(octaveUpPin) ? F("ON ") : F("OFF"));
  display.print(F(" DN: "));
  display.print(!digitalRead(octaveDownPin) ? F("ON ") : F("OFF"));
  
  display.setCursor(4, 35);
  display.print(F("MODE: "));
  display.print(!digitalRead(modeButtonPin) ? F("ON ") : F("OFF"));
  display.print(F(" Oct: "));
  display.print(currentOctaveShift);
  
  if(lastPlayedNote >= 0) {
    // Mostrar información detallada de la última tecla presionada
    display.setCursor(4, 45);
    display.print(F("MIDI: "));
    display.print(lastPlayedNote);
    
    // Simplificar nombre de nota para que quepa
    int noteIndex = lastPlayedNote % 12;
    char noteName[6];
    strcpy_P(noteName, (char*)pgm_read_word(&(noteNames[noteIndex])));
    display.print(F(" "));
    display.print(noteName);
    
    // Mostrar canal en la misma línea
    display.print(F(" Ch"));
    display.print(midiChannel + 1);
  } else {
    display.setCursor(4, 45);
    display.print(F("Presiona una tecla..."));
  }
}

void displayChordMode() {
  if(totalActiveNotes >= 2) {
    display.setTextSize(1);
    display.setCursor(4, 15);
    display.print(F("ACORDE DETECTADO:"));
    
    // Mostrar tipo de acorde básico
    display.setTextSize(2);
    display.setCursor(10, 25);
    if(totalActiveNotes == 2) {
      display.print(F("Intervalo"));
    } else if(totalActiveNotes == 3) {
      display.print(F("Triada"));
    } else if(totalActiveNotes == 4) {
      display.print(F("Septima"));
    } else {
      display.print(F("Complejo"));
    }
    
    // Mostrar notas del acorde (más compacto)
    display.setTextSize(1);
    display.setCursor(4, 43);
    display.print(F("Notas: "));
    bool first = true;
    byte count = 0;
    for(byte i = 0; i < 12 && count < 6; i++) {
      if(activeNotes[i] > 0) {
        if(!first) display.print(F(" "));
        char noteName[6];
        strcpy_P(noteName, (char*)pgm_read_word(&(noteNames[i])));
        display.print(noteName);
        first = false;
        count++;
      }
    }
  } else {
    display.setTextSize(1);
    display.setCursor(15, 20);
    display.println(F("MODO APRENDIZAJE"));
    display.setCursor(10, 30);
    display.println(F("Toca 2 o mas"));
    display.setCursor(10, 40);
    display.println(F("notas para acordes"));
  }
}

void displayScaleMode() {
  display.setTextSize(1);
  display.setCursor(4, 15);
  
  // Mostrar nombre de escala actual
  char scaleName[12];
  strcpy_P(scaleName, scaleNames[currentScale]);
  display.print(F("Escala: "));
  display.println(scaleName);
  
  // Mostrar patrón de la escala (más compacto)
  display.setCursor(4, 25);
  display.print(F("Notas: "));
  for(byte i = 0; i < 6; i++) {
    byte note = pgm_read_byte(&scalePatterns[currentScale][i]);
    if(note == 0 && i > 0) break;
    
    char noteName[6];
    strcpy_P(noteName, (char*)pgm_read_word(&(noteNames[note % 12])));
    display.print(noteName);
    if(i < 5 && pgm_read_byte(&scalePatterns[currentScale][i+1]) != 0) display.print(F(" "));
  }
  
  // Verificar si estás tocando la escala correctamente
  display.setCursor(4, 35);
  if(totalActiveNotes > 0) {
    bool correctScale = true;
    for(byte i = 0; i < 12; i++) {
      if(activeNotes[i] > 0) {
        bool inScale = false;
        for(byte j = 0; j < 8; j++) {
          if((pgm_read_byte(&scalePatterns[currentScale][j]) % 12) == i) {
            inScale = true;
            break;
          }
        }
        if(!inScale) correctScale = false;
      }
    }
    
    if(correctScale && totalActiveNotes >= 3) {
      display.print(F("¡Escala correcta!"));
    } else if(totalActiveNotes >= 1) {
      display.print(F("Sigue el patron"));
    }
  } else {
    display.print(F("Toca las notas"));
  }
  
  // Instrucciones (línea 45)
  display.setCursor(4, 45);
  display.print(F("UP/DN: cambiar ("));
  display.print(currentScale + 1);
  display.print(F("/4)"));
}

void displayOctaveMode() {
  // Display grande de octava
  display.setTextSize(3);
  display.setCursor(45, 25);
  if(currentOctaveShift >= 0) display.print(F("+"));
  display.println(currentOctaveShift);
  
  // Info de botones simplificada
  display.setTextSize(1);
  display.setCursor(4, 15);
  display.print(F("UP/DN: Octava | MODE: Cambiar"));
  
  // Barra visual de octava
  display.drawRect(20, 50, 88, 8, SSD1306_WHITE);
  int barPos = 20 + ((currentOctaveShift + 3) * 88) / 6; // -3 a +3 mapeado
  display.fillRect(barPos - 2, 48, 4, 12, SSD1306_WHITE);
}

void displayRecorderMode() {
  display.setTextSize(1);
  display.setCursor(4, 15);
  display.print(F("GRABADOR:"));
  
  if(isRecording) {
    display.print(F(" GRABANDO..."));
    display.setCursor(4, 25);
    display.print(F("Notas: "));
    display.print(recordedCount);
    display.print(F("/"));
    display.print(MAX_RECORDED_NOTES);
    
    // Indicador de grabación parpadeante
    if((millis() / 250) % 2) {
      display.fillCircle(120, 18, 3, SSD1306_WHITE);
    }
  } else if(isPlaying) {
    display.print(F(" REPRODUCIENDO"));
    display.setCursor(4, 25);
    display.print(F("Nota: "));
    display.print(playIndex + 1);
    display.print(F("/"));
    display.print(recordedCount);
    
    // Barra de progreso
    if(recordedCount > 0) {
      int progress = map(playIndex, 0, recordedCount - 1, 0, 100);
      display.drawRect(10, 35, 104, 4, SSD1306_WHITE);
      display.fillRect(10, 35, progress, 4, SSD1306_WHITE);
    }
  } else {
    display.setCursor(4, 25);
    display.print(F("Guardadas: "));
    display.print(recordedCount);
    display.print(F("/"));
    display.print(MAX_RECORDED_NOTES);
    
    display.setCursor(4, 35);
    display.print(F("UP: Grabar | DN: Reproducir"));
    
    if(recordedCount == 0) {
      display.setCursor(4, 45);
      display.print(F("Toca para empezar"));
    }
  }
}

void displaySettingsMode() {
  display.setTextSize(1);
  
  display.setCursor(4, 15);
  display.print(F("Canal MIDI: "));
  display.println(midiChannel + 1);
  
  display.setCursor(4, 25);
  display.print(F("Velocity: "));
  display.println(currentVelocity);
  
  display.setCursor(4, 35);
  display.print(F("Octava: "));
  if(currentOctaveShift >= 0) display.print(F("+"));
  display.println(currentOctaveShift);
  
  display.setCursor(4, 45);
  display.print(F("Teclas: 54 (6x9)"));
}

// Octava completa en la parte inferior con nota marcada
void drawFullOctavePiano(int startX, int startY) {
  byte keyWidth = 15;  // Teclas más anchas para mejor visualización
  byte keyHeight = 25; // Teclas más largas usando más líneas
  byte blackKeyHeight = 18; // Teclas negras también más largas
  
  // Notas de una octava: Do, Do#, Re, Re#, Mi, Fa, Fa#, Sol, Sol#, La, La#, Si
  byte whiteKeys[] = {0, 2, 4, 5, 7, 9, 11};  // Do, Re, Mi, Fa, Sol, La, Si
  byte blackKeys[] = {1, 3, 6, 8, 10};        // Do#, Re#, Fa#, Sol#, La#
  byte blackPos[] = {0, 1, 3, 4, 5};          // Posiciones de teclas negras
  
  // Calcular qué nota está sonando en esta octava (0-11)
  int currentNoteInOctave = -1;
  if(lastPlayedNote >= 0) {
    currentNoteInOctave = lastPlayedNote % 12;
  }
  
  // Dibujar teclas blancas
  for(byte i = 0; i < 7; i++) {
    int x = startX + i * keyWidth;
    bool isPressed = (currentNoteInOctave == whiteKeys[i]);
    
    if(isPressed) {
      // Tecla presionada: rellena con efecto
      display.fillRect(x, startY, keyWidth - 2, keyHeight, SSD1306_WHITE);
      display.drawRect(x, startY, keyWidth - 2, keyHeight, SSD1306_BLACK);
    } else {
      // Tecla normal: solo borde
      display.drawRect(x, startY, keyWidth - 2, keyHeight, SSD1306_WHITE);
    }
  }
  
  // Dibujar teclas negras encima
  for(byte i = 0; i < 5; i++) {
    int x = startX + blackPos[i] * keyWidth + keyWidth/2;
    bool isPressed = (currentNoteInOctave == blackKeys[i]);
    
    if(isPressed) {
      // Tecla negra presionada: blanca con borde negro
      display.fillRect(x, startY, 6, blackKeyHeight, SSD1306_WHITE);
      display.drawRect(x, startY, 6, blackKeyHeight, SSD1306_BLACK);
    } else {
      // Tecla negra normal: negra con borde blanco
      display.fillRect(x, startY, 6, blackKeyHeight, SSD1306_BLACK);
      display.drawRect(x, startY, 6, blackKeyHeight, SSD1306_WHITE);
    }
  }
}

// Piano mejorado con mejor visualización
void drawEnhancedPiano(int startX, int startY) {
  byte keyWidth = 10;
  byte whiteKeys[] = {0, 2, 4, 5, 7, 9, 11};
  
  // Dibujar teclas blancas (más altas)
  for(byte i = 0; i < 7; i++) {
    int x = startX + i * keyWidth;
    if(activeNotes[whiteKeys[i]] > 0) {
      // Tecla presionada: rellena
      display.fillRect(x, startY, keyWidth - 1, 12, SSD1306_WHITE);
      display.drawRect(x, startY, keyWidth - 1, 12, SSD1306_BLACK);
    } else {
      // Tecla normal: solo borde
      display.drawRect(x, startY, keyWidth - 1, 12, SSD1306_WHITE);
    }
  }
  
  // Dibujar teclas negras encima (más pequeñas)
  byte blackKeys[] = {1, 3, 6, 8, 10};
  byte blackPos[] = {0, 1, 3, 4, 5};
  for(byte i = 0; i < 5; i++) {
    int x = startX + blackPos[i] * keyWidth + keyWidth/2;
    if(activeNotes[blackKeys[i]] > 0) {
      // Tecla negra presionada: blanca
      display.fillRect(x, startY, 4, 7, SSD1306_WHITE);
    } else {
      // Tecla negra normal: negra con borde blanco
      display.fillRect(x, startY, 4, 7, SSD1306_BLACK);
      display.drawRect(x, startY, 4, 7, SSD1306_WHITE);
    }
  }
}

// Piano simplificado para ahorrar memoria
void drawSimplePiano(int startX, int startY) {
  // Solo dibujar rectangulos básicos para las teclas activas
  byte keyWidth = 8;
  byte whiteKeys[] = {0, 2, 4, 5, 7, 9, 11};
  
  // Teclas blancas
  for(byte i = 0; i < 7; i++) {
    int x = startX + i * keyWidth;
    if(activeNotes[whiteKeys[i]] > 0) {
      display.fillRect(x, startY, keyWidth - 1, 10, SSD1306_WHITE);
    } else {
      display.drawRect(x, startY, keyWidth - 1, 10, SSD1306_WHITE);
    }
  }
  
  // Teclas negras (simplificado)
  byte blackKeys[] = {1, 3, 6, 8, 10};
  byte blackPos[] = {0, 1, 3, 4, 5};
  for(byte i = 0; i < 5; i++) {
    int x = startX + blackPos[i] * keyWidth + keyWidth/2;
    if(activeNotes[blackKeys[i]] > 0) {
      display.fillRect(x, startY, 3, 6, SSD1306_WHITE);
    } else {
      display.drawRect(x, startY, 3, 6, SSD1306_WHITE);
    }
  }
}

void scanColumn(int colNum) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, B11111111);
  shiftOut(dataPin, clockPin, MSBFIRST, bits[colNum]);
  digitalWrite(latchPin, HIGH);
}

void noteOn(int row, int col) {
  // Verificar que row y col están en rango válido
  if(row < 0 || row >= NUM_ROWS || col < 0 || col >= NUM_COLS) {
    Serial.print("ERROR: Row/Col fuera de rango: ");
    Serial.print(row);
    Serial.print(",");
    Serial.println(col);
    return;
  }
  
  int midiNote = keyToMidiMap[row][col] + currentOctaveShift * 12;
  
  // Verificar rango MIDI válido ANTES de constrain
  if(midiNote < 0 || midiNote > 127) {
    Serial.print("WARN: Nota fuera de rango MIDI: ");
    Serial.print(midiNote);
    Serial.print(" (Row:");
    Serial.print(row);
    Serial.print(" Col:");
    Serial.print(col);
    Serial.print(" Base:");
    Serial.print(keyToMidiMap[row][col]);
    Serial.print(" Octave:");
    Serial.print(currentOctaveShift);
    Serial.println(")");
    return; // No tocar notas fuera de rango
  }
  
  // Debug MIDI - temporalmente mostrar en pantalla
  lastPlayedNote = midiNote; // Para mostrar en debug
  
  // Grabación de notas
  if(isRecording && recordedCount < MAX_RECORDED_NOTES) {
    recordedSequence[recordedCount].note = midiNote;
    recordedSequence[recordedCount].timestamp = millis() - recordStartTime;
    recordedSequence[recordedCount].isNoteOn = true;
    recordedCount++;
  }
  
  // Verificar que velocity está en rango válido
  byte safeVelocity = constrain(currentVelocity, 1, 127);
  
  // Usar velocity variable
  midiEventPacket_t noteOn = {0x09, (uint8_t)(0x90 | midiChannel), (uint8_t)midiNote, safeVelocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
  
  // Debug serial mejorado
  Serial.print("NoteON: ");
  Serial.print(midiNote);
  Serial.print(" Row:");
  Serial.print(row);
  Serial.print(" Col:");
  Serial.print(col);
  Serial.print(" Ch:");
  Serial.print(midiChannel);
  Serial.print(" Vel:");
  Serial.print(safeVelocity);
  Serial.print(" Base:");
  Serial.print(keyToMidiMap[row][col]);
  Serial.print(" OctShift:");
  Serial.println(currentOctaveShift);
  
  // Test adicional: verificar que la nota está realmente enviándose
  if(midiNote != lastPlayedNote) {
    Serial.print("WARNING: Discrepancia en lastPlayedNote! Calculado:");
    Serial.print(midiNote);
    Serial.print(" Almacenado:");
    Serial.println(lastPlayedNote);
  }
}

void noteOff(int row, int col) {
  // Verificar que row y col están en rango válido
  if(row < 0 || row >= NUM_ROWS || col < 0 || col >= NUM_COLS) {
    Serial.print("ERROR: Row/Col fuera de rango en noteOff: ");
    Serial.print(row);
    Serial.print(",");
    Serial.println(col);
    return;
  }
  
  int midiNote = keyToMidiMap[row][col] + currentOctaveShift * 12;
  
  // Verificar rango MIDI válido
  if(midiNote < 0 || midiNote > 127) {
    Serial.print("WARN: NoteOff fuera de rango MIDI: ");
    Serial.print(midiNote);
    Serial.print(" (Row:");
    Serial.print(row);
    Serial.print(" Col:");
    Serial.print(col);
    Serial.println(")");
    return; // No procesar notas fuera de rango
  }
  
  // Grabación de notas
  if(isRecording && recordedCount < MAX_RECORDED_NOTES) {
    recordedSequence[recordedCount].note = midiNote;
    recordedSequence[recordedCount].timestamp = millis() - recordStartTime;
    recordedSequence[recordedCount].isNoteOn = false;
    recordedCount++;
  }
  
  midiEventPacket_t noteOff = {0x08, (uint8_t)(0x80 | midiChannel), (uint8_t)midiNote, 64};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
  
  // Debug serial mejorado
  Serial.print("NoteOFF: ");
  Serial.print(midiNote);
  Serial.print(" Row:");
  Serial.print(row);
  Serial.print(" Col:");
  Serial.print(col);
  Serial.print(" Ch:");
  Serial.println(midiChannel);
}

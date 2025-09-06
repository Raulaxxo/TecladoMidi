# Configuración de Teclado MIDI - Guía de Adaptación

## 📋 Configuración Básica (modificar al inicio del código .ino)

### 🎹 Tamaño del Teclado
```cpp
#define NUM_ROWS 6        // Número de filas de tu matriz
#define NUM_COLS 9        // Número de columnas de tu matriz
```

### 🎵 Configuración MIDI
```cpp
#define FIRST_MIDI_NOTE 31    // Primera nota MIDI (más grave)
#define OCTAVE_RANGE_LOW -2   // Octavas hacia abajo permitidas (solo OLED)
#define OCTAVE_RANGE_HIGH 1   // Octavas hacia arriba permitidas (solo OLED)
```

### 📡 Pines de Hardware
```cpp
const int rowPins[NUM_ROWS] = {4, 5, 6, 7, 8, 9}; // Lista de pines de filas

// Shift Register (74HC595)
const int dataPin = 10;
const int latchPin = 11;
const int clockPin = 12;

// Botones de control (solo versiones OLED)
const int btn1Pin = 13;  // Subir octava / Subir BPM
const int btn2Pin = A5;  // Bajar octava / Bajar BPM  
const int btn3Pin = A1;  // Cambiar modo / ON/OFF metrónomo

// Buzzer (solo versiones OLED)
const int buzzerPin = A4;
```

## 📁 Archivos Disponibles

### 🔽 PullDown (Resistencias Pull-Down)
- **`MidiPiano_PullDown.ino`**: Versión básica solo MIDI
- **`MidiPiano_PullDownOLED.ino`**: Versión completa con OLED, metrónomo y modos

### 🔼 PullUp (Resistencias Pull-Up)  
- **`MidiPiano_PullUp.ino`**: Versión básica solo MIDI
- **`MidiPiano_PullUpOled.ino`**: Versión completa con OLED, metrónomo y modos

### 🆕 Versión Avanzada (Recomendada)
- **`MidiPiano_con_OLED.ino`**: Versión más actualizada con todas las características

## 🔧 Ejemplos de Configuración

### Teclado Pequeño (4x6 = 24 teclas)
```cpp
#define NUM_ROWS 4
#define NUM_COLS 6
#define FIRST_MIDI_NOTE 48    // Do central
const int rowPins[NUM_ROWS] = {4, 5, 6, 7};
```

### Teclado Mediano (6x9 = 54 teclas) - ACTUAL
```cpp
#define NUM_ROWS 6
#define NUM_COLS 9  
#define FIRST_MIDI_NOTE 31    // Sol1
const int rowPins[NUM_ROWS] = {4, 5, 6, 7, 8, 9};
```

### Teclado Grande (8x12 = 96 teclas)
```cpp
#define NUM_ROWS 8
#define NUM_COLS 12
#define FIRST_MIDI_NOTE 24    // Do1
const int rowPins[NUM_ROWS] = {2, 3, 4, 5, 6, 7, 8, 9};
```

### Teclado Muy Grande (8x16 = 128 teclas)
```cpp
#define NUM_ROWS 8
#define NUM_COLS 16
#define FIRST_MIDI_NOTE 0     // Do(-1) - nota más grave MIDI
const int rowPins[NUM_ROWS] = {2, 3, 4, 5, 6, 7, 8, 9};
// NOTA: Requiere 2 shift registers para 16 columnas
```

## 🎼 Tabla de Notas MIDI Comunes

| Nota | MIDI | Octava | Descripción |
|------|------|--------|-------------|
| Do(-1) | 0  | -1 | Nota más grave posible |
| Do0    | 12 | 0  | Do grave |
| Do1    | 24 | 1  | Do bajo |
| Sol1   | 31 | 1  | **Configuración actual** |
| Do2    | 36 | 2  | Do medio-bajo |
| Do3    | 48 | 3  | Do central |
| Do4    | 60 | 4  | Do medio |
| Do5    | 72 | 5  | Do agudo |
| Do6    | 84 | 6  | Do muy agudo |
| Sol9   | 127| 9  | Nota más aguda posible |

## ⚙️ Características de Cada Versión

### 🔧 Versiones Básicas (_PullDown.ino, _PullUp.ino)
- ✅ Configuración adaptable de tamaño
- ✅ Generación automática de bitmasks
- ✅ Pines dinámicos 
- ✅ Soporte para 1-16 columnas
- ✅ MIDI USB nativo (Arduino Leonardo)
- ✅ Información de configuración en Serial Monitor

### 🖥️ Versiones OLED (_OLED.ino, _Oled.ino)
- ✅ Todo lo anterior +
- ✅ Pantalla OLED SSD1306 (128x64)
- ✅ Teclado visual en pantalla
- ✅ Control de octavas configurable
- ✅ Modo Piano + Modo Metrónomo
- ✅ Buzzer para metrónomo (60-200 BPM)
- ✅ Botones para control
- ✅ Notación latina (Do, Re, Mi, etc.)

## 🔄 Diferencias PullUp vs PullDown

### PullUp (Resistencias Pull-Up)
```cpp
// Los pines se configuran con resistencia pull-up interna
pinMode(rowPins[i], INPUT_PULLUP);

// Lectura invertida (LOW = presionada)
rowValue[i] = !digitalRead(rowPins[i]);

// Bitmasks invertidos para shift register
bits[i] = ~(1 << i); // B11111110, B11111101, etc.
```

### PullDown (Resistencias Pull-Down externas)
```cpp
// Los pines se configuran como entrada normal
pinMode(rowPins[i], INPUT);

// Lectura directa (HIGH = presionada)  
rowValue[i] = digitalRead(rowPins[i]);

// Bitmasks directos para shift register
bits[i] = 1 << i; // B00000001, B00000010, etc.
```

## ⚙️ Consideraciones de Hardware

### Shift Registers
- **1-8 columnas**: 1 shift register (74HC595)
- **9-16 columnas**: 2 shift registers en cascada
- **17+ columnas**: Requiere modificación adicional del código

### Pines Arduino Leonardo
- **Pines digitales disponibles**: 2-13, A0-A5
- **Máximo recomendado**: 8 filas + 3 shift register + 3 botones + buzzer

### Consumo de memoria
- **RAM por tecla**: ~3 bytes
- **Máximo recomendado**: 300 teclas (900 bytes de los 2560 disponibles)

## 🚀 Pasos para Adaptar

1. **Elegir versión**: 
   - Básica: Solo MIDI
   - OLED: Con pantalla y funciones avanzadas
   - PullUp: Si usas resistencias pull-up internas
   - PullDown: Si usas resistencias pull-down externas

2. **Modificar configuración**: 
   - Cambiar `NUM_ROWS`, `NUM_COLS`, `FIRST_MIDI_NOTE`
   - Actualizar `rowPins[]` según tu conexión

3. **Verificar shift registers**: 
   - Añadir segundo registro si `NUM_COLS > 8`

4. **Compilar y probar**: 
   - El código se adapta automáticamente

5. **Ajustar octavas** (solo OLED): 
   - Modificar `OCTAVE_RANGE_LOW` y `OCTAVE_RANGE_HIGH`

## ✅ Verificación Automática

Todos los códigos incluyen verificación automática que muestra en Serial Monitor:
- Tamaño de teclado detectado (filas x columnas)
- Número total de teclas
- Rango de notas MIDI disponible
- Configuración de octavas (versiones OLED)
- Tipo de configuración (PullUp/PullDown)

¡Todos los códigos están preparados para adaptarse automáticamente a tu configuración!

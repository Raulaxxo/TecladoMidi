# 🎹 Teclado MIDI con Pantalla OLED

**Versión actual:** v2.0 - Teclado MIDI + Metrónomo  
**Microcontrolador:** Arduino Leonardo  
**Fecha:** Diciembre 2024

## 📋 Descripción

Proyecto de teclado MIDI de 54 teclas (6x9 matriz) con pantalla OLED integrada y funcionalidad de metrónomo. Construido con Arduino Leonardo para comunicación USB MIDI nativa.

## 🔧 Hardware Requerido

### Componentes Principales
- **Arduino Leonardo** - Microcontrolador principal con USB MIDI nativo
- **Pantalla OLED SSD1306** - 128x64 píxeles, comunicación I2C
- **2x Registros de desplazamiento 74HC595** - Para escaneo de columnas
- **Matriz de teclas 6x9** - 54 teclas totales
- **Buzzer activo** - Para metrónomo (pin A4)
- **3 botones de control** - Navegación y control

### Configuración de Pines

```
Teclado (Filas):
- Fila 1: Pin 4
- Fila 2: Pin 5  
- Fila 3: Pin 6
- Fila 4: Pin 7
- Fila 5: Pin 8
- Fila 6: Pin 9

Registros 74HC595:
- Data Pin: Pin 10
- Latch Pin: Pin 11
- Clock Pin: Pin 12

Controles:
- Botón 1: Pin 13 (Octava ↑ / BPM ↑)
- Botón 2: Pin A5 (Octava ↓ / BPM ↓)  
- Botón 3: Pin A1 (Cambiar modo / ON/OFF metrónomo)

Periféricos:
- Buzzer: Pin A4
- OLED: SDA/SCL (I2C por defecto)
```

## 🎵 Funcionalidades

### 🎹 Modo 0: Piano
- **Teclado visual** con representación de octava completa
- **Control de octavas** desde -2 a +2 (rango total: C-1 a G9)
- **Notación latina** (Do, Re, Mi, Fa, Sol, La, Si)
- **Visualización en tiempo real** de notas tocadas
- **Teclas blancas y negras** dibujadas con precisión
- **Bordes elegantes** en la pantalla

#### Controles Modo Piano:
- **Botón 1**: Subir octava (+1)
- **Botón 2**: Bajar octava (-1)
- **Botón 3**: Cambiar a modo metrónomo

### 🥁 Modo 1: Metrónomo
- **Rango BPM**: 60 a 200 BPM
- **Incrementos**: Pasos de 5 BPM
- **BPM por defecto**: 120 BPM
- **Sonido**: Beep de 800Hz por 30ms
- **Indicador visual**: Cuadrado que parpadea con el beat
- **Última nota**: Muestra la nota tocada más reciente

#### Controles Modo Metrónomo:
- **Botón 1**: Subir BPM (+5)
- **Botón 2**: Bajar BPM (-5)
- **Botón 3** (mantener 2s): Activar/Desactivar metrónomo

## 🖥️ Interfaz de Pantalla

### Modo Piano
```
┌────────────────────────────┐
│ PIANO - Oct:+1             │
│ ────────────────────────── │
│      Do4                   │
│ ██████████████████████████ │
│ ██  ██  ██  ██  ██  ██  ██ │
│ ██  ██  ██  ██  ██  ██  ██ │
│ ██████████████████████████ │
└────────────────────────────┘
```

### Modo Metrónomo
```
┌────────────────────────────┐
│ METRONOMO                  │
│ ────────────────────────── │
│    120  BPM     ●          │
│ ON              ■          │
│ Nota:Do4                   │
│ 1/2:BPM 3:ON/OFF           │
└────────────────────────────┘
```

## 📚 Librerías Requeridas

```cpp
#include "MIDIUSB.h"           // v1.0.5 - Comunicación MIDI USB
#include <Wire.h>              // v1.0 - Comunicación I2C
#include <Adafruit_GFX.h>      // v1.12.1 - Gráficos base
#include <Adafruit_SSD1306.h>  // v2.5.15 - Driver OLED
```

### Instalación de Librerías
1. Abrir Arduino IDE
2. Ir a **Sketch > Include Library > Manage Libraries**
3. Buscar e instalar:
   - `MIDIUSB` by Arduino
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`

## 🚀 Instalación y Uso

### ⚡ Antes de Empezar: Identificar tu Tipo de Teclado

**¿No sabes si tu teclado es PullUp o PullDown?**  
👉 **[Guía: Cómo identificar PullUp vs PullDown](IDENTIFICAR_PULLUP_PULLDOWN.md)**

Esta guía te ayudará a:
- 🔍 Identificar visualmente tu tipo de teclado
- 🔧 Usar un multímetro para verificar
- 💻 Ejecutar código de diagnóstico
- 📋 Elegir el código correcto para tu hardware

### 📁 Archivos Disponibles

#### 🔼 Para Teclados PullUp (más común):
- `MidiPiano_PullUp.ino` - Versión básica solo MIDI
- `MidiPiano_PullUpOled.ino` - Con pantalla OLED y metrónomo

#### 🔽 Para Teclados PullDown:
- `MidiPiano_PullDown.ino` - Versión básica solo MIDI  
- `MidiPiano_PullDownOLED.ino` - Con pantalla OLED y metrónomo

#### 🆕 Versión Recomendada (PullUp optimizada):
- `MidiPiano_con_OLED/MidiPiano_con_OLED.ino` - Más actualizada

### 📐 Configuración para Diferentes Tamaños

**¿Tienes un teclado de diferente tamaño?**  
👉 **[Guía de Configuración de Tamaños](CONFIGURACION_TECLADO.md)**

Todos los códigos son fácilmente adaptables para:
- Teclados pequeños (4x6 = 24 teclas)
- Teclados medianos (6x9 = 54 teclas) ← **Actual**
- Teclados grandes (8x12 = 96 teclas)
- Teclados muy grandes (8x16 = 128 teclas)

### 🛠️ Pasos de Instalación

### 1. Identificar tu Teclado
👉 **[Sigue esta guía primero](IDENTIFICAR_PULLUP_PULLDOWN.md)** para determinar si tu teclado es PullUp o PullDown

### 2. Preparación del Arduino IDE
```bash
# Seleccionar placa: Arduino Leonardo
# Puerto: COMx (Windows) / /dev/ttyACMx (Linux)
```

### 3. Elegir y Cargar el Código Correcto
1. **Clona este repositorio**
2. **Elige el archivo según tu tipo de teclado:**
   - PullUp: `PianoMidiPullUp/MidiPiano_PullUpOled.ino`
   - PullDown: `PianoMidiPullDown/MidiPiano_PullDownOLED.ino`
   - Recomendado: `MidiPiano_con_OLED/MidiPiano_con_OLED.ino`
3. **Verifica que todas las librerías estén instaladas**
4. **Selecciona Arduino Leonardo** como placa
5. **Compila y sube el código**

### 4. Configurar Tamaño (si es necesario)
Si tu teclado no es 6x9, modifica estas líneas al inicio del código:
```cpp
#define NUM_ROWS 6        // Tu número de filas
#define NUM_COLS 9        // Tu número de columnas
const int rowPins[NUM_ROWS] = {4, 5, 6, 7, 8, 9}; // Tus pines
```

### 5. Conexión MIDI
- **Windows**: Aparece como dispositivo MIDI "Arduino Leonardo"
- **macOS/Linux**: Reconocido automáticamente como interfaz MIDI
- **DAW**: Usar canal MIDI 1 (por defecto)

## 🔍 Estructura del Proyecto

```
TecladoMidi/
├── README.md
├── PianoMidiPullDown/         # Versión antigua (pull-down)
│   └── MidiPiano_PullDown.ino
└── PianoMidiPullUp/           # Versión actual (pull-up)
    ├── MatrizPiano.png        # Diagrama de matriz
    ├── PianoMidiPullUp2.png   # Imagen del proyecto
    ├── Pinout.png             # Diagrama de pines
    └── MidiPiano_con_OLED/    # 🎯 Código principal
        └── MidiPiano_con_OLED.ino
```

## ⚡ Características Técnicas

### Rendimiento
- **Latencia MIDI**: <1ms (USB nativo)
- **Polling rate**: Escaneo continuo de matriz
- **Actualización pantalla**: 500ms
- **Debounce botones**: 300ms
- **Rango MIDI**: 0-127 (C-1 a G9)

### Optimizaciones
- **Escaneo eficiente** de matriz con registros de desplazamiento
- **Actualización condicional** de pantalla para reducir carga
- **Control de octavas** para acceso a todo el rango MIDI
- **Gestión de memoria** optimizada

## 🛠️ Desarrollo y Depuración

### Debug Serie
El código incluye mensajes de debug por puerto serie (31250 baud):
```cpp
// Ejemplos de salida:
"OLED inicializado correctamente"
"Octava: +1"
"BPM: 125"
"Metrónomo: ACTIVO"
"Beat! BPM: 120"
```

### Solución de Problemas Comunes

#### Pantalla OLED no funciona
- Verificar conexiones SDA/SCL
- Probar dirección I2C (0x3C por defecto)
- Revisar alimentación (3.3V o 5V según modelo)

#### MIDI no se detecta
- Verificar que es Arduino Leonardo (no Uno/Nano)
- Reinstalar drivers USB
- Comprobar cable USB de datos (no solo carga)

#### Metrónomo no suena
- Verificar conexión del buzzer en pin A4
- Probar con buzzer activo (no pasivo)
- Revisar polaridad del buzzer

## � Documentación Adicional

### 📖 Guías de Configuración
- **[🔍 Identificar PullUp vs PullDown](IDENTIFICAR_PULLUP_PULLDOWN.md)** - Determina qué tipo de teclado tienes
- **[📐 Configuración de Tamaños](CONFIGURACION_TECLADO.md)** - Adapta el código para diferentes tamaños de teclado

### 🖼️ Esquemas y Diagramas
- **[Esquemas de Conexión](PianoMidiPullUp/images/)** - Diagramas de conexión visual
- **[Matriz del Piano](PianoMidiPullUp/images/MatrizPiano.png)** - Layout de la matriz de teclas
- **[Pinout del Arduino](PianoMidiPullUp/images/Pinout.png)** - Configuración de pines

### 📁 Estructura del Proyecto
```
TecladoMidi/
├── README.md                           # Este archivo
├── IDENTIFICAR_PULLUP_PULLDOWN.md     # Guía de identificación
├── CONFIGURACION_TECLADO.md           # Configuración de tamaños
├── PianoMidiPullUp/                   # Códigos para PullUp
│   ├── MidiPiano_PullUp.ino          # Básico
│   ├── MidiPiano_PullUpOled.ino      # Con OLED
│   └── images/                        # Esquemas
├── PianoMidiPullDown/                 # Códigos para PullDown
│   ├── MidiPiano_PullDown.ino        # Básico
│   └── MidiPiano_PullDownOLED.ino    # Con OLED
└── MidiPiano_con_OLED/               # Versión más actualizada
    └── MidiPiano_con_OLED.ino        # Recomendado
```

## �📈 Historial de Versiones

### v2.0 (Actual)
- ✅ 2 modos: Piano + Metrónomo
- ✅ Interfaz OLED optimizada
- ✅ Control de octavas ±2
- ✅ Metrónomo con buzzer
- ✅ Notación latina
- ✅ Código optimizado para estabilidad

### v1.x (Anteriores)
- Piano básico con OLED
- Sistema de grabación/reproducción
- 3 modos (Piano/Grabación/Config)
- Problemas de estabilidad

## 🤝 Contribuciones

Este proyecto está en desarrollo activo. Las contribuciones son bienvenidas:

1. **Fork** el repositorio
2. Crea una **rama** para tu feature
3. **Commit** tus cambios
4. **Push** a la rama
5. Abre un **Pull Request**

## 📄 Licencia

Proyecto de código abierto para uso educativo y personal.

## 👨‍💻 Autor

**Raulaxxo**  
Proyecto TecladoMidi - 2024

---

## 🔗 Enlaces Útiles

- [Documentación Arduino Leonardo](https://docs.arduino.cc/hardware/leonardo/)
- [Librería MIDIUSB](https://github.com/arduino-libraries/MIDIUSB)
- [Adafruit SSD1306 Guide](https://learn.adafruit.com/monochrome-oled-breakouts)
- [74HC595 Datasheet](https://www.ti.com/lit/ds/symlink/sn74hc595.pdf)

---

*¡Disfruta creando música con tu teclado MIDI! 🎵*
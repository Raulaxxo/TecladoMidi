# 🎹 Teclado MIDI Educativo - Raulaxxo

Un teclado MIDI educativo basado en Arduino Leonardo con pantalla OLED, diseñado específicamente para el aprendizaje musical interactivo.

![Estado del Proyecto](https://img.shields.io/badge/Estado-Funcional-brightgreen)
![Versión](https://img.shields.io/badge/Versión-2.1-blue)
![Arduino](https://img.shields.io/badge/Arduino-Leonardo-orange)
![Última Actualización](https://img.shields.io/badge/Actualizado-Sept_2025-yellow)

## 📋 Tabla de Contenidos

- [Características](#-características)
- [Hardware Requerido](#-hardware-requerido)
- [Instalación](#-instalación)
- [Modos de Funcionamiento](#-modos-de-funcionamiento)
- [Configuración](#-configuración)
- [Uso Educativo](#-uso-educativo)
- [Mejoras Futuras](#-mejoras-futuras)
- [Contribuir](#-contribuir)
- [Créditos](#-créditos)

## ✨ Características

### 🎵 **Funcionalidades Actuales**
- **54 teclas** (matriz 6x9) con detección individual
- **Pantalla OLED 128x64** con interfaz visual mejorada
- **7 modos interactivos** de aprendizaje musical
- **Notación en solfeo** (Do, Re, Mi, Fa, Sol, La, Si)
- **Conectividad MIDI USB** nativa
- **Detección de acordes** en tiempo real
- **Escalas interactivas** con validación
- **Sistema de grabación/reproducción** de secuencias musicales
- **Control de octava** (-3 a +3)
- **Velocity variable** (1-127)
- **16 canales MIDI** seleccionables
- **Modo debug** para diagnóstico
- **Visualización de teclado** en tiempo real

### 🎯 **Optimizado para Aprendizaje**
- **Feedback visual** inmediato de notas y acordes
- **Guías de escalas** predefinidas (Mayor, menor, Pentatónica, Blues)
- **Validación en tiempo real** de patrones musicales
- **Grabación de ejercicios** para práctica repetitiva
- **Interfaz intuitiva** con navegación por 3 botones
- **Modo sleep automático** para conservar energía
- **Pantalla principal optimizada** con teclado visual y nota central

## 🔧 Hardware Requerido

### **Componentes Principales**
- **Arduino Leonardo** (con MIDI USB nativo)
- **Pantalla OLED SSD1306** 128x64 (I2C)
- **Matriz de pulsadores** 6x9 (54 teclas)
- **2 Shift Registers 74HC595** para multiplexado
- **3 Botones** para navegación (UP, DOWN, MODE)
- **Resistencias pull-up** 10kΩ

### **Conexiones**
```
Arduino Leonardo Pinout:
├── Pines 4-9:     Filas de la matriz (INPUT_PULLUP)
├── Pin 10:        Data (74HC595)
├── Pin 11:        Latch (74HC595)  
├── Pin 12:        Clock (74HC595)
├── Pin 13:        Botón UP (octava/función +)
├── Pin A0:        Botón MODE (cambio de modo)
├── Pin A1:        Botón DOWN (octava/función -)
├── SDA/SCL:       Pantalla OLED I2C
└── 5V/GND:        Alimentación
```

### **Esquema de Matriz**
```
Matriz 6x9 = 54 teclas organizadas cromáticamente:
Fila 0: Do  Do# Re  Re# Mi  Fa  Fa# Sol Sol#
Fila 1: La  La# Si  Do  Do# Re  Re# Mi  Fa
Fila 2: Fa# Sol Sol# La  La# Si  Do  Do# Re
... (continúa cromáticamente)
```

## 🚀 Instalación

### **1. Preparar Arduino IDE**
```bash
# Instalar librerías requeridas:
- MIDIUSB (Arduino)
- Adafruit SSD1306
- Adafruit GFX Library
- Wire (incluida)
```

### **2. Cargar el Código**
1. Conecta el Arduino Leonardo
2. Selecciona **Herramientas > Placa > Arduino Leonardo**
3. Selecciona el puerto COM correcto
4. Carga `midi_lsd_teclado.ino`

### **3. Configurar Audio MIDI**
1. **Windows**: Instalar [MIDI-OX](http://www.midiox.com/) o [VirtualMIDISynth](https://coolsoft.altervista.org/en/virtualmidisynth)
2. **macOS**: Usar Audio MIDI Setup integrado
3. **Linux**: Configurar ALSA MIDI

### **4. Verificar Funcionamiento**
- Al encender: pantalla muestra "Raulaxxo - Teclado Educativo"
- Sonido de prueba: acorde Do mayor (Do-Mi-Sol)
- Monitor Serial (115200 baud): mensajes "NoteON/NoteOFF"

## 🎮 Modos de Funcionamiento

### **1. 🎹 PERFORMANCE** (Modo Principal) - ✨ RENOVADO
- **Función**: Tocar libremente con feedback visual mejorado
- **Pantalla**: 
  - Nota actual en **grande al centro** (Do, Re, Mi, etc.)
  - **Teclado visual** en la parte inferior con tecla presionada resaltada
  - Diseño limpio y minimalista
- **Botones**: 
  - MODE: Cambiar a modo ACORDES
  - UP/DOWN: Cambiar octava (-3 a +3)
- **Uso**: Práctica libre, aprendizaje visual de notas

### **2. 🎵 ACORDES** (Análisis de Armonía)
- **Función**: Detecta y clasifica acordes en tiempo real
- **Pantalla**: Tipo de acorde (Intervalo/Tríada/Séptima), notas activas
- **Botones**: 
  - MODE: Cambiar a modo ESCALAS
  - UP/DOWN: Cambiar octava
- **Uso**: Aprender construcción de acordes

### **3. 📈 ESCALAS** (Guías Interactivas)
- **Función**: Escalas predefinidas con validación
- **Escalas incluidas**: 
  - Do Mayor (Do-Re-Mi-Fa-Sol-La-Si)
  - Do menor (Do-Re-Mi♭-Fa-Sol-La♭-Si♭)
  - Pentatónica (Do-Re-Mi-Sol-La)
  - Blues (Do-Mi♭-Fa-Fa#-Sol-Si♭)
- **Pantalla**: Patrón de escala, validación en tiempo real
- **Botones**: 
  - MODE: Cambiar a modo OCTAVAS
  - UP/DOWN: Cambiar entre escalas (1/4, 2/4, etc.)
- **Uso**: Practicar escalas, entender patrones

### **4. 🎚️ OCTAVAS** (Control de Rango)
- **Función**: Visualización y control de octava
- **Pantalla**: Octava actual (grande), barra visual, instrucciones
- **Rango**: -3 a +3 octavas
- **Botones**: 
  - MODE: Cambiar a modo GRABADOR
  - UP/DOWN: Cambiar octava
- **Uso**: Cambio rápido de registro

### **5. 🎙️ GRABADOR** (Grabación/Reproducción) - 🆕 NUEVO
- **Función**: Graba y reproduce secuencias musicales con timing preciso
- **Capacidad**: Hasta 20 notas con timestamps exactos
- **Pantalla**: 
  - Estado actual (GRABANDO/REPRODUCIENDO/LISTO)
  - Contador de notas grabadas
  - Barra de progreso durante reproducción
  - Indicador visual parpadeante durante grabación
- **Botones**: 
  - MODE: Cambiar a modo CONFIG
  - UP: Iniciar/Parar grabación
  - DOWN: Reproducir/Parar reproducción
- **Uso**: Practicar melodías, crear ejercicios, repetir patrones

### **6. ⚙️ CONFIG** (Configuración)
- **Función**: Ajustes del sistema
- **Pantalla**: Canal MIDI, velocity, octava, versión
- **Botones**: 
  - MODE: Cambiar a modo DEBUG
  - UP/DOWN: Cambiar canal MIDI (1-16)
- **Uso**: Configuración técnica

### **7. 🔧 DEBUG** (Diagnóstico)
- **Función**: Verificar funcionamiento de botones y MIDI
- **Pantalla**: Estado en tiempo real de los 3 botones, última nota
- **Botones**: 
  - MODE: Volver a modo PERFORMANCE
  - UP/DOWN: Sin función (solo para test)
- **Uso**: Solución de problemas, verificación de hardware

## ⚙️ Configuración

### **Valores por Defecto (Optimizados para Aprendizaje)**
```cpp
Octava base:        +1 (rango medio cómodo)
Velocity:           100 (volumen moderado)
Canal MIDI:         1 (estándar)
Nota inicial:       31 (Sol1)
Auto-sleep:         30 segundos
Debounce:           20ms
```

### **Navegación (Nuevo Esquema de 3 Botones)**
- **Botón MODE**: Cambio entre modos (Performance → Acordes → Escalas → etc.)
- **Botón UP**: Función específica del modo actual (octava+, velocity+, canal+, etc.)
- **Botón DOWN**: Función específica del modo actual (octava-, velocity-, canal-, etc.)
- **Sleep automático**: Pantalla se apaga tras 30s inactividad

## 🎓 Uso Educativo

### **Ejercicios Recomendados**

#### **🎹 Nivel Principiante**
1. **Reconocimiento de notas**: Modo PERFORMANCE
   - Toca notas individuales
   - Memoriza posiciones: Do, Re, Mi, Fa, Sol, La, Si

2. **Escalas básicas**: Modo ESCALAS
   - Practica Do Mayor: Do-Re-Mi-Fa-Sol-La-Si-Do
   - Usa validación automática para corrección

#### **🎵 Nivel Intermedio**
3. **Acordes fundamentales**: Modo ACORDES
   - Do Mayor: Do-Mi-Sol
   - La menor: La-Do-Mi
   - Fa Mayor: Fa-La-Do
   - Sol Mayor: Sol-Si-Re

4. **Progresiones básicas**:
   - I-V-vi-IV: Do-Sol-La menor-Fa
   - ii-V-I: Re menor-Sol-Do

#### **🎼 Nivel Avanzado**
5. **Escalas modales**: 
   - Dórico, Frigio, Lidio, Mixolidio
   - Blues y pentatónicas en diferentes tonalidades

6. **Acordes extendidos**:
   - Séptimas: Do7, Am7, F maj7
   - Novenas y oncenas

### **Metodología de Aprendizaje**
1. **Visual**: Retroalimentación inmediata en pantalla
2. **Auditivo**: Sonido MIDI en tiempo real
3. **Kinestésico**: Posicionamiento físico de dedos
4. **Teórico**: Nombres en solfeo y análisis harmónico

## 🚀 Mejoras Futuras

### **🎵 Software - Próximas Versiones**

#### **v2.2 - Metrónomo Integrado**
```cpp
Características:
├── BPM ajustable (40-200)
├── Subdivisiones (negras, corcheas, tresillos)
├── Click visual y sonoro
├── Sincronización con grabación
└── Patrones rítmicos básicos
```

#### **v2.3 - Progresiones de Acordes**
```cpp
Progresiones incluidas:
├── I-V-vi-IV (Pop/Rock básico)
├── ii-V-I (Jazz estándar)
├── vi-IV-I-V (Balada pop)
├── I-vi-ii-V (Circle progression)
└── Blues 12 compases
```

#### **v2.4 - Grabador de Secuencias**
```cpp
Funcionalidades:
├── Grabación en tiempo real
├── Playback con loop
├── Overdub (grabación superpuesta)
├── Quantización básica
├── Export MIDI (futuro)
└── 4 slots de grabación
```

#### **v2.5 - Modo Juego Musical**
```cpp
Juegos incluidos:
├── "Simon Says" musical
├── Adivinanza de intervalos
├── Completar escalas
├── Identificar acordes
└── Sistema de puntuación
```

### **🔧 Hardware - Expansiones**

#### **Expansión Básica (+$20-30)**
```
Componentes adicionales:
├── Encoder rotativo (navegación más rápida)
├── Buzzer piezoeléctrico (metrónomo offline)
├── LED RGB indicador de estado
├── Potenciómetro (control de velocity/BPM)
└── Interruptor on/off
```

#### **Expansión Avanzada (+$50-80)**
```
Mejoras significativas:
├── Matriz 8x8 (64 teclas = 5+ octavas)
├── LEDs individuales bajo teclas
├── Pedal sustain (entrada analógica)
├── Rueda pitch bend
├── Salida MIDI DIN estándar
└── Módulo Bluetooth (MIDI wireless)
```

#### **Versión Pro (+$100-150)**
```
Hardware profesional:
├── Teclas weighted/semi-weighted
├── Pantalla LCD color 2.8"
├── MicroSD para samples/presets
├── Multiple MIDI outputs
├── Conexión USB-C
├── Batería recargable
└── Carcasa profesional
```

### **📱 Conectividad Futura**

#### **Bluetooth MIDI**
```cpp
Características:
├── Conexión wireless a dispositivos
├── Compatible con iOS/Android apps
├── Latencia ultra-baja (<10ms)
├── Multi-device pairing
└── Profile BLE-MIDI estándar
```

#### **Aplicación Companion**
```
Funcionalidades móviles:
├── Configuración remota
├── Biblioteca de escalas/acordes
├── Lessons interactivas
├── Backing tracks
├── Grabación y compartir
└── Community features
```

## 🎯 Roadmap de Desarrollo

### **Q1 2025**
- [ ] Implementar metrónomo integrado
- [ ] Optimizar uso de memoria
- [ ] Agregar más escalas (modos griegos)
- [ ] Mejorar interfaz visual

### **Q2 2025**
- [ ] Progresiones de acordes guiadas
- [ ] Grabador básico de secuencias
- [ ] Conectividad Bluetooth
- [ ] App móvil prototype

### **Q3 2025**
- [ ] Modo juego musical
- [ ] Hardware expansion kit
- [ ] MIDI file import/export
- [ ] Web interface

### **Q4 2025**
- [ ] Versión comercial
- [ ] Kit DIY completo
- [ ] Documentación educativa
- [ ] Community platform

## 🤝 Contribuir

¡Las contribuciones son bienvenidas! Por favor:

1. **Fork** el repositorio
2. **Crea** una rama para tu feature (`git checkout -b feature/AmazingFeature`)
3. **Commit** tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. **Push** a la rama (`git push origin feature/AmazingFeature`)
5. **Abre** un Pull Request

### **Áreas donde puedes contribuir:**
- 🐛 **Bug fixes** y optimizaciones
- ✨ **Nuevas funcionalidades** educativas
- 📚 **Documentación** y tutoriales
- 🎨 **Mejoras de UI/UX**
- 🔧 **Diseños de hardware**
- 🎵 **Contenido educativo** (escalas, progresiones)

## � Changelog

### **v2.1 - 5 Septiembre 2025** 🆕
#### **✨ Nuevas Características:**
- **🎙️ Modo GRABADOR**: Sistema completo de grabación/reproducción
  - Graba hasta 20 notas con timing preciso
  - Reproducción con timing original
  - Indicadores visuales de estado (grabando/reproduciendo)
  - Barra de progreso durante reproducción
  - Útil para práctica y creación de ejercicios

#### **🎨 Mejoras de Interfaz:**
- **🎹 Modo PERFORMANCE renovado**:
  - Nota actual mostrada en **grande al centro**
  - **Teclado visual** en tiempo real en la parte inferior
  - Teclas presionadas se **resaltan visualmente**
  - Diseño limpio y minimalista
  - Eliminada información técnica innecesaria

#### **🔧 Optimizaciones Técnicas:**
- Reemplazado modo VELOCITY por modo GRABADOR más útil
- Mejorada visualización de teclado con proporciones realistas
- Optimización de memoria para sistema de grabación
- Estructura de datos eficiente para secuencias musicales

#### **📚 Documentación:**
- README actualizado con nuevas características
- Documentación completa del sistema de grabación
- Guías de uso para el modo GRABADOR
- Changelog detallado implementado

### **v2.0 - Agosto 2025**
- Sistema de 3 botones (UP/DOWN/MODE)
- Optimización de memoria PROGMEM
- 7 modos de funcionamiento
- Notación solfeo española
- Display OLED optimizado

### **v1.0 - Inicial**
- Matriz 6x9 teclas funcional
- MIDI USB básico
- 5 modos básicos
- Sistema 2 botones

## �📄 Licencia

Este proyecto está bajo la Licencia MIT - ver el archivo [LICENSE](LICENSE) para detalles.

## 🙏 Créditos

### **Proyecto Original**
- **Base inspiration**: [Piano MIDI Arduino Leonardo](https://www.instructables.com/Piano-Midi-Arduno-Leonardo/?cb=1650779803)

### **Desarrollador Principal**
- **Raulaxxo** - Desarrollo, optimización educativa, documentación

### **Librerías Utilizadas**
- [MIDIUSB](https://github.com/arduino-libraries/MIDIUSB) - Arduino MIDI USB
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) - OLED Display
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library) - Graphics

### **Comunidad**
- **Arduino Community** - Soporte técnico
- **MIDI Association** - Estándares MIDI
- **Música makers** - Feedback educativo

---

## 📞 Contacto & Soporte

- **Repositorio**: [github.com/Raulaxxo/TecladoMidi](https://github.com/Raulaxxo/TecladoMidi)
- **Issues**: Para reportar bugs o solicitar features
- **Discussions**: Para preguntas y community support

---

### 🎵 *"La música es el lenguaje universal de la humanidad"* - Henry Wadsworth Longfellow

**¡Construye, aprende y crea música con tecnología!** 🎹✨
# 📝 Changelog - Teclado MIDI Educativo

Todos los cambios importantes de este proyecto están documentados en este archivo.

El formato está basado en [Keep a Changelog](https://keepachangelog.com/es-ES/1.0.0/),
y este proyecto adhiere a [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.1.0] - 2025-09-05

### 🆕 Agregado
- **Sistema de Grabación/Reproducción**: 
  - Nuevo modo GRABADOR que reemplaza modo VELOCITY
  - Capacidad de grabar hasta 20 notas con timing preciso
  - Reproducción con timing original preservado
  - Estructura de datos optimizada para memoria limitada
  - Indicadores visuales de estado (grabando/reproduciendo/listo)
  - Barra de progreso durante reproducción
  - Control por botones UP (grabar) y DOWN (reproducir)

- **Interfaz de Usuario Mejorada**:
  - Modo PERFORMANCE completamente rediseñado
  - Nota actual mostrada en tamaño grande al centro
  - Teclado visual en tiempo real en la parte inferior
  - Visualización de teclas presionadas con resaltado
  - Octava completa dibujada (Do a Si) con proporciones realistas
  - Eliminación de información técnica innecesaria de pantalla principal

### 🔄 Cambiado
- **Modo VELOCITY** reemplazado por **Modo GRABADOR**
- **displayPerformanceMode()**: Interfaz simplificada y más visual
- **drawFullOctavePiano()**: Nueva función para dibujar teclado completo
- Teclas del teclado visual aumentadas de 15 a 25 píxeles de altura
- Posición del teclado visual movida más arriba para mejor visibilidad

### 🐛 Corregido
- Overflow de texto en pantalla OLED optimizado
- Mejora en proporciones del teclado visual
- Posicionamiento correcto de elementos en pantalla 128x64

### 🔧 Técnico
- Agregadas variables para sistema de grabación:
  - `RecordedNote` struct para almacenar notas + timestamps
  - `recordedSequence[MAX_RECORDED_NOTES]` array para secuencia
  - Variables de control: `isRecording`, `isPlaying`, `playIndex`
- Modificadas funciones `noteOn()` y `noteOff()` para captura automática
- Agregada lógica de reproducción en `loop()` principal
- Enum `DisplayMode` actualizado con `MODE_RECORDER`
- Manejadores de botones actualizados para nueva funcionalidad

### 📚 Documentación
- README.md completamente actualizado con nuevas características
- Sección de modos de funcionamiento renovada
- Changelog detallado agregado
- Documentación técnica del sistema de grabación
- Badges de versión actualizados

## [2.0.0] - 2025-08-XX

### 🆕 Agregado
- Sistema de navegación con 3 botones (UP/DOWN/MODE)
- Optimización de memoria con PROGMEM
- 7 modos de funcionamiento completos
- Notación solfeo en español (Do, Re, Mi, etc.)
- Sistema de sleep automático
- Modo DEBUG para diagnóstico
- Control de 16 canales MIDI

### 🔄 Cambiado
- Migración de 2 botones a 3 botones
- Optimización de strings para memoria flash
- Interfaz de usuario mejorada

### 🐛 Corregido
- Problemas de memoria (overflow de 29KB a 28KB disponibles)
- Funcionalidad de botones mejorada
- Detección de acordes optimizada

## [1.0.0] - 2025-XX-XX

### 🆕 Inicial
- Matriz de 54 teclas (6x9) funcional
- Conectividad MIDI USB básica
- Pantalla OLED SSD1306 128x64
- 5 modos básicos de funcionamiento
- Sistema de 2 botones
- Detección básica de notas y acordes

---

## 🔮 Próximas Versiones

### [2.2.0] - Planificado
- **Mejoras de Grabación**:
  - Múltiples slots de grabación (A, B, C)
  - Grabación de acordes completos
  - Loop de reproducción automático
  - Metrónomo integrado

### [2.3.0] - Planificado
- **Funciones Educativas Avanzadas**:
  - Modo QUIZ musical
  - Ejercicios de intervalos
  - Progresiones de acordes guiadas
  - Sistema de puntuación

### [3.0.0] - Planificado
- **Hardware v2**:
  - Botones táctiles capacitivos
  - Display color TFT
  - Conectividad Bluetooth MIDI
  - Almacenamiento en SD

---

## 📋 Tipos de Cambios

- `🆕 Agregado` - Para nuevas características
- `🔄 Cambiado` - Para cambios en funcionalidad existente
- `🐛 Corregido` - Para corrección de bugs
- `🔧 Técnico` - Para cambios técnicos/internos
- `📚 Documentación` - Para cambios solo en documentación
- `🚫 Eliminado` - Para características eliminadas
- `🔒 Seguridad` - Para correcciones de seguridad

---

*Mantenido por: Raulaxxo*  
*Última actualización: 5 Septiembre 2025*

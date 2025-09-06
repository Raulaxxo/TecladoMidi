# 🔍 ¿Cómo saber si mi teclado es PullUp o PullDown?

Esta guía te ayudará a identificar qué tipo de configuración eléctrica tiene tu teclado MIDI para elegir el código correcto.

## 🔋 Conceptos Básicos

### Pull-Up (Resistencias Pull-Up)
- **Estado normal**: Los pines están en **HIGH** (5V)
- **Al presionar**: El pin va a **LOW** (0V)
- **Resistencias**: Internas del Arduino o externas a VCC
- **Más común en**: Teclados comerciales, matrices simples

### Pull-Down (Resistencias Pull-Down)
- **Estado normal**: Los pines están en **LOW** (0V)
- **Al presionar**: El pin va a **HIGH** (5V)
- **Resistencias**: Externas a GND (tierra)
- **Más común en**: Prototipos caseros, diseños personalizados

## 🔧 Métodos de Identificación

### Método 1: Revisión Visual del Circuito

#### 🔼 Indicadores de PullUp:
```
[Arduino Pin] ---- [Resistencia] ---- [+5V/VCC]
                         |
                    [Interruptor] ---- [GND]
```
- ✅ Resistencias conectadas a **+5V** o **VCC**
- ✅ Interruptores/teclas conectan a **GND**
- ✅ Sin resistencias visibles (usa pull-up interno del Arduino)

#### 🔽 Indicadores de PullDown:
```
[Arduino Pin] ---- [Resistencia] ---- [GND]
                         |
                    [Interruptor] ---- [+5V/VCC]
```
- ✅ Resistencias conectadas a **GND** o **tierra**
- ✅ Interruptores/teclas conectan a **+5V**

### Método 2: Prueba con Multímetro

#### 🔼 Para PullUp:
1. **Sin presionar ninguna tecla**:
   - Voltaje en pines de filas: **~5V** (HIGH)
2. **Presionando una tecla**:
   - Voltaje en el pin correspondiente: **~0V** (LOW)

#### 🔽 Para PullDown:
1. **Sin presionar ninguna tecla**:
   - Voltaje en pines de filas: **~0V** (LOW)
2. **Presionando una tecla**:
   - Voltaje en el pin correspondiente: **~5V** (HIGH)

### Método 3: Prueba con Código de Diagnóstico

Carga este código simple para diagnosticar tu teclado:

```cpp
// CÓDIGO DE DIAGNÓSTICO - IDENTIFICAR PULLUP/PULLDOWN
const int testPin = 4; // Cambia por tu primer pin de fila

void setup() {
  Serial.begin(9600);
  pinMode(testPin, INPUT_PULLUP); // Activar pull-up interno
  
  Serial.println("=== DIAGNÓSTICO DE TECLADO ===");
  Serial.println("Conecta una tecla al pin 4 y presiona");
  Serial.println("Observa los valores:");
  delay(2000);
}

void loop() {
  int valor = digitalRead(testPin);
  Serial.print("Pin 4: ");
  Serial.print(valor);
  Serial.print(" (");
  Serial.print(valor ? "HIGH" : "LOW");
  Serial.println(")");
  
  delay(500);
}
```

#### 📊 Interpretación de resultados:
- **Sin presionar → HIGH, Presionando → LOW** = **PullUp** ✅
- **Sin presionar → LOW, Presionando → HIGH** = **PullDown** ✅

### Método 4: Identificación por Fabricante

#### 🔼 Típicamente PullUp:
- **Teclados comerciales** (Yamaha, Casio, etc.)
- **Matrices de botones** prefabricadas
- **Shields de Arduino** para teclados
- **Teclados de computadora** adaptados

#### 🔽 Típicamente PullDown:
- **Prototipos caseros** con breadboard
- **PCBs personalizadas**
- **Sistemas con muchas resistencias externas visibles**
- **Diseños educativos/experimentales**

## 🚨 Problemas Comunes de Identificación Incorrecta

### Si usas código PullUp en teclado PullDown:
- ❌ Todas las teclas parecen estar presionadas
- ❌ Notas MIDI se envían constantemente
- ❌ No se pueden tocar notas individuales

### Si usas código PullDown en teclado PullUp:
- ❌ Las teclas no responden
- ❌ No se envían notas MIDI
- ❌ Serial Monitor no muestra actividad

## 📋 Guía Rápida de Decisión

```
¿Tu teclado tiene resistencias visibles hacia GND?
├─ SÍ → Probablemente PullDown 🔽
└─ NO → ¿Las resistencias van hacia +5V?
   ├─ SÍ → Definitivamente PullUp 🔼
   └─ NO VISIBLE → Probablemente PullUp (interno) 🔼

¿Es un teclado comercial o shield?
├─ SÍ → PullUp 🔼
└─ NO → ¿Lo construiste tú?
   ├─ SÍ → Revisa tu esquema
   └─ NO → Prueba con código de diagnóstico
```

## 🎯 ¿Qué código usar?

Una vez identificado tu tipo:

### 🔼 Para PullUp:
- `MidiPiano_PullUp.ino` (básico)
- `MidiPiano_PullUpOled.ino` (con OLED)

### 🔽 Para PullDown:
- `MidiPiano_PullDown.ino` (básico)  
- `MidiPiano_PullDownOLED.ino` (con OLED)

### 🆕 Versión Universal (Recomendada):
- `MidiPiano_con_OLED.ino` (detecta automáticamente*)

*\*Nota: La versión con OLED en `/MidiPiano_con_OLED/` está optimizada para PullUp pero puede adaptarse fácilmente.*

## 💡 Consejos Finales

1. **Si tienes dudas**: Empieza con PullUp, es más común
2. **Si no funciona**: Cambia a PullDown
3. **Para nuevos proyectos**: Usa PullUp (más simple, no necesita resistencias externas)
4. **Documenta tu teclado**: Anota qué tipo es para futuras referencias

## 🔗 Enlaces Útiles

- [Configuración de Tamaños de Teclado](CONFIGURACION_TECLADO.md)
- [README Principal](README.md)
- [Esquemas de Conexión](PianoMidiPullUp/images/)

---

💡 **¿Aún tienes dudas?** Usa el código de diagnóstico - es la forma más segura de identificar tu tipo de teclado.

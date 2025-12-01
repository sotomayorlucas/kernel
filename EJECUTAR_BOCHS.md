# 🚀 Guía para Ejecutar el Kernel en Bochs

Esta guía te ayudará a compilar y ejecutar el kernel refactorizado en el emulador Bochs.

## 📋 Requisitos Previos

### Instalar Dependencias

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y build-essential nasm bochs bochs-x mtools
```

**Fedora/RHEL:**
```bash
sudo dnf install -y gcc make nasm bochs bochs-x mtools
```

**Arch Linux:**
```bash
sudo pacman -S gcc make nasm bochs mtools
```

**macOS (con Homebrew):**
```bash
brew install nasm bochs mtools
```

## 🔨 Compilación

### Paso 1: Limpiar compilaciones anteriores
```bash
make clean
```

### Paso 2: Compilar el kernel
```bash
make
```

Este comando:
- ✅ Compila todos los archivos `.c` a `.o` usando GCC con flags estrictos
- ✅ Ensambla todos los archivos `.asm` usando NASM
- ✅ Linkea todos los objetos creando `kernel.bin.elf`
- ✅ Extrae el binario plano `kernel.bin`
- ✅ Copia el kernel a la imagen de diskette `diskette.img`

### Verificar la compilación
```bash
ls -lh kernel.bin diskette.img
```

Deberías ver:
```
-rw-r--r-- 1 user user  114K kernel.bin
-rw-r--r-- 1 user user  1.4M diskette.img
```

## 🖥️ Ejecutar en Bochs

### Opción 1: Modo Normal (Sin Debugger)
```bash
bochs -f bochsrc -q
```

**Parámetros:**
- `-f bochsrc`: Usa el archivo de configuración
- `-q`: Modo quiet (sin preguntas interactivas)

### Opción 2: Modo Debug (Recomendado para desarrollo)
```bash
bochs -f bochsrc -q -rc bochsdbg
```

**Parámetros adicionales:**
- `-rc bochsdbg`: Ejecuta comandos del script de debug al inicio

### Opción 3: Debug Interactivo Completo
```bash
bochsdbg -f bochsrc -q
```

Esto abre el debugger de Bochs donde puedes:
- Poner breakpoints
- Inspeccionar registros
- Ver el estado de la memoria
- Ejecutar paso a paso

## 🎮 Controles en Bochs

### Durante la ejecución:

**Teclado:**
- Las teclas funcionan normalmente
- Prueba escribir algo para ver el cursor moverse
- Las flechas del teclado mueven el cursor

**Ventana de Bochs:**
- `Ctrl+C` en la terminal: pausa la ejecución (modo debug)
- Cerrar ventana: termina la emulación

## 🔍 Comandos del Debugger de Bochs

Si ejecutaste en modo debug interactivo, puedes usar:

```
c           - Continue (continuar ejecución)
s [N]       - Step (ejecutar N instrucciones)
n           - Next (siguiente instrucción)
b ADDR      - Breakpoint en dirección
info r      - Ver registros
info idt    - Ver tabla IDT
info gdt    - Ver tabla GDT
x /10xw ADDR - Examinar 10 palabras en memoria
q           - Quit (salir)
```

### Breakpoints útiles:

```gdb
# Breakpoint al inicio del kernel
b 0x1200

# Breakpoint en modo protegido
b modo_protegido

# Ver registros después del switch
info r

# Continuar
c
```

## 🧪 Qué Deberías Ver

### 1. **Al arrancar (Modo Real):**
```
Habilitando A20........OK!
Iniciando kernel en Modo Real
```

### 2. **Al cambiar a Modo Protegido:**
```
Iniciando kernel en Modo Protegido
```

### 3. **Pantalla principal:**
- Pantalla negra limpia
- Cursor parpadeante en alguna posición
- Puedes escribir con el teclado
- El cursor se mueve con las flechas

### 4. **Reloj (esquina inferior derecha):**
```
|/-\  (animación rotando)
```

## 🐛 Debugging con Magic Breakpoints

El código tiene varios `xchg bx, bx` que son "magic breakpoints" para Bochs.

**Para usarlos:**

1. Edita `bochsrc` y agrega:
```
magic_break: enabled=1
```

2. Ejecuta con debugger:
```bash
bochs -f bochsrc -q
```

3. Bochs se detendrá automáticamente en cada `xchg bx, bx`

## 📊 Verificar que las Refactorizaciones Funcionan

### Test 1: IDT y Excepciones
El código ejecuta dos syscalls de prueba:
```assembly
int 0x58  ; Syscall de prueba 1
int 0x62  ; Syscall de prueba 2
```

Si ves que el kernel no crashea, ¡la IDT funciona! ✅

### Test 2: PIC y Teclado
- Escribe algo con el teclado
- Deberías ver caracteres en pantalla
- Si funciona, el PIC y el handler de teclado están bien ✅

### Test 3: Reloj (IRQ 0)
- Mira la esquina inferior derecha
- Deberías ver `|/-\` rotando
- Si funciona, el timer interrupt está funcionando ✅

## 🔧 Solución de Problemas

### Error: "nasm: command not found"
```bash
# Instala NASM
sudo apt-get install nasm  # Ubuntu/Debian
```

### Error: "bochs: command not found"
```bash
# Instala Bochs
sudo apt-get install bochs bochs-x  # Ubuntu/Debian
```

### El kernel no arranca
1. Verifica que `diskette.img` se haya creado:
```bash
ls -lh diskette.img
```

2. Recompila desde cero:
```bash
make clean && make
```

### Bochs muestra "Panic" o "Triple Fault"
Esto suele indicar:
- Error en la GDT
- Error en la IDT
- Stack overflow
- Código ejecutando desde memoria inválida

**Para debuggear:**
```bash
bochs -f bochsrc -q
# Dentro del debugger:
info gdt
info idt
info r
```

### La pantalla está en blanco
- Verifica que `screen_draw_layout()` se esté llamando
- Revisa que el selector de video esté correcto en la GDT

## 📝 Logs y Salida

Bochs genera un archivo `bochs.log` con toda la información de ejecución.

**Ver últimas líneas del log:**
```bash
tail -50 bochs.log
```

**Buscar errores:**
```bash
grep -i "error\|panic\|exception" bochs.log
```

## 🎯 Siguiente Nivel: GDB con Bochs

Para debugging más avanzado, puedes usar GDB:

1. Edita `bochsrc` y agrega:
```
gdbstub: enabled=1, port=1234, text_base=0, data_base=0, bss_base=0
```

2. En una terminal, ejecuta Bochs:
```bash
bochs -f bochsrc -q
```

3. En otra terminal, ejecuta GDB:
```bash
gdb kernel.bin.elf
(gdb) target remote localhost:1234
(gdb) break start
(gdb) continue
```

## ✨ Resumen de Comandos Rápidos

```bash
# Workflow completo
make clean          # Limpiar
make               # Compilar
bochs -f bochsrc -q # Ejecutar

# Con debug
bochs -f bochsrc -q -rc bochsdbg

# Ver logs
tail -f bochs.log
```

---

¡Ahora tu kernel refactorizado con principios SOLID debería ejecutarse perfectamente en Bochs! 🎉

; ** por compatibilidad se omiten tildes **
; ==============================================================================
; System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
; ==============================================================================
;
; Definicion de rutinas de atencion de interrupciones

%include "print.mac"

BITS 32

;; Constantes - Selectores de Segmento
;; -------------------------------------------------------------------------- ;;
%define CS_RING_0_SEL    (1 << 3)

;; Constantes - Puertos de Hardware
;; -------------------------------------------------------------------------- ;;
%define KEYBOARD_DATA_PORT  0x60    ; Puerto de datos del teclado

;; Constantes - Offsets del Stack tras PUSHAD
;; -------------------------------------------------------------------------- ;;
; El orden de PUSHAD es: EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
%define offset_EDI 0
%define offset_ESI 4
%define offset_EBP 8
%define offset_ESP 12
%define offset_EBX 16
%define offset_EDX 20
%define offset_ECX 24
%define offset_EAX 28

;; Constantes - Offsets del Stack para Frame de Excepcion
;; -------------------------------------------------------------------------- ;;
%define GREGS_SIZE      (8*4)       ; 8 registros de 4 bytes
%define SREGS_SIZE      (6*4)       ; 6 registros de segmento
%define CREGS_SIZE      (4*4)       ; 4 registros de control
%define CLEANUP_SIZE    (CREGS_SIZE + SREGS_SIZE)

;; Declaraciones Externas
;; -------------------------------------------------------------------------- ;;
extern pic_finish1
extern kernel_exception
extern process_scancode

;; Definicion de Macros para ISRs
;; -------------------------------------------------------------------------- ;;

; Macro principal que maneja el contexto de una excepcion
%macro ISRc 1
    push DWORD %1
    ; Estado del Stack despues del push:
    ; [ INTERRUPT #] esp
    ; [ ERROR CODE ] esp + 0x04
    ; [ EIP        ] esp + 0x08
    ; [ CS         ] esp + 0x0c
    ; [ EFLAGS     ] esp + 0x10
    ; [ ESP        ] esp + 0x14 (si DPL(cs) == 3)
    ; [ SS         ] esp + 0x18 (si DPL(cs) == 3)

    ; Guardar registros generales
    pushad

    ; Verificar si hubo cambio de privilegio
    mov edx, [esp + (GREGS_SIZE + 3*4)]

    ; Guardar registros de segmento
    xor eax, eax
    mov ax, ss
    push eax
    mov ax, gs
    push eax
    mov ax, fs
    push eax
    mov ax, es
    push eax
    mov ax, ds
    push eax
    push eax ; cs

    ; Guardar registros de control
    mov eax, cr4
    push eax
    mov eax, cr3
    push eax
    mov eax, cr2
    push eax
    mov eax, cr0
    push eax

    cmp edx, CS_RING_0_SEL
    je .ring0_exception

    ; Excepcion en Ring 3 (no implementado)
    ;call ring3_exception
    jmp $

.ring0_exception:
    call kernel_exception
    add esp, CLEANUP_SIZE
    popad

    xchg bx, bx  ; Magic breakpoint para debugger
    jmp $
%endmacro

; Macro para ISR con codigo de error (Error)
%macro ISRE 1
global _isr%1

_isr%1:
  ISRc %1
%endmacro

; Macro para ISR sin codigo de error (No Error)
%macro ISRNE 1
global _isr%1

_isr%1:
  push DWORD 0x0
  ISRc %1
%endmacro

;; Rutinas de Atencion de las EXCEPCIONES
;; -------------------------------------------------------------------------- ;;
; Excepciones del CPU (0-20)

ISRNE 0   ; Division por cero
ISRNE 1   ; Debug
ISRNE 2   ; NMI Interrupt
ISRNE 3   ; Breakpoint
ISRNE 4   ; Overflow
ISRNE 5   ; BOUND Range Exceeded
ISRNE 6   ; Invalid Opcode
ISRNE 7   ; Device Not Available
ISRE  8   ; Double Fault
ISRNE 9   ; Coprocessor Segment Overrun
ISRE  10  ; Invalid TSS
ISRE  11  ; Segment Not Present
ISRE  12  ; Stack Fault
ISRE  13  ; General Protection
ISRE  14  ; Page-Fault
ISRNE 15  ; Reserved
ISRNE 16  ; x87 FPU Floating-Point Error
ISRE  17  ; Alignment Check
ISRNE 18  ; Machine-Check
ISRNE 19  ; SIMD Floating-Point
ISRNE 20  ; Virtualization

;; Rutina de Atencion del RELOJ (IRQ 0)
;; -------------------------------------------------------------------------- ;;
global _isr32

_isr32:
    pushad

    call next_clock

    call pic_finish1

    popad
    iret

;; Rutina de Atencion del TECLADO (IRQ 1)
;; -------------------------------------------------------------------------- ;;
global _isr33

_isr33:
    pushad

    ; Leer scancode del puerto de datos del teclado
    mov dx, KEYBOARD_DATA_PORT
    in eax, dx

    ; Pasar scancode como parametro a la funcion C
    push eax
    call process_scancode

    call pic_finish1

    add esp, 4
    popad
    iret

;; Rutinas de Atencion de las SYSCALLS
;; -------------------------------------------------------------------------- ;;

global _isr88
_isr88:
    mov eax, 0x58
    iret

global _isr98
_isr98:
    mov eax, 0x62
    iret

;; Funciones Auxiliares - Reloj
;; -------------------------------------------------------------------------- ;;

; Constantes para animacion del reloj
%define CLOCK_CHARS_COUNT  4
%define CLOCK_ROW          49
%define CLOCK_COL          79
%define CLOCK_COLOR        0x0f

section .data
    isrNumber:  dd 0x00000000
    isrClock:   db '|/-\'

section .text
next_clock:
    pushad
    inc DWORD [isrNumber]
    mov ebx, [isrNumber]
    cmp ebx, CLOCK_CHARS_COUNT
    jl .ok
        mov DWORD [isrNumber], 0x0
        mov ebx, 0
    .ok:
        add ebx, isrClock
        print_text_pm ebx, 1, CLOCK_COLOR, CLOCK_ROW, CLOCK_COL
        popad
    ret

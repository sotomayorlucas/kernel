; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TALLER System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
; ==============================================================================

%include "print.mac"

;; Constantes - Selectores de Segmento
;; -------------------------------------------------------------------------- ;;
%define CS_RING_0_SEL (1 << 3)
%define DS_RING_0_SEL (3 << 3)

;; Constantes - Configuracion de Video
;; -------------------------------------------------------------------------- ;;
%define VIDEO_MODE_80x50    0x0003  ; Modo de video 80x50
%define VIDEO_FONT_8x8      0x1112  ; Font 8x8

;; Constantes - Stack
;; -------------------------------------------------------------------------- ;;
%define KERNEL_STACK_TOP    0x25000 ; Tope del stack del kernel

;; Constantes - Syscalls de Prueba
;; -------------------------------------------------------------------------- ;;
%define SYSCALL_TEST_1      0x58    ; Syscall de prueba 1 (INT 88)
%define SYSCALL_TEST_2      0x62    ; Syscall de prueba 2 (INT 98)

;; Constantes - Valores de Debug
;; -------------------------------------------------------------------------- ;;
%define DEBUG_VALUE         0xFFFF  ; Valor para llenar registros en debug

;; Declaraciones Globales y Externas
;; -------------------------------------------------------------------------- ;;
global start

extern GDT
extern GDT_DESC
extern GDT_IDX_CODE_0
extern IDT_DESC

extern print_text_rm
extern A20_enable
extern screen_draw_layout
extern idt_init
extern pic_reset
extern pic_enable
extern pic_disable

;; Seccion de Datos
;; -------------------------------------------------------------------------- ;;
BITS 16
jmp start   ; Saltear seccion de datos

start_rm_msg db     'Iniciando kernel en Modo Real'
start_rm_len equ    $ - start_rm_msg

start_pm_msg db     'Iniciando kernel en Modo Protegido'
start_pm_len equ    $ - start_pm_msg

;; Seccion de Codigo - Modo Real
;; -------------------------------------------------------------------------- ;;

; Punto de entrada del kernel en Modo Real
BITS 16
start:
    cli  ; Desactivar interrupciones

    ; Configurar modo de video 80x50
    mov ax, VIDEO_MODE_80x50
    int 10h                     ; Establecer modo de video
    xor bx, bx
    mov ax, VIDEO_FONT_8x8
    int 10h                     ; Cargar font 8x8

    ; Imprimir mensaje de inicio en Modo Real
    print_text_rm start_rm_msg, start_rm_len, 0x1F, 2, 80

    ; Habilitar linea A20 para acceder a memoria extendida
    call A20_enable

    ; Cargar la GDT (Global Descriptor Table)
    LGDT [GDT_DESC]

    ; Activar modo protegido (PE bit en CR0)
    mov eax, CR0
    or eax, 1
    mov cr0, eax

    ; Saltar a modo protegido con far jump
    JMP CS_RING_0_SEL:modo_protegido

;; Seccion de Codigo - Modo Protegido
;; -------------------------------------------------------------------------- ;;
BITS 32
modo_protegido:
    ; Configurar registros de segmento para modo protegido
    mov ax, DS_RING_0_SEL
    mov DS, ax
    mov ES, ax
    mov GS, ax
    mov FS, ax
    mov SS, ax

    ; Inicializar stack del kernel
    mov esp, KERNEL_STACK_TOP
    mov ebp, esp

    ; Imprimir mensaje de inicio en Modo Protegido
    print_text_pm start_pm_msg, start_pm_len, 0x1F, 4, 1

    xchg bx, bx  ; Magic breakpoint para debugger

    ; Inicializar layout de pantalla
    call screen_draw_layout

    ; Configurar IDT (Interrupt Descriptor Table)
    call idt_init
    LIDT [IDT_DESC]

    ; Configurar PIC (Programmable Interrupt Controller)
    call pic_reset
    call pic_enable

    ; Activar interrupciones
    sti

    ; Probar syscalls de usuario (INT 88 y INT 98)
    int SYSCALL_TEST_1
    xchg bx, bx  ; Breakpoint despues de syscall 1

    int SYSCALL_TEST_2
    xchg bx, bx  ; Breakpoint despues de syscall 2

    ; Llenar registros con valores de debug antes del loop infinito
    mov eax, DEBUG_VALUE
    mov ebx, DEBUG_VALUE
    mov ecx, DEBUG_VALUE
    mov edx, DEBUG_VALUE

    ; Loop infinito
    jmp $

;; Includes
;; -------------------------------------------------------------------------- ;;
%include "a20.asm"

; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TALLER System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
; ==============================================================================

%include "print.mac"



global start
extern GDT


extern print_text_rm
extern A20_enable
extern GDT_DESC
extern GDT_IDX_CODE_0
extern screen_draw_layout
extern IDT_DESC
extern idt_init
extern pic_reset
extern pic_enable
extern pic_disable


; GDT_IDX_CODE_0 = 1
%define CS_RING_0_SEL (1 << 3)

%define DS_RING_0_SEL (3 << 3) 


BITS 16
;; Saltear seccion de datos
jmp start

;;
;; Seccion de datos.
;; -------------------------------------------------------------------------- ;;
start_rm_msg db     'Iniciando kernel en Modo Real'
start_rm_len equ    $ - start_rm_msg

start_pm_msg db     'Iniciando kernel en Modo Protegido'
start_pm_len equ    $ - start_pm_msg

;;
;; Seccion de código.
;; -------------------------------------------------------------------------- ;;

;; Punto de entrada del kernel.
BITS 16
start:
   
    cli ; desactiva interrup flag

    ; Cambiar modo de video a 80 X 50
    mov ax, 0003h
    int 10h ; set mode 03h
    xor bx, bx
    mov ax, 1112h
    int 10h ; load 8x8 font(3 << 3 )

    
    ; (revisar las funciones definidas en print.mac y los mensajes se encuentran en la
    ; sección de datos)
    
    print_text_rm start_rm_msg, start_rm_len , 0x1F , 2 ,80 ; fila 0 col 0  


    
    ; (revisar las funciones definidas en a20.asm)
    call A20_enable

    
    LGDT [GDT_DESC] 
    ;xchg bx,bx ;Magic breakpoint 

    
    mov eax, CR0
    or eax, 1
    mov cr0, eax
    
    JMP CS_RING_0_SEL: modo_protegido

BITS 32
modo_protegido:
    

    ;GDT_IDX_DATA_0 = 3
    mov ax, DS_RING_0_SEL
    mov DS, ax
    MOV es, ax
    mov gs, ax
    mov fs, ax
    mov ss, ax

    

    mov esp, 0x25000
    mov ebp, esp


    
    print_text_pm start_pm_msg, start_pm_len, 0x1F, 4, 1
    xchg bx,bx ;Magic breakpoint
    call screen_draw_layout 
    ;xchg bx,bx

    ;Iniciamos IDT

    call idt_init
    LIDT [IDT_DESC] 

    call pic_reset
    call pic_enable
    sti

    int 0x58

    xchg bx,bx

    int 0x62

    xchg bx,bx


    ; Ciclar infinitamente 
    mov eax, 0xFFFF
    mov ebx, 0xFFFF
    mov ecx, 0xFFFF
    mov edx, 0xFFFF
    jmp $

;; -------------------------------------------------------------------------- ;;

%include "a20.asm"

; ** por compatibilidad se omiten tildes **
; ==============================================================================
; Rutinas para habilitar/deshabilitar la linea A20
; ==============================================================================

BITS 16

;; Constantes - Puertos del Controlador de Teclado (8042)
;; -------------------------------------------------------------------------- ;;
%define KBD_STATUS_PORT     0x64    ; Puerto de estado/comando
%define KBD_DATA_PORT       0x60    ; Puerto de datos

;; Constantes - Comandos del Controlador 8042
;; -------------------------------------------------------------------------- ;;
%define KBD_CMD_DISABLE     0xAD    ; Deshabilitar teclado
%define KBD_CMD_ENABLE      0xAE    ; Habilitar teclado
%define KBD_CMD_READ_OUT    0xD0    ; Leer output port
%define KBD_CMD_WRITE_OUT   0xD1    ; Escribir output port

;; Constantes - Bits del Output Port
;; -------------------------------------------------------------------------- ;;
%define A20_BIT             0x02    ; Bit 1 = A20 gate

;; Constantes - Flags de Estado
;; -------------------------------------------------------------------------- ;;
%define KBD_STATUS_IBF      0x02    ; Input Buffer Full (bit 1)
%define KBD_STATUS_OBF      0x01    ; Output Buffer Full (bit 0)

;; Constantes - Colores y Posiciones
;; -------------------------------------------------------------------------- ;;
%define COLOR_NORMAL        0x07    ; Blanco sobre negro
%define COLOR_OK            0x0A    ; Verde brillante
%define COLOR_FAIL          0x0C    ; Rojo brillante
%define COL_STATUS          23      ; Columna para mensajes de estado
%define NO_INCREMENT        0xFFFF  ; No incrementar linea

section .text

;; Mensajes de Estado
;; -------------------------------------------------------------------------- ;;
enable_msg:     db 'Habilitando A20........'
enable_len      equ $ - enable_msg

disable_msg:    db 'Desabilitando A20......'
disable_len     equ $ - disable_msg

check_msg:      db 'Checkeando A20.........'
check_len       equ $ - check_msg

ok_msg:         db 'OK!'
ok_len          equ $ - ok_msg

fail_msg:       db 'FALLO!'
fail_len        equ $ - fail_msg

contadorlineas: dw 0x0000

;; Macro para imprimir en Modo Real
;; -------------------------------------------------------------------------- ;;
; Imprime un string en memoria de video (solo funciona en Modo Real)
;
; Parametros:
;   %1 - Mensaje (offset)
;   %2 - Longitud
;   %3 - Color (atributo)
;   %4 - Fila (0xFFFF = no incrementar contador)
;   %5 - Columna
%macro REAL_MODE_PRINT 5
    pusha
    push es

    ; Apuntar al segmento de video
    mov ax, 0xB800
    mov es, ax

    ; Manejar contador de lineas
    %if %4 <> dx
    mov dx, %4
    %endif
    cmp dx, NO_INCREMENT
    je %%skip_line_inc
    add WORD [contadorlineas], 0x0001
%%skip_line_inc:

    ; Calcular offset en memoria de video (fila * 80 + columna)
    mov ax, [contadorlineas]
    mov bx, 80
    mul bx
    mov bx, ax

    %if %5 <> dx
    mov dx, %5
    %endif
    add bx, dx
    shl bx, 1               ; Multiplicar por 2 (caracter + atributo)

    ; Preparar parametros para el loop
    %if %1 <> di
    mov di, %1
    %endif
    %if %2 <> cx
    mov cx, %2
    %endif
    %if %3 <> ah
    mov ah, %3
    %endif

%%print_loop:
    mov al, [di]            ; Leer caracter
    mov [es:bx], ax         ; Escribir caracter + atributo
    add bx, 2
    inc di
    loop %%print_loop

    pop es
    popa
%endmacro

;; Macro para operaciones del controlador A20
;; -------------------------------------------------------------------------- ;;
; Secuencia comun para habilitar/deshabilitar A20
; %1 - Operacion AND/OR para el bit A20 (and al, ~A20_BIT  o  or al, A20_BIT)
%macro A20_OPERATION 1
    call A20_wait

    ; Deshabilitar teclado
    mov al, KBD_CMD_DISABLE
    out KBD_STATUS_PORT, al

    call A20_wait

    ; Leer output port
    mov al, KBD_CMD_READ_OUT
    out KBD_STATUS_PORT, al

    call A20_wait2
    in al, KBD_DATA_PORT
    push ax

    call A20_wait

    ; Escribir output port
    mov al, KBD_CMD_WRITE_OUT
    out KBD_STATUS_PORT, al

    call A20_wait
    pop ax

    ; Modificar bit A20
    %1

    out KBD_DATA_PORT, al
    call A20_wait

    ; Habilitar teclado
    mov al, KBD_CMD_ENABLE
    out KBD_STATUS_PORT, al

    call A20_wait
%endmacro

;; Funcion: A20_disable
;; -------------------------------------------------------------------------- ;;
; Deshabilita la linea A20 usando el controlador de teclado
A20_disable:
    pushf
    pusha

    REAL_MODE_PRINT disable_msg, disable_len, COLOR_NORMAL, 0, 0
    A20_OPERATION and al, ~A20_BIT
    REAL_MODE_PRINT ok_msg, ok_len, COLOR_OK, NO_INCREMENT, COL_STATUS

    popa
    popf
    ret

;; Funcion: A20_enable
;; -------------------------------------------------------------------------- ;;
; Habilita la linea A20 usando el controlador de teclado
A20_enable:
    pushf
    pusha

    REAL_MODE_PRINT enable_msg, enable_len, COLOR_NORMAL, 0, 0
    A20_OPERATION or al, A20_BIT
    REAL_MODE_PRINT ok_msg, ok_len, COLOR_OK, NO_INCREMENT, COL_STATUS

    popa
    popf
    ret

;; Funcion: A20_wait
;; -------------------------------------------------------------------------- ;;
; Espera hasta que el buffer de entrada este vacio (IBF = 0)
A20_wait:
    in al, KBD_STATUS_PORT
    test al, KBD_STATUS_IBF
    jnz A20_wait
    ret

;; Funcion: A20_wait2
;; -------------------------------------------------------------------------- ;;
; Espera hasta que el buffer de salida este lleno (OBF = 1)
A20_wait2:
    in al, KBD_STATUS_PORT
    test al, KBD_STATUS_OBF
    jz A20_wait2
    ret

;; Funcion: A20_check
;; -------------------------------------------------------------------------- ;;
; Verifica si la linea A20 esta habilitada
; Retorna: AX = 0 si A20 esta deshabilitada, 1 si esta habilitada
A20_check:
    pushf
    push fs
    push gs
    push di
    push si

    REAL_MODE_PRINT check_msg, check_len, COLOR_NORMAL, 0, 0

    ; Configurar segmentos para test
    xor ax, ax          ; FS apunta a 0x0000:0x0500
    mov fs, ax
    not ax              ; GS apunta a 0xFFFF:0x0510 (wrap around si A20 deshabilitada)
    mov gs, ax

    mov di, 0x0500
    mov si, 0x0510

    ; Guardar valores originales
    mov al, byte [fs:di]
    push ax
    mov al, byte [gs:si]
    push ax

    ; Test: escribir valores diferentes
    mov byte [fs:di], 0x00
    mov byte [gs:si], 0xFF

    ; Verificar si hubo wrap-around (A20 deshabilitada)
    cmp byte [fs:di], 0xFF
    je .fallo
        REAL_MODE_PRINT ok_msg, ok_len, COLOR_OK, NO_INCREMENT, COL_STATUS
        jmp .restore
    .fallo:
        REAL_MODE_PRINT fail_msg, fail_len, COLOR_FAIL, NO_INCREMENT, COL_STATUS

    .restore:
    ; Restaurar valores originales
    pop ax
    mov byte [gs:si], al
    pop ax
    mov byte [fs:di], al

    ; Retornar resultado
    mov ax, 0
    je .exit
    mov ax, 1

    .exit:
    pop si
    pop di
    pop gs
    pop fs
    popf
    ret

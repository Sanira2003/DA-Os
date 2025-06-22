; Simple bootloader (boot.asm)
[org 0x7C00]        ; BIOS loads bootloader at 0x7C00
[bits 16]

; Set up stack
xor ax, ax
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7C00      ; Stack grows downward from 0x7C00

; Load kernel from disk
mov ah, 0x02        ; BIOS read sector function
mov al, 10          ; Number of sectors to read (kernel size)
mov ch, 0           ; Cylinder 0
mov dh, 0           ; Head 0
mov cl, 2           ; Start from sector 2 (after bootloader)
mov bx, 0x7E00      ; Load kernel at 0x7E00 (right after bootloader)
int 0x13            ; BIOS disk interrupt

jc disk_error       ; Jump if error

; Jump to kernel
jmp 0x7E00

disk_error:
mov si, error_msg
call print_string
jmp $

print_string:
lodsb               ; Load next byte from SI into AL
or al, al           ; Check for null terminator
jz .done
mov ah, 0x0E        ; BIOS teletype function
int 0x10            ; BIOS video interrupt
jmp print_string
.done:
ret

error_msg db "Disk error!", 0

; Boot signature
times 510-($-$$) db 0
dw 0xAA55
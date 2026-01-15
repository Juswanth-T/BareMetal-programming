[BITS 16]
[ORG 0x7C00]

start:
    ; Setup segments and stack
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Load game from disk to 0x1000 (DL already has boot drive from BIOS)
    mov ah, 0x02        ; Read sectors
    mov al, 8           ; 8 sectors
    mov cx, 0x0002      ; Cylinder 0, Sector 2
    mov dh, 0           ; Head 0
    mov bx, 0x1000      ; Destination
    int 0x13
    jc error

    ; Set VGA mode 13h (320x200, 256 colors)
    mov ax, 0x0013
    int 0x10

    ; Enable A20 line
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Load GDT and switch to protected mode
    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x08:pmode

error:
    mov ax, 0x0E45      ; Print 'E'
    int 0x10
    hlt

; GDT
align 8
gdt:
    dq 0                                    ; Null
    dq 0x00CF9A000000FFFF                   ; Code: base=0, limit=4GB, 32-bit
    dq 0x00CF92000000FFFF                   ; Data: base=0, limit=4GB, 32-bit
gdt_desc:
    dw 23                                   ; Size (3 entries * 8 - 1)
    dd gdt

[BITS 32]
pmode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000
    jmp 0x1000

times 510-($-$$) db 0
dw 0xAA55

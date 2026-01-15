# Running Code Without an OS

**What if you could delete Windows, Linux, and every OS from existence... and still run your code?**

This project does exactly that. A game that boots directly on your CPU - no operating system, no drivers, no safety net. Just raw machine code talking directly to hardware.

Built from scratch because I wanted to understand what actually happens when you press the power button.

---

## What is Baremetal Programming?

When you run a normal program, there's a whole tower of abstraction beneath you:

```
  Your Code
      ↓
  C Runtime Library
      ↓
  Operating System (Windows/Linux)
      ↓
  Drivers
      ↓
  Actual Hardware
```

**Baremetal programming throws all of that away.** Your code runs directly on the CPU, one level above the silicon itself.

When you turn on a computer:

1. The BIOS/UEFI initializes hardware
2. It looks for a bootable device (HDD, USB, Floppy)
3. Loads the first 512 bytes (boot sector) into memory at address `0x7C00`
4. Jumps to that address and starts executing

In baremetal programming, YOU write that code. There's no OS to help you - no `printf()`, no file system, no memory management. You talk directly to hardware through:

- **I/O Ports** - Reading/writing to hardware registers
- **Memory-mapped I/O** - Hardware accessible through memory addresses
- **BIOS Interrupts** - Low-level services (only in Real Mode)

---

## What I built

This project implements:

1. **Custom Bootloader** (512 bytes) - Written in x86 Assembly
   - Loads game code from disk
   - Switches CPU from 16-bit Real Mode to 32-bit Protected Mode
   - Sets up VGA graphics mode (320x200, 256 colors)
   - Jumps to game code

2. **Pong-style Game** - Written in C++
   - Direct VGA framebuffer manipulation
   - Keyboard input via I/O ports
   - No standard library - everything from scratch

---

## How It Works

### Boot Process

```
┌─────────────────────────────────────────────────────────────┐
│  BIOS loads bootloader.bin (512 bytes) to 0x7C00            │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│  Bootloader:                                                │
│  1. Sets up segment registers                               │
│  2. Loads game.bin from disk to 0x1000                      │
│  3. Switches to VGA Mode 13h (320x200, 256 colors)          │
│  4. Enables A20 line (access memory > 1MB)                  │
│  5. Sets up GDT (Global Descriptor Table)                   │
│  6. Switches to 32-bit Protected Mode                       │
│  7. Jumps to game code at 0x1000                            │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│  Game Code:                                                 │
│  - Runs in 32-bit Protected Mode                            │
│  - Writes directly to VGA memory at 0xA0000                 │
│  - Reads keyboard via port 0x60                             │
└─────────────────────────────────────────────────────────────┘
```

### Memory Layout

| Address     | Content                    |
|-------------|----------------------------|
| `0x0000`    | Interrupt Vector Table     |
| `0x1000`    | Game code (loaded here)    |
| `0x7C00`    | Bootloader (BIOS loads here)|
| `0x90000`   | Stack (grows downward)     |
| `0xA0000`   | VGA Framebuffer            |

### Real Mode vs Protected Mode

| Feature          | Real Mode (16-bit) | Protected Mode (32-bit) |
|------------------|-------------------|-------------------------|
| Registers        | AX, BX, CX, DX    | EAX, EBX, ECX, EDX      |
| Memory Access    | 1 MB (segmented)  | 4 GB (flat)             |
| BIOS Interrupts  | Available         | Not available           |
| Code Type        | 16-bit            | 32-bit (GCC output)     |

The bootloader starts in Real Mode (required by BIOS), then switches to Protected Mode so we can run 32-bit C++ code.

### VGA Mode 13h

- Resolution: 320 x 200 pixels
- Colors: 256 (8-bit palette)
- Framebuffer: Linear at `0xA0000`
- Each byte = one pixel's color index

```cpp
// Writing a red pixel at (x, y)
unsigned char* vga = (unsigned char*)0xA0000;
vga[y * 320 + x] = 4;  // Color 4 = Red
```

### Keyboard Input

```cpp
// Check if key is available
if (inb(0x64) & 1) {
    unsigned char scancode = inb(0x60);
    // Process scancode...
}
```

- Port `0x64` - Keyboard status (bit 0 = data available)
- Port `0x60` - Keyboard data (scancode)

---

## Requirements

### Tools Needed

| Tool    | Purpose                      | Install (MSYS2)              |
|---------|------------------------------|------------------------------|
| NASM    | Assembler for bootloader     | `pacman -S nasm`             |
| GCC     | C++ compiler (32-bit)        | `pacman -S mingw-w64-ucrt-x86_64-gcc` |
| LD      | Linker                       | (included with GCC)          |
| OBJCOPY | Binary extraction            | (included with binutils)     |
| QEMU    | x86 emulator for testing     | `pacman -S mingw-w64-ucrt-x86_64-qemu` |


---

## Building & Running

### Build

```batch
build.bat
```

This will:
1. Assemble `bootloader.asm` → `bootloader.bin`
2. Compile `game.cpp` → `game.o`
3. Link `game.o` → `game.pe`
4. Extract binary `game.pe` → `game.bin`
5. Concatenate `bootloader.bin` + `game.bin` → `disk.img`

### Run

```batch
qemu-system-i386 -fda disk.img
```

### Controls

| Key         | Action           |
|-------------|------------------|
| Left Arrow  | Move paddle left |
| Right Arrow | Move paddle right|
| Enter       | Restart (after game over) |

---

## Technical Challenges Solved

### 1. Windows Toolchain Compatibility
- Windows GCC produces PE/COFF format, not ELF
- Solution: Use `ld -m i386pe` then `objcopy -O binary`

### 2. 16-bit to 32-bit Transition
- Can't run 32-bit GCC code in 16-bit Real Mode
- Solution: Bootloader sets up GDT and switches to Protected Mode

### 3. Screen Flickering
- Clearing entire screen each frame causes flicker
- Solution: Only erase/redraw changed regions (dirty rectangles)

---

## Key Learnings

1. **CPU Modes Matter** - x86 starts in 16-bit mode; need explicit switch to 32-bit
2. **No Standard Library** - Everything must be implemented from scratch
3. **Direct Hardware Access** - I/O ports and memory-mapped registers
4. **Boot Sector Constraints** - Must fit in 512 bytes with magic signature `0xAA55`


*Built for fun and learning. No operating system was harmed in the making of this project.*

// Port I/O functions
static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// VGA framebuffer address
#define VGA ((unsigned char*)0xA0000)

// Fill screen with a color
void fill_screen(unsigned char color) {
    unsigned char* vga = VGA;
    for (int i = 0; i < 320 * 200; i++) {
        vga[i] = color;
    }
}

// Draw a filled rectangle
void draw_rect(int x, int y, int width, int height, unsigned char color) {
    unsigned char* vga = VGA;
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && px < 320 && py >= 0 && py < 200) {
                vga[py * 320 + px] = color;
            }
        }
    }
}

// Entry point - bootloader jumps here
extern "C" __attribute__((section(".text.startup"))) void _start() {
    int player_x = 160;
    int player_y = 100;
    int player_size = 8;
    int player_speed = 2;

    // Game loop
    while (1) {
        // Read keyboard scan code
        unsigned char scancode = inb(0x60);

        // Arrow key movement
        if (scancode == 0x48 && player_y > 0)
            player_y -= player_speed;
        if (scancode == 0x50 && player_y < 200 - player_size)
            player_y += player_speed;
        if (scancode == 0x4B && player_x > 0)
            player_x -= player_speed;
        if (scancode == 0x4D && player_x < 320 - player_size)
            player_x += player_speed;

        // Clear screen and draw player
        fill_screen(0);
        draw_rect(player_x, player_y, player_size, player_size, 15);

        // Frame delay
        for (volatile int i = 0; i < 50000; i++);
    }
}

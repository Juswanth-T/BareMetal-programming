// Port I/O
static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// VGA
#define VGA ((unsigned char*)0xA0000)

// Draw filled rectangle
void rect(int x, int y, int w, int h, unsigned char c) {
    unsigned char* v = VGA;
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            int px = x + i;
            int py = y + j;
            if (px >= 0 && px < 320 && py >= 0 && py < 200) {
                v[py * 320 + px] = c;
            }
        }
    }
}

// Clear screen
void cls(unsigned char c) {
    unsigned char* v = VGA;
    for (int i = 0; i < 64000; i++) v[i] = c;
}

// Entry point
extern "C" __attribute__((section(".text.startup"))) void _start() {
    // Paddle
    int px = 140;    // paddle x
    int pw = 60;     // paddle width
    int ph = 8;      // paddle height
    int py = 185;    // paddle y (near bottom)

    // Ball
    int bx = 160;    // ball x
    int by = 100;    // ball y
    int bw = 6;      // ball size
    int bdx = 2;     // ball x velocity
    int bdy = 2;     // ball y velocity

    // Score
    int score = 0;
    int playing = 1;
    int ball_timer = 0;

    // Main loop
    while (1) {
        // Input - check keyboard status first
        if (inb(0x64) & 1) {
            unsigned char key = inb(0x60);
            if (!(key & 0x80)) {  // key press only
                if (key == 0x4B) px -= 25;       // Left arrow
                else if (key == 0x4D) px += 25;  // Right arrow
                else if (key == 0x1C && !playing) {  // Enter to restart
                    bx = 160;
                    by = 100;
                    bdx = 2;
                    bdy = 2;
                    score = 0;
                    playing = 1;
                }
            }
        }

        // Keep paddle in bounds
        if (px < 0) px = 0;
        if (px > 320 - pw) px = 320 - pw;

        // Update ball
        ball_timer++;
        if (playing && (ball_timer % 15 == 0)) {
            bx += bdx;
            by += bdy;

            // Ball hits left/right walls
            if (bx <= 0 || bx >= 320 - bw) {
                bdx = -bdx;
                if (bx <= 0) bx = 0;
                if (bx >= 320 - bw) bx = 320 - bw;
            }

            // Ball hits top
            if (by <= 0) {
                bdy = -bdy;
                by = 0;
            }

            // Ball hits paddle
            if (by + bw >= py && by <= py + ph) {
                if (bx + bw >= px && bx <= px + pw) {
                    bdy = -bdy;
                    by = py - bw;
                    score++;
                    // Speed up slightly
                    if (bdx > 0 && bdx < 6) bdx++;
                    else if (bdx < 0 && bdx > -6) bdx--;
                }
            }

            // Ball falls off bottom - game over
            if (by > 200) {
                playing = 0;
            }
        }

        // Draw
        cls(0);  // black background

        // Draw border
        rect(0, 0, 320, 2, 8);      // top
        rect(0, 0, 2, 200, 8);      // left
        rect(318, 0, 2, 200, 8);    // right

        // Draw score as bars at top
        for (int i = 0; i < score && i < 30; i++) {
            rect(10 + i * 10, 5, 8, 4, 14);  // yellow bars
        }

        // Draw paddle (white)
        rect(px, py, pw, ph, 15);

        // Draw ball (red)
        rect(bx, by, bw, bw, 4);

        // Game over indicator
        if (!playing) {
            // Flash border red
            rect(0, 0, 320, 2, 4);
            rect(0, 0, 2, 200, 4);
            rect(318, 0, 2, 200, 4);
            rect(0, 198, 320, 2, 4);
        }

        // Delay
        for (volatile int i = 0; i < 5000; i++);
    }
}

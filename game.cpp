// ============================================
// Port I/O
// ============================================

static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}


// ============================================
// Graphics
// ============================================

#define VGA ((unsigned char*)0xA0000)

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

void draw_border(unsigned char c) {
    rect(0, 0, 320, 5, c);
    rect(0, 0, 5, 200, c);
    rect(315, 0, 5, 200, c);
}

void draw_score(int score) {
    for (int i = 0; i < score && i < 25; i++) {
        rect(12 + i * 12, 8, 10, 8, 14);
    }
}


// ============================================
// Game Entry Point
// ============================================

extern "C" __attribute__((section(".text.startup"))) void _start() {

    // Boundaries
    int left = 6;
    int right = 314;
    int top = 20;

    // Paddle
    int px = 130;
    int pw = 60;
    int ph = 10;
    int py = 175;

    // Ball
    int bx = 155;
    int by = 80;
    int bw = 10;
    int bdx = 1;
    int bdy = 1;

    // State
    int score = 0;
    int playing = 1;
    int frame = 0;

    // Init screen
    unsigned char* v = VGA;
    for (int i = 0; i < 64000; i++) v[i] = 0;
    draw_border(7);
    rect(px, py, pw, ph, 15);


    // ========================================
    // Main Loop
    // ========================================

    while (1) {
        frame++;

        // Input
        if (inb(0x64) & 1) {
            unsigned char key = inb(0x60);
            if (!(key & 0x80)) {
                if (key == 0x4B && px > left) {
                    rect(px + pw - 10, py, 10, ph, 0);
                    px -= 10;
                    if (px < left) px = left;
                    rect(px, py, pw, ph, 15);
                }
                else if (key == 0x4D && px + pw < right) {
                    rect(px, py, 10, ph, 0);
                    px += 10;
                    if (px + pw > right) px = right - pw;
                    rect(px, py, pw, ph, 15);
                }
                else if (key == 0x1C && !playing) {
                    rect(bx, by, bw, bw, 0);
                    bx = 155; by = 80;
                    bdx = 1; bdy = 1;
                    score = 0;
                    playing = 1;
                    draw_border(7);
                    rect(6, 6, 300, 12, 0);
                }
            }
        }

        // Ball update
        if (playing && (frame % 800 == 0)) {
            rect(bx, by, bw, bw, 0);

            bx += bdx;
            by += bdy;

            if (bx < left) { bx = left; bdx = 1; }
            if (bx + bw > right) { bx = right - bw; bdx = -1; }
            if (by < top) { by = top; bdy = 1; }

            if (by + bw >= py && by + bw <= py + ph + 5) {
                if (bx + bw > px && bx < px + pw) {
                    by = py - bw;
                    bdy = -1;
                    score++;
                    draw_score(score);
                }
            }

            if (by + bw > 195) {
                playing = 0;
                draw_border(4);
                rect(0, 195, 320, 5, 4);
            }

            rect(bx, by, bw, bw, 12);
        }

        for (volatile int i = 0; i < 1000; i++);
    }
}

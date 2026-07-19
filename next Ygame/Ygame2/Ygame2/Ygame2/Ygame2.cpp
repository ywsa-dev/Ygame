#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GRID 10
#define MAX_Y 20

struct YObj {
    int x, y;
    int dx, dy;
    bool passed;
};

void gotoxy(int x, int y) {
    COORD c = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO ci = { 1, FALSE };
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);
}

void setConsoleSize(int cols, int lines) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD size = { (SHORT)cols, (SHORT)lines };
    SetConsoleScreenBufferSize(h, size);
    SMALL_RECT rect = { 0, 0, (SHORT)(cols - 1), (SHORT)(lines - 1) };
    SetConsoleWindowInfo(h, TRUE, &rect);
}

void getIniPath(char *out) {
    GetModuleFileNameA(NULL, out, MAX_PATH);
    char *last = strrchr(out, '\\');
    if (last) *(last + 1) = '\0';
    strcat(out, "set.ini");
}

int loadIniInt(const char *key, int def) {
    char path[MAX_PATH];
    getIniPath(path);
    FILE *f = fopen(path, "r");
    if (!f) return def;
    char line[64];
    char pattern[32];
    sprintf(pattern, "%s=%%d", key);
    while (fgets(line, sizeof(line), f)) {
        int val;
        if (sscanf(line, pattern, &val) == 1) {
            fclose(f);
            return (val > 0) ? val : def;
        }
    }
    fclose(f);
    return def;
}

void spawnY(YObj &obj) {
    int side = rand() % 4;
    if (side == 0) { obj.x = rand() % GRID; obj.y = -1; obj.dx = 0; obj.dy = 1; }
    else if (side == 1) { obj.x = rand() % GRID; obj.y = GRID; obj.dx = 0; obj.dy = -1; }
    else if (side == 2) { obj.x = -1; obj.y = rand() % GRID; obj.dx = 1; obj.dy = 0; }
    else { obj.x = GRID; obj.y = rand() % GRID; obj.dx = -1; obj.dy = 0; }
    obj.passed = false;
}

char yChar(const YObj &obj) {
    if (obj.dx == 1)  return '>';
    if (obj.dx == -1) return '<';
    return 'Y';
}

#define CON_TOP 2

void drawAll(int px, int py, YObj ys[], int ycount, int score, int secsLeft) {
    char grid[GRID][GRID];
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++) grid[r][c] = ' ';

    for (int i = 0; i < ycount; i++) {
        int gx = ys[i].x, gy = ys[i].y;
        if (gx >= 0 && gx < GRID && gy >= 0 && gy < GRID) grid[gy][gx] = yChar(ys[i]);
    }
    grid[py][px] = 'O';

    gotoxy(0, 0); printf("  SCORE: %-5d", score);
    gotoxy(0, 1); printf(" X");
    for (int c = 0; c < GRID; c++) printf("--");
    printf("X");
    for (int r = 0; r < GRID; r++) {
        gotoxy(0, CON_TOP + r); printf(" |");
        for (int c = 0; c < GRID; c++) printf("%c ", grid[r][c]);
        printf("|");
    }
    gotoxy(0, CON_TOP + GRID); printf(" X");
    for (int c = 0; c < GRID; c++) printf("--");
    printf("X");
    gotoxy(0, CON_TOP + GRID + 2); printf("  second: %-3d    ", secsLeft);
}

int main() {
    srand((unsigned)time(NULL));
    SetConsoleTitle("CMD SYSTEM GAME");
    setConsoleSize(40, 20);
    hideCursor();

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hIn, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

restart:;
    system("cls");
    int timeLimit = loadIniInt("time", 5);
    int ycount = loadIniInt("Ycount", 5);
    if (ycount > MAX_Y) ycount = MAX_Y;
    DWORD LIMIT = (DWORD)(timeLimit * 1000);
    int px = GRID / 2, py = GRID / 2;
    int score = 0;
    YObj ys[MAX_Y];
    for (int i = 0; i < ycount; i++) spawnY(ys[i]);

    FlushConsoleInputBuffer(hIn);
    DWORD lastKey = GetTickCount();
    drawAll(px, py, ys, ycount, score, timeLimit);

    while (true) {
        DWORD now = GetTickCount();
        DWORD elapsed = now - lastKey;
        int secsLeft = (int)((int)LIMIT - (int)elapsed) / 1000 + 1;
        if (secsLeft < 1) secsLeft = 1;
        gotoxy(0, CON_TOP + GRID + 2);
        printf("  second: %-3d    ", secsLeft);
        if ((int)elapsed >= (int)LIMIT) break;

        if (_kbhit()) {
            int key = _getch();
            int dx = 0, dy = 0;
            if (key == 'w' || key == 'W') dy = -1;
            else if (key == 's' || key == 'S') dy = 1;
            else if (key == 'a' || key == 'A') dx = -1;
            else if (key == 'd' || key == 'D') dx = 1;
            else if (key == 0 || key == 0xE0) {
                int ext = _getch();
                if (ext == 72) dy = -1; else if (ext == 80) dy = 1;
                else if (ext == 75) dx = -1; else if (ext == 77) dx = 1;
            }

            if (dx != 0 || dy != 0) {
                int nextX = px + dx, nextY = py + dy;
                // 벽에 막혔는지 확인 (막혔으면 아무것도 실행 안 함)
                if (nextX >= 0 && nextX < GRID && nextY >= 0 && nextY < GRID) {
                    px = nextX; py = nextY;
                    bool anyPassed = false;
                    for (int i = 0; i < ycount; i++) {
                        ys[i].x += ys[i].dx; ys[i].y += ys[i].dy;
                        if (ys[i].x >= 0 && ys[i].x < GRID && ys[i].y >= 0 && ys[i].y < GRID) ys[i].passed = true;
                        bool out = (ys[i].x < 0 || ys[i].x >= GRID || ys[i].y < 0 || ys[i].y >= GRID);
                        if (out && ys[i].passed) { anyPassed = true; spawnY(ys[i]); }
                    }
                    if (anyPassed) score++;
                    lastKey = GetTickCount();
                    bool hit = false;
                    for (int i = 0; i < ycount; i++) if (ys[i].x == px && ys[i].y == py) hit = true;
                    drawAll(px, py, ys, ycount, score, timeLimit);
                    if (hit) goto gameover;
                }
            }
        }
        Sleep(16);
    }
gameover:
    system("cls");
    printf("  FINAL SCORE: %d\n  AGAIN? (Press any key)\n", score);
    _getch();
    goto restart;
    return 0;
}
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_Y 10

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

void drawGame(int px, int yy, int yx[], int ycount, int score, int secondsLeft) {
    gotoxy(0, 0);
    // 너비: 1 + 10*4 = 41자 → 여유있게 50으로
    printf("==============================================\n");
    printf("  SCORE: %-5d                               \n", score);
    printf("==============================================\n");

    for (int i = 0; i <= 8; i++) {
        printf(" ");
        for (int j = 0; j <= 9; j++) {
            if (i == yy) {
                bool found = false;
                for (int k = 0; k < ycount; k++)
                    if (j == yx[k]) found = true;
                printf(found ? "Y   " : "    ");
            } else {
                printf("    ");
            }
        }
        printf("\n");
    }

    // 플레이어
    printf(" ");
    for (int j = 0; j <= 9; j++)
        printf(j == px ? "O   " : "    ");
    printf("\n");

    printf(" XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    printf("==============================================\n");
    printf(" second: %d   \n", secondsLeft);
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

void initY(int yx[], int ycount) {
    for (int i = 0; i < ycount; i++)
        yx[i] = rand() % 10;
}

int main() {
    srand((unsigned)time(NULL));
    SetConsoleTitle("CMD SYSTEM GAME");
    setConsoleSize(50, 19);
    hideCursor();

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hIn, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

restart:;
    int timeLimit = loadIniInt("time", 5);
    int ycount    = loadIniInt("Ycount", 5);
    if (ycount > MAX_Y) ycount = MAX_Y;

    DWORD LIMIT = (DWORD)(timeLimit * 1000);

    int px = 5;
    int score = 0;
    int yy = 0;
    int yx[MAX_Y];
    initY(yx, ycount);

    FlushConsoleInputBuffer(hIn);
    DWORD startTime = GetTickCount();
    drawGame(px, yy, yx, ycount, score, timeLimit);

    while (true) {
        DWORD elapsed = GetTickCount() - startTime;
        int secsLeft = (int)((LIMIT - (int)elapsed) / 1000) + 1;
        if (secsLeft < 1) secsLeft = 1;

        gotoxy(0, 15);
        printf(" second: %d   ", secsLeft);
        fflush(stdout);

        if ((int)elapsed >= (int)LIMIT)
            break;

        if (_kbhit()) {
            int key = _getch();
            int newPx = px;
            bool valid = false;

            if (key == 'a' || key == 'A') {
                if (px > 0) { newPx = px - 1; valid = true; }
            } else if (key == 'd' || key == 'D') {
                if (px < 9) { newPx = px + 1; valid = true; }
            } else if (key == 0 || key == 0xE0) {
                int ext = _getch();
                if (ext == 75) {
                    if (px > 0) { newPx = px - 1; valid = true; }
                } else if (ext == 77) {
                    if (px < 9) { newPx = px + 1; valid = true; }
                }
            }

            if (valid) {
                px = newPx;
                yy++;
                startTime = GetTickCount();

                if (yy > 8) {
                    yy = 0;
                    initY(yx, ycount);
                    score++;
                }

                if (yy == 8) {
                    bool hit = false;
                    for (int k = 0; k < ycount; k++)
                        if (px == yx[k]) hit = true;
                    if (hit) {
                        drawGame(px, yy, yx, ycount, score, 0);
                        goto gameover;
                    }
                }

                drawGame(px, yy, yx, ycount, score, timeLimit);
            }
        }

        Sleep(20);
    }

gameover:
    system("cls");
    printf("==============================================\n");
    printf("                    FAIL                     \n");
    printf("==============================================\n");
    printf("  FINAL SCORE: %d\n", score);
    printf("==============================================\n");
    printf("  AGAIN? (Press any key)\n");
    printf("==============================================\n");
    FlushConsoleInputBuffer(hIn);
    _getch();
    system("cls");
    goto restart;

    return 0;
}
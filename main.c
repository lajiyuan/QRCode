#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define QR_SIZE 21
#define PRINT_SIZE 31
#define GF_SIZE 256
#define DATACODE_SIZE 129
#define DEBUG 0

int mat[QR_SIZE][QR_SIZE];
int vis[QR_SIZE][QR_SIZE];
int output[PRINT_SIZE][PRINT_SIZE];
int GF256[GF_SIZE], RGF256[GF_SIZE];

int posMat[9][9];
int errorCodeNum[12];

char dataCode[DATACODE_SIZE];
char errorCodeChar[DATACODE_SIZE];
const char blue[] = "101010000010010";
const char dict[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";
const int codePoly[] = {0, 251, 67, 46, 61, 118, 70, 64, 94, 32, 45};
const int bluePos[2][15][2] = {
        8, 0, 8, 1, 8, 2, 8, 3, 8, 4, 8, 5, 8, 7, 8, 8, 7, 8, 5, 8, 4, 8, 3, 8, 2, 8, 1, 8, 0, 8,
        20, 8, 19, 8, 18, 8, 17, 8, 16, 8, 15, 8, 14, 8, 8, 13, 8, 14, 8, 15, 8, 16, 8, 17, 8, 18, 8, 19, 8, 20
};

void PrepareDingWei() {
    for (int i = 1; i < 8; i++) {
        for (int j = 1; j < 8; j++) {
            if (i == 1 || j == 1 || i == 7 || j == 7) {
                posMat[i][j] = 1;
            }
        }
    }
    for (int i = 3; i <= 5; i++) {
        for (int j = 3; j <= 5; j++) {
            posMat[i][j] = 1;
        }
    }
}

void DrawDingWei(int x1, int y1, int x2, int y2) {
    for (int i = x1, i2 = x2; i < 9; i++, i2++) {
        if (i2 >= QR_SIZE) continue;
        for (int j = y1, j2 = y2; j < 9; j++, j2++) {
            if (j2 >= QR_SIZE) continue;
            mat[i2][j2] = posMat[i][j];
            vis[i2][j2] = 1;
        }
    }
}

// 最终的打印，显示构造二维码矩阵，然后用一个大的矩阵包住小矩阵
void Print() {
    int sum = 0;
    for (int i = 0; i < QR_SIZE; i++) {
        for (int j = 0; j < QR_SIZE; j++) {
            if (vis[i][j]) {
                sum++;
            }
        }
    }
    int id = 0;
    int x = QR_SIZE - 1, y = QR_SIZE - 1, flag = 1;
    for (int j = QR_SIZE - 1; j >= 0; j--) {
        int pos = 0;
        if (flag == 1) {
            for (;;) {
                mat[x][y] = dataCode[id++] - '0';
                if (pos == 0) {
                    y--;
                } else {
                    if (x == 0) {
                        y--;
                        flag = 0;
                        break;
                    } else if (vis[x - 1][y + 1]) {
                        if (!vis[x - 2][y + 1]) {
                            x -= 2;
                            y += 1;
                        } else {
                            y--;
                            flag = 0;
                            break;
                        }
                    } else {
                        x--;
                        y++;
                    }
                }
                pos = 1 - pos;
            }
        } else {
            for (;;) {
                mat[x][y] = dataCode[id++] - '0';
                if (pos == 0) {
                    y--;
                } else {
                    if (x == QR_SIZE - 1) {
                        if (y == 9) {
                            x = 12;
                            y--;
                            flag = 1;
                            break;
                        } else {
                            y--;
                            flag = 1;
                            break;
                        }
                    } else if (vis[x + 1][y + 1]) {
                        if (!vis[x + 2][y + 1]) {
                            x += 2;
                            y += 1;
                        } else {
                            flag = 1;
                            break;
                        }
                    } else {
                        x += 1;
                        y += 1;
                    }
                }
                pos = 1 - pos;
            }
        }
        if (j == 8) j -= 2, y--;
        else j--;
    }
    for (int i = 0; i < QR_SIZE; i++) {
        for (int j = 0; j < QR_SIZE; j++) {
            if (!vis[i][j] && (i + j) % 2 == 0) {
                mat[i][j] = 1 - mat[i][j];
            }
        }
    }

    char black = ' ';
    char white = 219;
    for (int i = 0; i < PRINT_SIZE; i++) {
        for (int j = 0; j < PRINT_SIZE; j++) {
            output[i][j] = 0;
        }
    }

    for (int i = 0; i < QR_SIZE; i++) {
        for (int j = 0; j < QR_SIZE; j++) {
            output[i + 5][j + 5] = mat[i][j];
        }
    }

    for (int i = 0; i < PRINT_SIZE; i++) {
        for (int j = 0; j < PRINT_SIZE; j++) {
            if (output[i][j]) {
                printf("%c%c", black, black);
            } else {
                printf("%c%c", white, white);
            }
        }
        printf("\n");
    }
}

// 画出二维码中固定的部分，例如定位符和掩码

void DrawBackground() {
    // 准备定位符的一份矩阵，然后直接copy三份到左上，右上，右下
    PrepareDingWei();
    DrawDingWei(1, 1, 0, 0);
    DrawDingWei(1, 0, 0, 13);
    DrawDingWei(0, 1, 13, 0);


    // 开始画背景中的掩码
    {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 15; j++) {
                vis[bluePos[i][j][0]][bluePos[i][j][1]] = 1;
                mat[bluePos[i][j][0]][bluePos[i][j][1]] = blue[j] - '0';
            }
        }
        for (int i = 8; i <= 12; i++) {
            mat[6][i] = !((i + 6) & 1);
            vis[6][i] = 1;
            mat[i][6] = !((i + 6) & 1);
            vis[i][6] = 1;
        }
    }
}


// 查找字符c所对应的权值
int FindIndex(char c) {
    int len = strlen(dict);
    for (int i = 0; i < len; i++) {
        if (dict[i] == c) {
            return i;
        }
    }
    return -1;
}

// 得到数值num所对应的len长度的二进制编码
char *GetLenBinString(int num, int len) {
    char *rtn = (char *) malloc(len + 1);
    rtn[len] = '\0';
    int idx = len - 1;
    while (idx >= 0) {
        rtn[idx] = (num % 2) + '0';
        num /= 2;
        idx--;
    }
    return rtn;
}

// 得到数据的数据码 数据码 + 补码
void GetDataCode(char *text) {
    int len = strlen(text);
    char headCode[14] = "0010";
    char *textLen = GetLenBinString(strlen(text), 9);
    strcat(headCode, textLen);
    strcat(dataCode, headCode);
    for (int i = 0; i < len - 1; i += 2) {
        char *result = GetLenBinString(FindIndex(text[i]) * 45 + FindIndex(text[i + 1]), 11);
        strcat(dataCode, result);
        free(result);
    }
    if (len & 1) {
        char *result = GetLenBinString(FindIndex(text[len - 1]), 6);
        strcat(dataCode, result);
        free(result);
    }
    if (DATACODE_SIZE - strlen(dataCode) >= 4) {
        strcat(dataCode, "0000");
    }
    int currentLen = strlen(dataCode);
    while (currentLen % 8 != 0) {
        strcat(dataCode, "0");
        currentLen++;
    }
    char appendCode1[9] = "11101100";
    char appendCode2[9] = "00010001";
    while (currentLen < 128) {
        strcat(dataCode, appendCode1);
        currentLen += 8;
        if (currentLen < 128) {
            strcat(dataCode, appendCode2);
            currentLen += 8;
        }
    }
}

void PrepareGF256() {
    GF256[0] = 1;
    RGF256[1] = 0;
    for (int i = 1; i < GF_SIZE; i++) {
        GF256[i] = GF256[i - 1] * 2;
        if (GF256[i] > 255) {
            GF256[i] = (GF256[i] ^ 285);
        }
        RGF256[GF256[i]] = i;
    }
}

// Reed-Solomon纠错码 “HELLO WORLD” -> 196  35  39  119  235  215  231  226  93  23
// 算法细节参考: https://www.thonky.com/qr-code-tutorial/error-correction-coding
void GetErrorCode() {
    int infoPolyCoefficient[30];
    for (int i = 0; i <= 25; i++) {
        infoPolyCoefficient[i] = 0;
    }
    for (int i = 0; i < strlen(dataCode); i += 8) {
        int num = 0;
        for (int j = i; j < i + 8; j++) {
            num = num * 2 + (int) (dataCode[j] - '0');
        }
        infoPolyCoefficient[i / 8] = num;
    }
    int GenPolyCoefficient[15];
    for (int i = 0; i <= 10; i++) {
        GenPolyCoefficient[i] = codePoly[i];
    }
    for (int i = 0; i < 16; i++) {
        int mul = RGF256[infoPolyCoefficient[i]];
        for (int j = i; j <= i + 10; j++) {
            infoPolyCoefficient[j] = (infoPolyCoefficient[j] ^ GF256[(GenPolyCoefficient[j - i] + mul) % 255]);
        }
    }
    for (int j = 0; j < 10; j++) {
        errorCodeNum[j] = infoPolyCoefficient[j + 16];
    }

    if (DEBUG) {
        for (int i = 0; i < 10; i ++) {
            printf("%d ",errorCodeNum[i]);
        }
        printf("\n");
    }

    for (int i = 0; i < 10; i++) {
        char *result = GetLenBinString(errorCodeNum[i], 8);
        strcat(errorCodeChar, result);
        free(result);
    }
}

int main(int argc, char **argv) {
    SetConsoleOutputCP(437);
    char *text = argv[1];
    DrawBackground();
    GetDataCode(text);
    PrepareGF256();
    GetErrorCode();
    strcat(dataCode, errorCodeChar);
    if (DEBUG) {
        printf("ErrorCode : %s \n len : %llu \n",dataCode, strlen(errorCodeChar));
        printf("DataCode : %s \n len : %llu \n",dataCode, strlen(dataCode));
    }
    Print();
    return 0;
}
#include "macro.h"
#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>

#define N 1024

static void run(const char* source) {
    initScanner(source);
    int line = -1;
    // ��ӡToken, ����TOKEN_EOFΪֹ
    while (1) {
        Token token = scanToken();
        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        }
        else {
            printf("   | ");
        }
        printf("%2d '%.*s'\n", token.type, token.length, token.start);

        if (token.type == TOKEN_EOF) break;
    }
}

static void repl() {
    // ���û��������û�ÿ����һ�д��룬����һ�д��룬����������
    // repl��"read evaluate print loop"����д
    char line[N];
    while (1) {
        printf("> ");
        if (fgets(line, sizeof(line), stdin) == NULL){
            printf("\n");
            break;
        }
        run(line);
    }
}

static char* readFile(const char* path) {
    // �û������ļ������������ļ������ݶ����ڴ棬����ĩβ���'\0'
    // ע��: ����Ӧ��ʹ�ö�̬�ڴ���䣬���Ӧ������ȷ���ļ��Ĵ�С��
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "fopen failed in readFile for %s.\n", path);
        exit(-1);
    }

    fseek(fp, 0, SEEK_END);
    size_t buffer_size = ftell(fp) + 1;
    rewind(fp);

    char* buffer = MALLOC(buffer_size, char);
    if (buffer == NULL) {
        fprintf(stderr, "malloc failed in readFile.\n");
        exit(-1);
    }
    size_t read_size = fread(buffer, sizeof(char), buffer_size, fp);
    if (read_size < buffer_size - 1) {
        fprintf(stderr, "fread failed in readFile for %s.\n", path);
        exit(-1);
    }
    buffer[buffer_size - 1] = '\0';
    fclose(fp);
    return buffer;
}

static void runFile(const char* path) {
    // ����'.c'�ļ�:�û������ļ��������������ļ�������������
    char* source = readFile(path);
    run(source);
    free(source);
    return;
}

int main(int argc, const char* argv[]) {
    if (argc == 1) {
        // ./scanner û�в���,����뽻��ʽ����
        repl();
    }
    else if (argc == 2) {
        // ./scanner file �����һ������,����������ļ�
        runFile(argv[1]);
    }
    else {
        fprintf(stderr, "Usage: scanner [path]\n");
        exit(1);
    }

    return 0;
}

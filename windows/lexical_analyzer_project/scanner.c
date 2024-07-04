#include "macro.h"
#include "scanner.h"
#include "stdbool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    const char* start;
    const char* current;
    int line;
}Scanner;

/*全局变量*/
Scanner scanner;

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAtEnd() {
    return *scanner.current == '\0';
}

static char advance() {
    return *scanner.current++;
}

static char peek() {
    return *scanner.current;
}

static char peekNext() {
    if (isAtEnd()) return '\0';
    return *(scanner.current + 1);
}

static bool match(char expected) {
    if (isAtEnd()) return false;
    if (peek() != expected) return false;
    scanner.current++;
    return true;
}

// 传入TokenType, 创建对应类型的Token，并返回。
static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

// 遇到不能解析的情况时，我们创建一个ERROR Token. 比如：遇到@，$等符号时，比如字符串，字符没有对应的右引号时。
static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

static void skipWhitespace() {
    // 跳过空白字符: ' ', '\r', '\t', '\n'和注释
    // 注释以'//'开头, 一直到行尾
    // 注意更新scanner.line！
    while (1) {
        switch (peek()) {
        case ' ':
            advance();
            break;
        case '\r':
            advance();
            break;
        case '\t':
            advance();
            break;
        case '\n':
            advance();
            scanner.line++;
            break;
        case '/':
            if (peekNext() == '/') {
                while (!isAtEnd() && peek() != '\n') {
                    advance();
                }
            }
        default:
            break;
        }
    }
    return;
}

static TokenType identifierType() {
    // 确定identifier类型主要有两种方式：
    // 1. 将所有的关键字放入哈希表中，然后查表确认
    // 2. 将所有的关键字放入Trie树中，然后查表确认
    // Trie树的方式不管是空间上还是时间上都优于哈希表的方式
    int len = scanner.current - scanner.start;
    const char* tmp_ptr = scanner.current;
    scanner.current = scanner.start;
    advance();
    switch (*scanner.start){
    case 'b':
        if (len != 4) { break; }
        if (advance() != 'r') { break; }
        if (advance() != 'e') { break; }
        if (advance() != 'a') { break; }
        if (advance() != 'k') { break; }
        return TOKEN_BREAK;
    case 'c':
        switch (len){
        case 4:
            switch (advance()){
            case 'a':
                if (advance() != 's') { break; }
                if (advance() != 'e') { break; }
                return TOKEN_CASE;
            case 'h':
                if (advance() != 'a') { break; }
                if (advance() != 'r') { break; }
                return TOKEN_CHAR;
            default:
                break;
            }
        case 8:
            if (advance() != 'o') { break; }
            if (advance() != 'n') { break; }
            if (advance() != 't') { break; }
            if (advance() != 'i') { break; }
            if (advance() != 'n') { break; }
            if (advance() != 'u') { break; }
            if (advance() != 'e') { break; }
            return TOKEN_CONTINUE;
        case 5:
            if (advance() != 'o') { break; }
            if (advance() != 'n') { break; }
            if (advance() != 's') { break; }
            if (advance() != 't') { break; }
            return TOKEN_CONST;
        default:
            break;
        }
    case 'd':
        switch (len) {
        case 7:
            if (advance() != 'e') { break; }
            if (advance() != 'f') { break; }
            if (advance() != 'a') { break; }
            if (advance() != 'u') { break; }
            if (advance() != 'l') { break; }
            if (advance() != 't') { break; }
            return TOKEN_RETURN;
        case 6:
            if (advance() != 'o') { break; }
            if (advance() != 'u') { break; }
            if (advance() != 'b') { break; }
            if (advance() != 'l') { break; }
            if (advance() != 'e') { break; }
            return TOKEN_DOUBLE;
        default:
            break;
        }
    case 'e':
        if (len != 4) { break; }
        switch (advance()){
        case 'n':
            if (advance() != 'u') { break; }
            if (advance() != 'm') { break; }
            return TOKEN_ENUM;
        case 'l':
            if (advance() != 's') { break; }
            if (advance() != 'e') { break; }
            return TOKEN_ELSE;
        default:
            break;
        }
    case 'f':
        switch (len){
        case 5:
            if (advance() != 'l') { break; }
            if (advance() != 'o') { break; }
            if (advance() != 'a') { break; }
            if (advance() != 't') { break; }
            return TOKEN_FLOAT;
        case 3:
            if (advance() != 'o') { break; }
            if (advance() != 'r') { break; }
            return TOKEN_FOR;
        default:
            break;
        }
    case 'g':
        if (len != 4) { break; }
        if (advance() != 'o') { break; }
        if (advance() != 't') { break; }
        if (advance() != 'o') { break; }
        return TOKEN_GOTO;
    case 'i':
        switch (len){
        case 2:
            if (advance() != 'f') { break; }
            return TOKEN_IF;
        case 3:
            if (advance() != 'i') { break; }
            if (advance() != 't') { break; }
            return TOKEN_IF;
        default:
            break;
        }
    case 'l':
        if (len != 4) { break; }
        if(advance() != 'o') { break; }
        if(advance() != 'n') { break; }
        if(advance() != 'g') { break; }
        return TOKEN_LONG;
    case 'r':
        if (len != 6) { break; }
        if (advance() != 'e') { break; }
        if (advance() != 't') { break; }
        if (advance() != 'u') { break; }
        if (advance() != 'r') { break; }
        if (advance() != 'n') { break; }
        return TOKEN_RETURN;
    case 's':
        switch (len){
        case 5:
            if (advance() != 'h') { break; }
            if (advance() != 'o') { break; }
            if (advance() != 'r') { break; }
            if (advance() != 't') { break; }
            return TOKEN_SHORT;
        case 6:
            switch (advance()){
            case 'i':
                switch (advance()){
                case 'g':
                    if (advance() != 'n') { break; }
                    if (advance() != 'e') { break; }
                    if (advance() != 'd') { break; }
                    return TOKEN_SIGNED;
                case 'z':
                    if (advance() != 'e') { break; }
                    if (advance() != 'o') { break; }
                    if (advance() != 'f') { break; }
                    return TOKEN_SIZEOF;
                default:
                    break;
                }
            case 't':
                if (advance() != 'r') { break; }
                if (advance() != 'u') { break; }
                if (advance() != 'c') { break; }
                if (advance() != 't') { break; }
                return TOKEN_STRUCT;
            case 'w':
                if (advance() != 'i') { break; }
                if (advance() != 't') { break; }
                if (advance() != 'c') { break; }
                if (advance() != 'h') { break; }
                return TOKEN_SWITCH;
            default:
                break;
            }
        default:
            break;
        }
    case 't':
        if (len != 7) { break; }
        if (advance() != 'y') { break; }
        if (advance() != 'p') { break; }
        if (advance() != 'e') { break; }
        if (advance() != 'd') { break; }
        if (advance() != 'e') { break; }
        if (advance() != 'f') { break; }
        return TOKEN_TYPEDEF;
    case 'u':
        switch (len){
        case 8:
            if (advance() != 'n') { break; }
            if (advance() != 's') { break; }
            if (advance() != 'i') { break; }
            if (advance() != 'g') { break; }
            if (advance() != 'n') { break; }
            if (advance() != 'e') { break; }
            if (advance() != 'd') { break; }
            return TOKEN_UNSIGEND;
        case 5:
            if (advance() != 'n') { break; }
            if (advance() != 'i') { break; }
            if (advance() != 'o') { break; }
            if (advance() != 'n') { break; }
            return TOKEN_UNION;
        default:
            break;
        }
    case 'v':
        if (len != 4) { break; }
        if (advance() != 'o') { break; }
        if (advance() != 'i') { break; }
        if (advance() != 'd') { break; }
        return TOKEN_VOID;
    case 'w':
        if (len != 5) { break; }
        if (advance() != 'h') { break; }
        if (advance() != 'i') { break; }
        if (advance() != 'l') { break; }
        if (advance() != 'e') { break; }
        return TOKEN_WHILE;
    default:
        break;
    }
    scanner.current = tmp_ptr;
    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    // IDENTIFIER包含: 字母，数字和下划线
    while (isAlpha(peek()) || isDigit(peek())) {
        advance();
    }
    // 这样的Token可能是标识符, 也可能是关键字, identifierType()是用来确定Token类型的
    return makeToken(identifierType());
}

static Token number() {
    // 简单起见，我们将NUMBER的规则定义如下:
    // 1. NUMBER可以包含数字和最多一个'.'号
    // 2. '.'号前面要有数字
    // 3. '.'号后面也要有数字
    // 这些都是合法的NUMBER: 123, 3.14
    // 这些都是不合法的NUMBER: 123., .14
    int point_count = 0;
    while (isDigit(peek()) || match('.')) {
        if (match('.')) {
            point_count++;
            if (point_count > 1 || !isDigit(peekNext()) || !isDigit(*(scanner.current - 1))) {
                break;
            }
        }
        advance();
    }
    if (isAlpha(peek())) {
        while (isAlpha(peek()) || isDigit(peek())) {
            advance();
        }
        return makeToken(TOKEN_ERROR);
    }
    else {
        return makeToken(TOKEN_NUMBER);
    }
}

static Token string() {
    // 字符串以"开头，以"结尾，而且不能跨行
    advance();
    while (match('\"')||match('\n')) {
        advance();
    }
    if (advance() == '\n') {
        scanner.line++;
        return makeToken(TOKEN_ERROR);
    }
    return errorToken(scanner.start);
}

static Token character() {
    // 字符'开头，以'结尾，而且不能跨行
    advance();
    if (peekNext() == '\'') {
        scanner.current += 2;
        return makeToken(TOKEN_CHARACTER);
    }
    while (match('\n')) {
        advance();
    }
    return errorToken(scanner.start);
}


/*初始化scanner*/
void initScanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
    return;
}

/*调用 scanToken(), 返回下一个 Token */
Token scanToken() {
    if (isAtEnd()) {
        return makeToken(TOKEN_EOF);
    }

    skipWhitespace();
    scanner.start = scanner.current;

    /*标识符或关键字*/
    if (isAlpha(peek())) {
        return identifier();
    }
    /*字符*/
    if (match('\'')) {
        return character();
    }
    /*字符串*/
    if (match('\"')) {
        return string();
    }
    /*数字*/
    if (isDigit(peek())) {
        return number();
    }

    /*single-character tokens*/
    switch (advance()) {
    case '(':
        return makeToken(TOKEN_LEFT_PAREN);
    case ')':
        return makeToken(TOKEN_RIGHT_PAREN);
    case '[':
        return makeToken(TOKEN_LEFT_BRACKET);
    case ']':
        return makeToken(TOKEN_RIGHT_BRACKET);
    case '{':
        return makeToken(TOKEN_LEFT_BRACE);
    case '}':
        return makeToken(TOKEN_RIGHT_BRACE);
    case ',':
        return makeToken(TOKEN_COMMA);
    case '.':
        return makeToken(TOKEN_DOT);
    case ';':
        return makeToken(TOKEN_SEMICOLON);
    case '~':
        return makeToken(TOKEN_TILDE);

    /*one or two character tokens*/
    case '+':
        if (match('+')) {
            return makeToken(TOKEN_PLUS_PLUS);
        }
        else if (match('=')) {
            return makeToken(TOKEN_PLUS_EQUAL);
        }
        else {
            return makeToken(TOKEN_PLUS);
        }
    case '-':
        if (match('-')) {
            return makeToken(TOKEN_MINUS_MINUS);
        }
        else if (match('=')) {
            return makeToken(TOKEN_MINUS_EQUAL);
        }
        else if (match('>')) {
            return makeToken(TOKEN_MINUS_GREATER);
        }
        else {
            return makeToken(TOKEN_MINUS);
        }
    case '*':
        return makeToken(match('=') ? TOKEN_STAR_EQUAL : TOKEN_STAR);
    case '/':
        return makeToken(match('=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH);
    case '%':
        return makeToken(match('=') ? TOKEN_PERCENT_EQUAL : TOKEN_PERCENT);
    case '&':
        if (match('=')) {
            return makeToken(TOKEN_AMPER_EQUAL);
        }
        else if (match('&')) {
            return makeToken(TOKEN_AMPER_AMPER);
        }
        else {
            return makeToken(TOEKN_AMPER);
        }
    case '|':
        if (match('=')) {
            return makeToken(TOKEN_PIPE_EQUAL);
        }
        else if (match('|')) {
            return makeToken(TOKEN_PIPE_PIPE);
        }
        else {
            return makeToken(TOKEN_PIPE);
        }
    case '^':
        return makeToken(match('=') ? TOKEN_HAT_EQUAL : TOKEN_HAT);
    case '=':
        return makeToken(match('=') ? TOKEN_EQUAL_EUQAL : TOKEN_EQUAL);
    case '!':
        return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOEKN_BANG);
    case '<':
        if (match('=')) {
            return makeToken(TOKEN_LESS_EQUAL);
        }
        else if (match('<')) {
            return makeToken(TOKEN_LESS_LESS);
        }
        else {
            return makeToken(TOKEN_LESS);
        }
    case '>':
        if (match('=')) {
            return makeToken(TOKEN_GREATER_EQUAL);
        }
        else if (match('>')) {
            return makeToken(TOKEN_GREATER_GREATER);
        }
        else {
            return makeToken(TOKEN_GREATER);
        }
    default:
        return makeToken(TOKEN_ERROR);
        break;
    }
}

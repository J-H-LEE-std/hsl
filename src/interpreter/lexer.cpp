#include "token.h"
#include "lexer.h"

#include <utility>
#include <iostream>

namespace hsl {
    Lexer::Lexer(std::string input)
            : input(std::move(input)), pos(0), readPos(0), ch('\0'), line(1), column(0) {
        readChar(); // 첫 글자 로드
    }

    Token Lexer::nextToken() {
        skipWhitespace();

        Token tok;
        tok.line = line;
        tok.column = column;

        switch (ch) {
            // 연산자 및 기호
            case '+':
                tok = Token{TokenType::PLUS, "+", line, column};
                break;
            case '-':
                tok = Token{TokenType::MINUS, "-", line, column};
                break;
            case '*':
                tok = Token{TokenType::ASTERISK, "*", line, column};
                break;
            case '^':
                tok = Token{TokenType::CARET, "^", line, column};
                break;
            case '/':
                tok = Token{TokenType::SLASH, "/", line, column};
                break;
            case '(':
                tok = Token{TokenType::LPAREN, "(", line, column};
                break;
            case ')':
                tok = Token{TokenType::RPAREN, ")", line, column};
                break;
            case ',':
                tok = Token{TokenType::COMMA, ",", line, column};
                break;

                //비교 연산자
            case '<':
                if (peekChar() == '=') {
                    readChar();
                    tok = Token{TokenType::LEQ, "<=", line, column};
                } else {
                    tok = Token{TokenType::LT, "<", line, column};
                }
                break;

            case '>':
                if (peekChar() == '=') {
                    readChar();
                    tok = Token{TokenType::GEQ, ">=", line, column};
                } else {
                    tok = Token{TokenType::GT, ">", line, column};
                }
                break;

            case '=':
                if (peekChar() == '=') {
                    // ==도 EQ로 인식
                    readChar();
                    tok = Token{TokenType::EQ, "==", line, column};
                } else {
                    tok = Token{TokenType::EQ, "=", line, column};
                }
                break;

            case '!':
                if (peekChar() == '=') {
                    readChar();
                    tok = Token{TokenType::NEQ, "!=", line, column};
                } else {
                    tok = Token{TokenType::ILLEGAL, "!", line, column};
                }
                break;

            case '[': {
                // [OBJ], [VAR], [ST], [END]는 그 자체로 예약어 토큰. 이걸 읽기 위해 별도의 조치 마련.
                size_t savePos = readPos;
                size_t tmp = savePos;
                std::string lookahead;

                size_t i = tmp;
                while (i < input.size() && input[i] != ']' && !std::isspace((unsigned char)input[i])) {
                    lookahead.push_back(input[i]);
                    i++;
                }

                if (lookahead == "OBJ" || lookahead == "VAR" ||
                    lookahead == "ST"  || lookahead == "END") {
                    tok = readSectionKeyword(); // 예약어는 별도로 분기.
                } else {
                    tok = Token{TokenType::LBRACKET, "[", line, column};
                }

                break;
            }

            case ']':
                tok = Token{TokenType::RBRACKET, "]", line, column};
                break;

            case '.':
                if (peekChar() == '.') {
                    readChar();
                    tok = Token{TokenType::RANGE, "..", line, column};
                } else {
                    tok = Token{TokenType::ILLEGAL, ".", line, column};
                }
                break;

            case '#':
                for (;;) {
                    skipWhitespace();
                    if (ch == '#') {
                        skipComment();
                        continue;
                    } // 재귀 대신 continue
                    break;
                }

            case '\0':
                tok = Token{TokenType::END_OF_FILE, "", line, column};
                break;

            default:
                if (std::isalpha(ch) || ch == '_') {
                    return readIdentifier();
                } else if (std::isdigit(ch)) {
                    return readNumber();
                } else {
                    tok = Token{TokenType::ILLEGAL, std::string(1, ch), line, column};
                }
        }

        readChar();
        return tok;
    }

    Token Lexer::readSectionKeyword() {
        const int startCol = column;
        readChar(); // '[' 다음 문자로 이동

        std::string buf;
        while (ch != ']' && ch != '\0') {
            buf.push_back(ch);
            readChar();
        }

        if (ch == ']') {
            // ']'까지 읽었으면 섹션 토큰 완성
            if (buf == "OBJ") return Token{TokenType::OBJ, buf, line, startCol};
            if (buf == "VAR") return Token{TokenType::VAR, buf, line, startCol};
            if (buf == "ST") return Token{TokenType::ST, buf, line, startCol};
            if (buf == "END") return Token{TokenType::END, buf, line, startCol};
        }

        // 매칭 실패되면 오류
        return Token{TokenType::ILLEGAL, "[" + buf, line, startCol};
    }

    void Lexer::skipWhitespace() {
        while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            readChar();
        }
    }

    Token Lexer::readIdentifier() {
        const int startCol = column; size_t startPos = pos; // 첫 글자의 조건은 [A-Za-z_]
        if (!(std::isalpha(ch) || ch == '_')) {
            return Token{TokenType::ILLEGAL, std::string(1, ch), line, startCol};
        } readChar();

        // 이후로는 [A-Za-z0-9_]
        while (std::isalnum(ch) || ch == '_') {
            readChar();
        }
        std::string literal = input.substr(startPos, pos - startPos);
        const TokenType type = lookupIdent(literal);
        return Token{type, literal, line, startCol};
    }

    TokenType Lexer::lookupIdent(const std::string &ident) {
        if (ident == "max") return TokenType::MAX;
        if (ident == "min") return TokenType::MIN;
        if (ident == "int") return TokenType::INT;
        if (ident == "any") return TokenType::ANY;
        return TokenType::IDENT;
    }

    Token Lexer::readNumber() {
        const int startLine = line;
        const int startCol = column;
        size_t startPos = pos;

        bool isFloat = false;

        // 정수부
        while (std::isdigit(static_cast<unsigned char>(ch))) {
            readChar();
        }

        // 소수부
        if (ch == '.') {
            char p = peekChar();
            // 소수의 경우
            if (std::isdigit(static_cast<unsigned char>(p))) {
                isFloat = true;
                readChar(); // '.' 소비
                while (std::isdigit(static_cast<unsigned char>(ch))) readChar();
            } else if (p == '.') {
                ; // ..는 range로 처리해야하므로 따로 여기서 처리 안 하고 TokenType::RANGE 처리할 때 같이 처리.
            } else {
                ; // '1.' 같은 유형은 ILLEGAL로 처리 예정, 여기서 하지 않음.
            }
        }

        // 지수부 -> 해당 프로그램은 1e02 등의 형식도 지원함.
        if (ch == 'e' || ch == 'E') {
            isFloat = true;
            readChar(); // consume 'e' or 'E'
            if (ch == '+' || ch == '-') {
                readChar();
            }
            if (!std::isdigit(static_cast<unsigned char>(ch))) {
                return Token{TokenType::ILLEGAL, "bad exponent", startLine, startCol};
            }
            while (std::isdigit(static_cast<unsigned char>(ch))) {
                readChar();
            }
        }

        std::string literal = input.substr(startPos, pos - startPos);
        const TokenType type = isFloat ? TokenType::NUMBER_FLOAT : TokenType::NUMBER_INT;

        return Token{type, literal, startLine, startCol};
    }

    void Lexer::skipComment() {
        while (ch != '\n' && ch != '\0') { //공백은 다음 nextToken()이 처리.
            readChar();
        }
    }

    void Lexer::readChar() {
        if (readPos >= input.size()) {
            ch = '\0';             // EOF sentinel
            pos = readPos;
        } else {
            ch = input[readPos];
            pos = readPos;
        }
        readPos++;

        if (ch == '\n') {
            line++;
            column = 0;            // 열은 새 줄에서 0으로 리셋 (0-based 유지)
        } else {
            column++;              // 현재 ch의 칼럼
        }
    }

    char Lexer::peekChar() const {
        return (readPos < input.size()) ? input[readPos] : '\0';
    }

}
#ifndef HSL_TOKEN_
#define HSL_TOKEN_

#include <string>

namespace hsl {
    enum class TokenType {
        // 키워드
        OBJ, VAR, ST, END,
        MAX, MIN,
        INT, ANY,

        // 리터럴
        IDENT,      // 변수명, 함수명
        NUMBER_INT, NUMBER_FLOAT,     // 정수/실수(double)

        // 연산자
        PLUS, MINUS, ASTERISK, SLASH,
        LPAREN, RPAREN, COMMA,
        LEQ, GEQ, LT, GT, EQ, NEQ, CARET,

        // 기타
        END_OF_FILE, ILLEGAL
    };

    struct Token {
        TokenType type;
        std::string literal{}; // 추후 필요 시에 확장
        int line = 0;
        int column = 0;
    };

}

#endif
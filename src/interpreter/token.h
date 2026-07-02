/**
 * @file token.h
 * @brief Header file defining token for interpret HS-L source code.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#ifndef HSL_TOKEN_
#define HSL_TOKEN_

#include <string>

namespace hsl {
    enum class TokenType {
        // keyword
        OBJ, VAR, DEFCONST, DEFFUNC, ST, END,
        MAX, MIN,
        INT, ANY,

        // literal
        IDENT, // Identical name for function or variable.
        NUMBER_INT, NUMBER_FLOAT, // Integer or float number.

        // operators
        PLUS, MINUS, ASTERISK, SLASH,
        LPAREN, RPAREN, LBRACKET, RBRACKET, COMMA,
        LEQ, GEQ, LT, GT, EQ, NEQ, CARET, RANGE,

        // special tokens.
        END_OF_FILE, ILLEGAL,
    };

    struct Token {
        TokenType type;
        std::string literal{};
        int line = 0;
        int column = 0;
    };
}

#endif

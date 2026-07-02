/**
 * @file lexer.h
 * @brief Header file for lexer class defination.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#ifndef HSL_LEXER_
#define HSL_LEXER_

#include <string>
#include "token.h"

namespace hsl{
    class Lexer {
    public:
        explicit Lexer(std::string input);

        Token nextToken(); // Return next token.

    private:
        std::string input;
        size_t pos; // Current position.
        size_t readPos; // Next position.
        char ch; // Current reading character.
        int line;
        int column;

        void readChar();
        [[nodiscard]] char peekChar() const;
        void skipWhitespace();
        void skipComment();
        Token readIdentifier();
        Token readNumber();
        Token readSectionKeyword(); // Method for reading section keyword tokens.
        static TokenType lookupIdent(const std::string &ident);
        void skipIrrelevant();
    };
}

#endif

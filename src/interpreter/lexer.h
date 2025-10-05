#ifndef HSL_LEXER_
#define HSL_LEXER_

#include <string>
#include "token.h"

namespace hsl{
    class Lexer {
    public:
        explicit Lexer(std::string input);

        // 토큰 하나 반환
        Token nextToken();

    private:
        std::string input;
        size_t pos;      // 현재 읽고 있는 위치
        size_t readPos;  // 다음 읽을 위치
        char ch;         // 현재 문자
        int line;
        int column;

        void readChar();
        [[nodiscard]] char peekChar() const;
        void skipWhitespace();
        void skipComment();
        Token readIdentifier();
        Token readNumber();
        Token readSectionKeyword(); // [OBJ], [VAR], [ST], [END] 처리용
        static TokenType lookupIdent(const std::string &ident);
    };

}

#endif
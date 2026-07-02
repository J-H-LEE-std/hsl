/**
 * @file lexer.h
 * @brief HS-L Lexer implementation.
 * Lexer implemented as interpreter reading all character and return valid token.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#include "token.h"
#include "lexer.h"

#include <utility>
#include <iostream>

namespace hsl {
    /**
    * @brief Initializes the Lexer with the given input string.
    * @param input The source code string to be tokenized.
    * @details Sets up initial positions and loads the first character into the buffer.
    */
    Lexer::Lexer(std::string input)
            : input(std::move(input)), pos(0), readPos(0), ch('\0'), line(1), column(0) {
        readChar(); // load first character
    }

    /**
    * @brief Scans and returns the next token from the input.
    * @return The next identified Token object (e.g., operator, identifier, or keyword).
    * @details Skips whitespace and comments automatically before identifying symbols,
    * comparators, and section keywords like [OBJ] or [CONST].
    */
    Token Lexer::nextToken() {
        skipIrrelevant();
        skipWhitespace();

        Token tok;
        tok.line = line;
        tok.column = column;

        switch (ch) {
            // operators and symbols.
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

            // Comparators.
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
                    // operators == and = are considered equivalence relations
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
                // [OBJ], [VAR], [CONST], [FUNC], [ST], [END] are section keywords.
                size_t savePos = readPos;
                size_t tmp = savePos;
                std::string lookahead;

                size_t i = tmp;
                while (i < input.size() && input[i] != ']' && !std::isspace((unsigned char)input[i])) {
                    lookahead.push_back(input[i]);
                    i++;
                }

                if (lookahead == "OBJ" || lookahead == "VAR" ||
                    lookahead == "CONST" || lookahead == "FUNC" ||
                    lookahead == "ST"  || lookahead == "END") {
                    tok = readSectionKeyword(); // If keyword detected, branch for keyword method.
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
                    } // Used continue for comment instead of recursive.
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

    /**
    * @brief Specialized handler for section keywords enclosed in brackets.
    * @return A Token corresponding to section keyword if a match is found;
    * otherwise, returns an ILLEGAL token.
    */
    Token Lexer::readSectionKeyword() {
        const int startCol = column;
        readChar(); // Move to next character of '['.

        std::string buf;
        while (ch != ']' && ch != '\0') {
            buf.push_back(ch);
            readChar();
        }

        if (ch == ']') {
            // Section token generated if read ']' successfully.
            if (buf == "OBJ") return Token{TokenType::OBJ, buf, line, startCol};
            if (buf == "VAR") return Token{TokenType::VAR, buf, line, startCol};
            if (buf == "CONST") return Token{TokenType::DEFCONST, buf, line, startCol};
            if (buf == "FUNC") return Token{TokenType::DEFFUNC, buf, line, startCol};
            if (buf == "ST") return Token{TokenType::ST, buf, line, startCol};
            if (buf == "END") return Token{TokenType::END, buf, line, startCol};
        }

        // Throw error token if does not match.
        return Token{TokenType::ILLEGAL, "[" + buf, line, startCol};
    }

    /**
    * @brief Consumes whitespace characters (spaces, tabs, newlines) from the input.
    */
    void Lexer::skipWhitespace() {
        while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            readChar();
        }
    }

    /**
    * @brief Reads a literal identifier or keyword from the current position.
    * @return A Token of type IDENT, MAX, MIN, INT, or ANY.
    */
    Token Lexer::readIdentifier() {
        const int startCol = column;
        size_t startPos = pos;
        // First letter of literal must be [A-Za-z_]
        if (!(std::isalpha(ch) || ch == '_')) {
            return Token{TokenType::ILLEGAL, std::string(1, ch), line, startCol};
        } readChar();

        // [A-Za-z0-9_] for another chracter in ilteral.
        while (std::isalnum(ch) || ch == '_') {
            readChar();
        }
        std::string literal = input.substr(startPos, pos - startPos);
        const TokenType type = lookupIdent(literal);
        return Token{type, literal, line, startCol};
    }

    /**
    * @brief Determines if a string literal is a reserved keyword or a standard identifier.
    * @param ident The string to check.
    * @return The corresponding TokenType.
    */
    TokenType Lexer::lookupIdent(const std::string &ident) {
        if (ident == "max") return TokenType::MAX;
        if (ident == "min") return TokenType::MIN;
        if (ident == "int") return TokenType::INT;
        if (ident == "any") return TokenType::ANY;
        return TokenType::IDENT;
    }

    /**
    * @brief Reads a numeric literal, supporting integers, floating points, and scientific notation.
    * @return A Token of type NUMBER_INT or NUMBER_FLOAT.
    */
    Token Lexer::readNumber() {
        const int startLine = line;
        const int startCol = column;
        size_t startPos = pos;

        bool isFloat = false;

        // Integer part.
        while (std::isdigit(static_cast<unsigned char>(ch))) {
            readChar();
        }

        // Fractional part.
        if (ch == '.') {
            char p = peekChar();
            // If floating point number is needed:
            if (std::isdigit(static_cast<unsigned char>(p))) {
                isFloat = true;
                readChar(); // Consume '.'.
                while (std::isdigit(static_cast<unsigned char>(ch))) readChar();
            } else if (p == '.') {
                ;
                /* Literal ".." is used in range-based variable defination.
                 They will be rocessed it together when handling TokenType::RANGE. */
            } else {
                ; // Not supported literal type "1.", so they are ILLEGAL.
            }
        }

        // HS-L support exponetional notation(ex: 1e-1).
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

    /**
    * @brief Skips characters until the end of the current line (triggered by '#').
    */
    void Lexer::skipComment() {
        while (ch != '\n' && ch != '\0') { // Whitespase are treated by nextToken().
            readChar();
        }
    }

    /**
    * @brief Reads the next character from the input and updates position/line/column trackers.
    */
    void Lexer::readChar() {
        if (readPos >= input.size()) {
            ch = '\0'; // EOF sentinel
            pos = readPos;
        } else {
            ch = input[readPos];
            pos = readPos;
        }
        readPos++;

        if (ch == '\n') {
            line++;
            column = 0; // Column's position reset when enter new line (keep 0-based).
        } else {
            column++; // Renew colun position for current character.
        }
    }

    /**
    * @brief Looks ahead to the next character without consuming it.
    * @return The character at readPos, or '\0' if at the end of input.
    */
    char Lexer::peekChar() const {
        return (readPos < input.size()) ? input[readPos] : '\0';
    }

    /**
    * @brief A helper utility that continuously skips both whitespace and comments.
    */
    void Lexer::skipIrrelevant() {
        for (;;) {
            // Skip whitespase.
            while (isspace(ch)) readChar();

            // Skip comments.
            if (ch == '#') {
                skipComment();
                continue;
            }

            break;
        }
    }
}

/**
 * @file printer.h
 * @brief Header file for utility for printing tokens and AST structures for debugging purposes.
 * @deprecated This utility is considered abandoned and is no longer used in the current version of HS-L.
 * @note This was originally designed for initial testing and verification during early development.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#ifndef HSL_PRINTER_
#define HSL_PRINTER_

#include "../interpreter/ast.h"
#include <iostream>
#include <string>

namespace hsl {
    void printToken(std::string& str);
    void printAST(const Program* prg, int indent = 0);
    void printExpr(const Expression* expr, int indent = 0);
}

#endif

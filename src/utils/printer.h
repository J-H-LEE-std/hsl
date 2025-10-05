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

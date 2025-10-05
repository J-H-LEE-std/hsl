#include "printer.h"

#include <utility>
#include "../interpreter/lexer.h"

namespace hsl{
    void printToken(std::string& str){
        hsl::Lexer lex(str);
        hsl::Token tok;
        do {
            tok = lex.nextToken();
            std::cout << tok.line << ":" << tok.column << "  "
                      << static_cast<int>(tok.type) << "  "
                      << tok.literal << "\n";
        } while (tok.type != hsl::TokenType::END_OF_FILE);
    }

    static void indentPrint(int indent, const std::string& text) {
        for (int i = 0; i < indent; i++) std::cout << "  ";
        std::cout << text << "\n";
    }

    void printExpr(const Expression* expr, int indent) {
        if (!expr) { indentPrint(indent, "(null)"); return; }

        if (auto n = dynamic_cast<const NumberExpr*>(expr)) {
            indentPrint(indent, "Number(" + std::to_string(n->value) + ")");
        } else if (auto id = dynamic_cast<const IdentExpr*>(expr)) {
            indentPrint(indent, "Ident(" + id->name + ")");
        } else if (auto u = dynamic_cast<const UnaryExpr*>(expr)) {
            indentPrint(indent, "Unary(" + std::to_string((int)u->op) + ")");
            printExpr(u->expr, indent+1);
        } else if (auto b = dynamic_cast<const BinaryExpr*>(expr)) {
            indentPrint(indent, "Binary(" + std::to_string((int)b->op) + ")");
            printExpr(b->left, indent+1);
            printExpr(b->right, indent+1);
        } else {
            indentPrint(indent, "UnknownExpr");
        }
    }

    void printAST(const Program* prg, int indent) {
        if (!prg) { indentPrint(indent, "(null program)"); return; }

        indentPrint(indent, "Program:");

        // Objective
        if (prg->obj) {
            indentPrint(indent+1, prg->obj->isMax ? "Objective: max" : "Objective: min");
            printExpr(prg->obj->expr, indent+2);
        }

        // Vars
        for (auto* v : prg->vars) {
            indentPrint(indent + 1, "VarDecl " + v->name + " "
                                    + (v->isInt ? "int" : "any"));
            indentPrint(indent + 2, "Lower bound expr:");
            printExpr(v->lower, indent + 3);
            indentPrint(indent + 2, "Upper bound expr:");
            printExpr(v->upper, indent + 3);
        }

        // Constraints
        for (auto* c : prg->constraints) {
            indentPrint(indent+1, "Constraint (" + std::to_string((int)c->comparator) + ")");
            printExpr(c->left, indent+2);
            printExpr(c->right, indent+2);
        }
    }
}
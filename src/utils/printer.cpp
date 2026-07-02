/**
 * @file printer.cpp
 * @brief Utility for printing tokens and AST structures for debugging purposes.
 * @deprecated This utility is considered abandoned and is no longer used in the current version of HS-L.
 * @note This was originally designed for initial testing and verification during early development.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#include "printer.h"

#include <utility>
#include "../interpreter/lexer.h"

namespace hsl{
    /**
     * @brief Tokenizes an input string and prints each token to the standard output.
     * @param str The source string to be tokenized.
     * @deprecated Used for early-stage lexer debugging.
     */
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

    /**
     * @brief Prints a text string with a specified indentation level.
     * @param indent The number of double-spaces to indent.
     * @param text The string to print.
     */
    static void indentPrint(int indent, const std::string& text) {
        for (int i = 0; i < indent; i++) std::cout << "  ";
        std::cout << text << "\n";
    }

    /**
     * @brief Recursively traverses and prints an Expression tree to the console.
     * @param expr The expression node to print.
     * @param indent The current indentation level for tree visualization.
     * @deprecated Replaced by more robust debugging or GUI-based inspection tools.
     */
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

    /**
     * @brief Prints the entire Abstract Syntax Tree (AST) of a Program.
     * @param prg The Program structure containing objectives, variables, and constraints.
     * @param indent The starting indentation level.
     * @details Visualizes the hierarchy of the parsed DSL, including the objective function type,
     * variable declarations with bounds, and all defined constraints.
     * @deprecated This function is abandoned and may not support recent AST node additions.
     */
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

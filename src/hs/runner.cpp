#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include "../interpreter/lexer.h"
#include "../interpreter/parser.h"
#include "params.h"
#include "hsalgorithm.h"
#include "runner.h"

namespace hsl {

    static std::string readAll(const std::string& path) {
        std::ifstream in(path);
        if (!in) throw std::runtime_error("Cannot open HS source file: " + path);
        std::ostringstream ss; ss << in.rdbuf();
        return ss.str();
    }

    Harmony runHarmonySearch(const HSProblem& prob, const HSParams& params) {
        HarmonySearch hs(prob, params);
        return hs.optimize();
    }

    Harmony runHarmonySearch(Program* program, const HSParams& params) {
        HSProblem prob = buildHSProblem(program);
        return runHarmonySearch(prob, params);
    }

    Harmony runHarmonySearchFromFile(const std::string& hsFilePath,
                                     const HSParams& params,
                                     std::vector<std::string>* parseErrors) {
        std::string src = readAll(hsFilePath);

        hsl::Lexer lex(src);
        hsl::Parser parser(lex);

        Program* program = parser.parseProgram();

        // 에러 전달을 원하면 복사해주기
        if (parseErrors) {
            *parseErrors = parser.getErrors();
        }

        // 파싱 에러가 있으면 예외로 처리 (원하면 경고만 찍고 계속 진행하도록 바꿔도 됨)
        const auto& errs = parser.getErrors();
        if (!errs.empty()) {
            std::ostringstream msg;
            msg << "Parse failed with " << errs.size() << " error(s):\n";
            for (const auto& e : errs) msg << "  - " << e << "\n";
            throw std::runtime_error(msg.str());
        }

        return runHarmonySearch(program, params);
    }

}

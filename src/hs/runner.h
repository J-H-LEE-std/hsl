#ifndef HSL_RUNNER_
#define HSL_RUNNER_

#include <string>
#include <vector>
#include "../interpreter/ast.h"
#include "../interpreter/evaluator.h"
#include "hsalgorithm.h"
#include "params.h"

namespace hsl {

    // 1) 이미 AST(Program*)가 있는 경우: evaluator → HS 실행
    Harmony runHarmonySearch(Program* program, const HSParams& params, unsigned int seed);

    // 2) 이미 HSProblem이 있는 경우: 바로 HS 실행
    Harmony runHarmonySearch(const HSProblem& prob, const HSParams& params, unsigned int seed);

    // 3) .hs 파일 경로만 주면: 파일 로드 → Lexer → Parser → evaluator → HS 실행
    //    parseErrors 포인터를 넘기면 파싱 에러 메시지를 채워준다.
    Harmony runHarmonySearchFromFile(const std::string& hsFilePath,
                                     const HSParams& params,
                                     unsigned int seed = std::random_device{}(),
                                     std::vector<std::string>* parseErrors = nullptr);
}

#endif

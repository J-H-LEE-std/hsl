#include "bridge.h"
#include <fstream>
#include <ctime>
#include <memory>
#include "../hs/io.h"  // hsl::cout 연결용

namespace hslgui {

// GUI 로그 스트림 정의
std::ostringstream Bridge::cout;

// 파일 + GUI 동시 출력용 streambuf
struct TeeBuf : public std::streambuf {
    std::streambuf *a, *b;
    TeeBuf(std::streambuf *x, std::streambuf *y) : a(x), b(y) {}
    int overflow(int c) override {
        if (c == EOF) return !EOF;
        if (a) a->sputc(c);
        if (b) b->sputc(c);
        return c;
    }
    int sync() override {
        if (a) a->pubsync();
        if (b) b->pubsync();
        return 0;
    }
};

// 내부 상태
static std::ofstream logFile;
static std::unique_ptr<TeeBuf> teeBuf;
static std::unique_ptr<std::ostream> dual;

// 로그 초기화: 파일 + Bridge::cout tee 설정, hsl::cout 연결
static void InitLog() {
    if (!logFile.is_open()) {
        char buf[32];
        std::time_t t = std::time(nullptr);
        std::tm tm{};
    #ifdef _WIN32
        localtime_s(&tm, &t);
    #else
        localtime_r(&t, &tm);
    #endif
        std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
        std::string name = std::string("hsl_log_") + buf + ".txt";
        logFile.open(name, std::ios::out | std::ios::trunc);
    }

    // GUI 로그창(Bridge::cout)과 파일 동시 출력
    teeBuf = std::make_unique<TeeBuf>(logFile.rdbuf(), Bridge::cout.rdbuf());
    dual = std::make_unique<std::ostream>(teeBuf.get());

    // HS-L 공용 출력(hsl::cout)을 GUI 로그창으로 연결
    hsl::cout.rdbuf(dual->rdbuf());
}

// 내부 실행 로직: 파싱 후 HarmonySearch 실행
static hslgui::ResultStruct RunParsed(const std::string& source, const hslgui::ParamStruct& param, unsigned int seed) {
    ResultStruct res;

    // Parse
    hsl::Lexer lex(source);
    hsl::Parser parser(lex);
    auto program = parser.parseProgram();
    const auto& errs = parser.getErrors();

    if (!errs.empty()) {
        std::ostringstream em;
        em << "Parse failed with " << errs.size() << " error(s):\n";
        for (auto& e : errs) em << "  - " << e << "\n";
        res.error_msg = em.str();
        return res;
    }

    // Evaluator 로직, Problem 생성
    auto problem = hsl::buildHSProblem(program);
    hsl::HSParams p;
    p.HMS = param.HMS;
    p.HMCR = param.HMCR;
    p.PAR = param.PAR;
    p.MaxImp = param.MaxImp;
    p.N_Seg = param.N_Seg;

    // 실행
    auto out = hsl::runHarmonySearch(problem, p, seed, *dual);

    // 결과 변환
    res.best_value = out.value;
    res.cpu_time = out.cpu_time;
    res.variables.reserve(problem.variables.size());
    for (size_t i = 0; i < problem.variables.size() && i < out.vars.size(); ++i)
        res.variables.emplace_back(problem.variables[i].name, out.vars[i]);

    return res;
}

// 외부 API: GUI에서 호출
ResultStruct Bridge::Run(const std::string& source, const ParamStruct& param, unsigned int seed) {
    hsl::cout << "RunParsed received seed = " << seed << std::endl;
    InitLog();

    hsl::cout << "[HS-L] Start Running..." << std::endl;
    hsl::cout << "Seed was set at " << seed << "." << std::endl;
    auto res = RunParsed(source, param, seed);
    if (!res.error_msg.empty())
        hsl::cout << "[HS-L Error] " << res.error_msg << std::endl;
    hsl::cout << "[HS-L] Optimization Finished." << std::endl;

    dual->flush();
    logFile.flush();
    return res;
}

} // namespace hslgui

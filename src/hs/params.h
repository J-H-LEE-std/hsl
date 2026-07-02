/**
 * @file params.h
 * @brief Header defination for parameter utilized in HS engine.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#ifndef HSL_PARAMS_
#define HSL_PARAMS_
#include <string>

namespace hsl{
    // Define parameters in HS.
    struct HSParams {
        int HMS = 30;
        double HMCR = 0.95;
        double PAR = 0.7;
        unsigned int MaxImp = 30000;
        int N_Seg = 300;
    };

    HSParams loadParams(const std::string& filename);
    void editParams(HSParams& param, int HMS, double HMCR, double PAR, unsigned int maxiter);
}
#endif

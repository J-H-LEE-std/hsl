/**
 * @file paramstruct.h
 * @brief Header of parameter struct for the HS-L GUI application.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-11-28
 */

#ifndef HSL_GUI_PARAMSTRUCT_H
#define HSL_GUI_PARAMSTRUCT_H

#include <string>
#include <sstream>

namespace hslgui {
    struct ParamStruct {
        int HMS = 30; double HMCR = 0.95; double PAR = 0.7; unsigned int MaxImp = 30000; int N_Seg = 300;

        std::string ToCSV() const {
            std::ostringstream oss;
            oss << "HMS,"<<HMS<<"\n"<<"HMCR,"<<HMCR<<"\n"<<"PAR,"<<PAR<<"\n"<<"MaxImp,"<<MaxImp<<"\n"<<"N_Seg,"<<N_Seg<<"\n";
            return oss.str();
        }
        // Since the hsparm file is saved in CSV format, use the existing CSV reading logic as is.
        void FromCSV(const std::string& data) {
            std::istringstream ss(data); std::string line;
            while (std::getline(ss, line)) {
                auto p=line.find(','); if(p==std::string::npos) continue;
                auto k=line.substr(0,p), v=line.substr(p+1);
                try{
                    if(k=="HMS") HMS=std::stoi(v);
                    else if(k=="HMCR") HMCR=std::stod(v);
                    else if(k=="PAR") PAR=std::stod(v);
                    else if(k=="MaxImp") MaxImp=(unsigned int)std::stoul(v);
                    else if(k=="N_Seg") N_Seg=std::stoi(v);
                }catch(...){}
            }
        }
    };
}

#endif

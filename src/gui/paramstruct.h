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
    // hsparm 파일은 csv 형태의 파일로 저장하기에 CSV 읽는 로직을 그대로 사용
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

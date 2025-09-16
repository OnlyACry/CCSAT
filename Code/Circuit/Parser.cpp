#include"Circuit/Parser.h"

namespace WorkSpace{
namespace CircuitSAT{
bool Parser::parse(std::istream &is,AigerGraph & graph){
    Str startTitle=deliA+">> BEGIN PARSING";
    cout<<startTitle<<endl;

    Int line_ctr=0;
    char delimiter=' ';
    StringUtil stringUtil;
    GateId PIs=0,POs=0,gateNum=0,andGateNum=0;
    for(Str line;std::getline(is,line,'\n');){
        if(!andGateNum&&line.find("aag")!=Str::npos){
            Vec<Str> arr;
            stringUtil.string_split_by_delimiter(line,arr,delimiter);
            andGateNum=std::stoi(arr[5]);
            if(andGateNum==0){Print("[ERROR]: NO AND GATE!!!",COLORS::red); return false;}
            gateNum=std::stoul(arr[1]);
            PIs=std::stoul(arr[2]);
            POs=std::stoul(arr[4]);
            graph.SpaceInit(gateNum,PIs,POs);
            continue;
        }
        else if(!andGateNum) continue;
        line_ctr++;  
        
        if(line_ctr>0&&line_ctr<=PIs){
            graph.addPI(std::stoul(line));
        }
        else if(line_ctr>PIs&&line_ctr<=PIs+POs){
            graph.addPO(std::stoul(line));
        }
        else if(line_ctr<=PIs+POs+andGateNum){
            Vec<Str> arr;
            Vec<GateId> signals;
            stringUtil.string_split_by_delimiter(line,arr,delimiter);
            for(auto str:arr) {if(str=="") continue;else signals.push_back(std::stoul(str));}
            graph.addGateInit(signals);
        }
        
    }
    Str endTitle=deliA+">> END PARSING";
    cout<<endTitle<<endl<<endl;
    return true;
}

} // namespace CircuitSAT
} // namespace WorkSpace



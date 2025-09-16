#include"Solver/Solver.h"
#include"Common/Timer.h"
#include<stdlib.h>
#include<set>
#include<iterator> 
#include<filesystem>

namespace WorkSpace{
namespace CircuitSAT{
void Solver::run(Config &config){
    if(parameter.isDraw){
        draw(parameter.FileName);
        return ;
    }
    Status status=cdcl();
    timer.getElapsedTime();
    ofstream ofs(parameter.OutPutFile,ios::out);
     if (!ofs.is_open()) {
        std::cerr << "Error: Unable to open output file: " << parameter.OutPutFile << std::endl;
        // Try to recreate the file
        ofs.open(parameter.OutPutFile, std::ios::out);
        if (!ofs.is_open()) {
            std::cerr << "Error: Unable to create output file at: " << parameter.OutPutFile << std::endl;
            return;
        }
        std::cout << "Created new output file at: " << parameter.OutPutFile << std::endl;
    }
    // cout<<parameter.OutPutFile<<endl;
    if(status==Status::UNSAT) {
        ofs<<"ANS=UNSAT"<<endl;
        Print("ANS: UNSAT",COLORS::black,MODELS::HighLight);}
    else if(status==Status::SAT){
        ofs<<"ANS=SAT"<<endl;
        Print("ANS: SAT",COLORS::black,MODELS::HighLight);}
    else if(status==Status::TIMEOUT){
        ofs<<"ANS=TimeOut"<<endl;
        Print("ANS: TimeOUT",COLORS::black,MODELS::HighLight);}
    else if(status==Status::UNKWON){
        ofs<<"ANS=UNKWON"<<endl;
        Print("ANS: UNKWON",COLORS::black,MODELS::HighLight);}
    else{
        ofs<<"ANS=Error"<<endl;
        Print("ANS: Error",COLORS::black,MODELS::HighLight);
    }
    ofs<<"GateNum="<<gatesNums<<endl;
    ofs<<"Conflic Num="<<conflicNums<<endl;
    ofs<<"All Iter="<<allIter<<endl;
    ofs << "RunTime=" << std::to_string(timer.getRunTime()) << " s" << std::endl;
    ofs << "Learnt Gate Count=" << to_string(gateInfos.getGateInfoSize() - gatesNums) << std::endl;
    config.writeConfigToFile(ofs);                      // write config parameters in output file
    
    Print("RunTime: " + std::to_string(timer.getRunTime()) + " s", COLORS::black, MODELS::Normal);
    Print("All Iter: "+to_string(allIter),COLORS::black,MODELS::HighLight);
    Print("Conflic Num: " + std::to_string(conflicNums) + " s", COLORS::black, MODELS::Normal);
    Print("Learning Gate: "+to_string(gateInfos.getGateInfoSize()-gatesNums),COLORS::black,MODELS::HighLight);
    check(ofs);
}
Solver::Solver(AigerGraph &_graph,Config &config):graph(_graph), timer(config){
    Print(deliB+"BEGIN INIT"+deliB,COLORS::black,MODELS::HighLight);
    Print(deliA+">>"+"SPACE INIT",COLORS::blue);
    parameter.init(config);
    piNum=graph.getPIsNum();
    poNum=graph.getPOsNum();
    gatesNums=_graph.getGatesNum();
    gatesNumLimit=parameter.LearnGateNum+gatesNums;
    //------------------------------------------------------------------
    gateInfos.init(gatesNums,piNum,gatesNumLimit);
    vis.resize(gatesNumLimit,0);
    isVis.resize(gatesNumLimit,0);
    noneAssignPos.resize(gatesNums,0);
    noneAssign.reserve(gatesNums);
    candidate.reserve(gatesNums/10);
    nearPoint.reserve(gatesNums/10);
    noneAssign.resize(gatesNums,0);
    for(int i=0;i<gatesNums;i++) noneAssign[i]=noneAssignPos[i]=i;

    assigned.resize(gatesNumLimit);
    hasVal.resize(gatesNumLimit,false);
    unknowListL.resize(gatesNumLimit);
    unknowListR.resize(gatesNumLimit);
    unknowListLevel.resize(gatesNumLimit);
    ActiveCout[0].resize(gatesNumLimit,0);
    ActiveCout[1].resize(gatesNumLimit,0);

    watchList.reserve(gatesNumLimit);
    luby.resize(limit+1);
    luby[1]=luby[2]=1;
    for(int i=3,t=3,val=2;i<=limit;i++){
        if(i==t){luby[i]=val;t<<=1;t^=1;val<<=1;}
        else luby[i]=luby[i-val+1];
        luby[i]*=100;
    }
    for(int i=30;i>=0;i--){
        if(gatesNums&(1<<i)){
            high=i+1;
            break;
        }
    }
    rootVal=_getInv(graph.getGate(graph.getPOs().front()).get_outputs()[0]==HAVE_INV);
    for(GateId i=0;i<piNum;i++){
        Gate& gate=graph.getGate(i);
        Vec<GateId> inputs;
        SignalVal oWatchVal=1;
        GateId activeVal=gate.get_outputs().size();
        gateInfos.initGateInfo(i,inputs,oWatchVal,activeVal);
    }
    
    for(GateId i=piNum;i<gatesNums;i++){
        Gate& gate=graph.getGate(i);
        Vec<GateId> &inputs=gate.inputs();
        SignalVal oWatchVal=0; 
        GateId activeVal=gate.get_outputs().size();
        gateInfos.initGateInfo(i,inputs,oWatchVal,activeVal);
    }
    Print(deliB+" END INIT "+deliB+"\n",COLORS::black,MODELS::HighLight);
}
void Solver::returnLevel(GateId Level){
    // Print("From Level "+to_string(decisions.back().level)+" to Level "+to_string(Level),COLORS::blue,MODELS::HighLight);
    while(decisions.size()&&decisions.back().level>Level) Recovery();
    GateId newLevel=(decisions.size()==0?0:decisions.back().level);
    // Print("Return to Level "+to_string(newLevel),COLORS::green,MODELS::HighLight);
}
Status Solver::cdcl(){
    GateId root=graph.getPOs().front();
    DecInfo rootDec= DecInfo(1,{root,(SignalVal)(rootVal)},assignedNum);
    Print("Root Decision: ["+to_string(rootDec.decSign.gateId)+"] val: "+to_string(rootDec.decSign.wathchVal),COLORS::black,MODELS::HighLight);
    Print(deliB+"BEGIN CHECKING"+deliB,COLORS::black,MODELS::HighLight);
    decisions.push_back(rootDec);
    while(conflicNums<parameter.ConflicNum){
        allIter++;
        if(timer.isTimeOut()) return Status::TIMEOUT;       // timeout check
        DecInfo &decinfo=decisions.back();
        auto res=BCP();
        // PrintLevelInfo(decisions.back());
        if(res==Status::BCPSAT){
            Pin decSign=getNewDecLevel();
            DecInfo newDecInfo=DecInfo(decisions.back().level+1,decSign,assignedNum);
            decisions.push_back(newDecInfo);
            if(newDecInfo.decSign.gateId==HAVE_NO_CANDIDATE){
                for(GateId i=0;i<piNum;i++){
                    if(gateInfos.getGateInfo(i).val==HAVE_NO_VAL){
                        Pin decSign={i,0};
                        DecInfo newDecInfo=DecInfo(decisions.back().level+1,decSign,assignedNum);
                        decisions.push_back(newDecInfo);
                        if(BCP()!=Status::BCPSAT) return Status::UNSAT;
                    }
                }
                return Status::SAT;
            }

        }else{
            // Print(deliA+">>CONFLIC"); 
            conflicNums++;
            iter++;
            // gateInfos.ActiveBump();
            ConflicAnalysis();  
            // Print(deliA+">>CONFLIC ANALYSIZ END"); 
            if(isUnsat==1) {return Status::UNSAT;}
  
            restartTimes++;
            if(lubyIter<=limit&&restartTimes==luby[lubyIter]){
                //Print(deliA+">>Restart"); 
                returnLevel(1);
                restartTimes=0;
                lubyIter++;
            }
        }
    }
    return Status::UNKWON;
}
//-------------------------todo
Pin Solver::getNewDecLevel(){
    int maxVal=-1,maxActVal=0;
    Vec<GateId> candidate;
    Pin decsign={HAVE_NO_CANDIDATE,0};
    for(int i=0;i<gateInfos.getUncertainStatusGateNum();i++){
        auto &t=gateInfos.getGateInfo(i);
        maxActVal=max(t.val,maxActVal);
        if(t.val!=2) continue;
        if(t.activeVal>maxVal) {candidate.clear();candidate.push_back(t.gateId);maxVal=t.activeVal;}
        else if(t.activeVal==maxVal){candidate.push_back(t.gateId);}
    }
    if(candidate.size()!=0) 
    decsign={candidate[RandomGen().rand(candidate.size())],(SignalVal)RandomGen().rand(2)};
    if(maxActVal>=parameter.MaxReduceGateActVal) gateInfos.reduceNodeWeight();
    return decsign;
}
//-------------------------todo

Status Solver::BCP(){
    Status status=Status::BCPSAT;
    auto &decInfo=decisions.back();
    GateId &pos=decInfo.pos;
    if(decInfo.l==pos) { addNode(decInfo.decSign);}
    while(pos<assignedNum&&status!=Status::BCPCONFLIT){
        Pin &pin=assigned[pos++];
        GateInfo &gateInfo=gateInfos.getGateInfo(pin.gateId);
        assignGate(pin.gateId);
        if(!gateInfos.isRemove(pin.gateId)) unknowGate.insert(pin.gateId);
        gateInfo.val=pin.wathchVal;
        gateInfo.level=decInfo.level;
        gateInfo.timeStemp=pos;
        status=Transfer(pin,decInfo);
    }
    if(decInfo.unknowPos!=decInfo.unknowR){
        if(decInfo.unknowPos==HAVE_NO_LEVEL)decInfo.unknowPos=decInfo.unknowL;
        else {decInfo.unknowPos=unknowListR[decInfo.unknowPos];}
        while(true){
            unknowGate.erase(decInfo.unknowPos);
            if(decInfo.unknowPos==decInfo.unknowR) break;
            decInfo.unknowPos=unknowListR[decInfo.unknowPos];
        }
    }
    return status;
}
Status Solver::Transfer(Pin pin,DecInfo& decInfo){
    for(auto &oPin:gateInfos.getBackImpList(pin.gateId,pin.wathchVal)){
        if(gateInfos.isRemove(oPin.gateId)) continue;
        SignalVal oVal=gateInfos.getGateInfo(oPin.gateId).val;
        if(oVal==HAVE_NO_VAL){
            gateInfos.addTopology(oPin.gateId,pin.gateId);
            addNode(oPin);
            gateInfos.vitrualRemoveGate(oPin.gateId);
        }
        else if(oVal!=oPin.wathchVal){
            gateInfos.addTopology(oPin.gateId,pin.gateId);
            return Status::BCPCONFLIT;
        }
        else {
            addUnknowList(decInfo,oPin.gateId);
            gateInfos.vitrualRemoveGate(oPin.gateId);
        }
    }
    
    if(!isPI(pin.gateId)){
        GateInfo &gateInfo=gateInfos.getGateInfo(pin.gateId);
        if(gateInfo.val!=gateInfo.getGateWatchVal()){  
            bool isremove=true;
            for(auto iPin:gateInfo.pins){
                if(iPin.gateId==pin.gateId) continue;
                SignalVal &iVal=gateInfos.getGateInfo(iPin.gateId).val;
                if(iVal==HAVE_NO_VAL){
                    gateInfos.addTopology(iPin.gateId,pin.gateId);
                    addNode(iPin);
                    isremove=false;
                }
                else if(iVal!=iPin.wathchVal){
                    gateInfos.addTopology(iPin.gateId,pin.gateId);
                    return Status::BCPCONFLIT;
                }
            }
            if(isremove&&!gateInfos.isRemove(pin.gateId)){
                addUnknowList(decInfo,pin.gateId);
                gateInfos.vitrualRemoveGate(pin.gateId);
            }
        }

    }
    watchList=gateInfos.getWatchList(pin.gateId,pin.wathchVal);
    for(auto &point:watchList){
        if(gateInfos.isRemove(point)) continue;
        GateId newPoint=gateInfos.findNewWatchPoint(point,pin.gateId);
        if(newPoint!=pin.gateId){
            if(gateInfos.getGateInfo(newPoint).val==HAVE_NO_VAL){
                gateInfos.changGateWatchPoint(point,pin.gateId,newPoint);
            }
            else {
                GateInfo &faGateInfo=gateInfos.getGateInfo(point);
                if(faGateInfo.val!=HAVE_NO_VAL){addUnknowList(decInfo,point);}
                gateInfos.vitrualRemoveGate(point);
            }
        }
        else{
            GateInfo &faGateInfo=gateInfos.getGateInfo(point);
            GateId anotherPoint=gateInfos.getAnotherPoint(point,pin.gateId);
            SignalVal aVal=gateInfos.getGateInfo(anotherPoint).val;
            SignalVal aWatchVal=faGateInfo.getPin(anotherPoint).wathchVal;
            if(aVal==HAVE_NO_VAL){
                for(auto &tPin:faGateInfo.pins){
                    if(tPin.gateId!=anotherPoint){
                        gateInfos.addTopology(anotherPoint,tPin.gateId);
                    }
                }
                if(point>=gatesNums){gateInfos.resetGateLBD(point,getGateLbd(point));}
                addNode({anotherPoint,_getInv(aWatchVal)});
            }
            else if(aVal==aWatchVal){
                return Status::BCPCONFLIT;
            }
            else{
                if(faGateInfo.val!=HAVE_NO_VAL){addUnknowList(decInfo,point);}
                gateInfos.vitrualRemoveGate(point);
            }
        }
    }
    if(isPI(pin.gateId)&&!gateInfos.isRemove(pin.gateId))
    {
        addUnknowList(decInfo,pin.gateId);
        gateInfos.vitrualRemoveGate(pin.gateId);
    }
    return Status::BCPSAT;
}
void Solver::Recovery(){
    DecInfo& decInfo=decisions.back();
    if(decInfo.unknowL!=HAVE_NO_LEVEL){
        GateId st=decInfo.unknowL;
        while(true){
            if(gateInfos.getGateInfo(st).level!=decInfo.level){
                gateInfos.vitrualAddGate(st);
                unknowGate.insert(st); 
            }
            if(st==decInfo.unknowR) break;
            st=unknowListR[st];
        }
        
    }
    for(GateId id=decInfo.l;id<decInfo.pos;id++){
        disAssignGate(assigned[id].gateId);
    }
    for(GateId id=decInfo.l;id<assignedNum;id++){
        Pin &pin=assigned[id];
        GateInfo& gateIn=gateInfos.getGateInfo(pin.gateId);
        gateIn.reset();
        hasVal[pin.gateId]=false;
        if(gateInfos.isRemove(pin.gateId)==false) {
            unknowGate.erase(pin.gateId);
        }
        gateInfos.vitrualAddGate(pin.gateId);
    }
    for(GateId id=decInfo.l;id<assignedNum;id++){
        Pin &pin=assigned[id];
        gateInfos.resetPoint(pin.gateId);
    }
    assignedNum=decInfo.l;
    decisions.pop_back();
}
int Solver::ReduceFind(GateId stgateId){
    queue<GateId> q;
    q.push(stgateId);
    isVis[stgateId]=1;
    int nums=0;
    while(!q.empty()){
        GateId gateId=q.front();q.pop();
        GateInfo &gate=gateInfos.getGateInfo(gateId);
        if(gate.level==1) cout<<"no"<<endl;
        for(auto &i:gateInfos.getBackImpList(gateId,gate.val)){
            GateInfo &igate=gateInfos.getGateInfo(i.gateId);
            if(isVis[i.gateId]||igate.level==1) continue;
            isVis[i.gateId]=1;
            nums+=vis[i.gateId];
            vis[i.gateId]=0;
            allVis.push_back(i.gateId);
            q.push(i.gateId);
        }
        if(!isPI(gateId)&& gate.val!=gate.getGateWatchVal()){
            for(auto &i:gate.pins){
                GateInfo &igate=gateInfos.getGateInfo(i.gateId);
                if(isVis[i.gateId]||igate.level==1) continue;
                isVis[i.gateId]=1;
                nums+=vis[i.gateId];
                vis[i.gateId]=0;
                allVis.push_back(i.gateId);
                q.push(i.gateId);
            }
        }
    }
    isVis[stgateId]=0;
    return nums;
}
GateId Solver::FindConflicGate(){
    newGateInfo.clear();
    levels.clear();
    DecInfo& decInfo=decisions.back();
    GateId conflictGate=assigned[decInfo.pos-1].gateId;
    GateId uip=conflictGate,uipLevel=gateInfos.getGateInfo(conflictGate).timeStemp,backLevel=1;
    queue<GateId> q,qq;
    q.push(conflictGate);vis[q.front()]=1;
    newGateInfo.push_back({uip,0});
    while(!q.empty()){
        GateId gateId=q.front();q.pop();qq.push(gateId);
        gateInfos.getGateInfo(gateId).activeVal += gateInfos.ReturnActivate();
        if(uip!=conflictGate&&gateId==uip) continue;
        auto timeStemp=gateInfos.getGateInfo(gateId).timeStemp;
        if(timeStemp<uipLevel){  swap(uip,gateId); uipLevel=timeStemp; }   
        for(auto i:gateInfos.getGateSourceFa(gateId)){
            if(vis[i]) continue;
            auto &fa=gateInfos.getGateInfo(i);
            if(fa.level==1) continue;
            if(fa.level<decInfo.level){backLevel=max(backLevel,fa.level);newGateInfo.push_back({i,fa.val});levels.push_back(fa.level);}
            else {q.push(i);}
            vis[i]=1;
            gateInfos.getGateInfo(i).activeVal += gateInfos.ReturnActivate();
        }
    }
    newGateInfo.front()={uip,gateInfos.getGateInfo(uip).val};
    while(!qq.empty()){vis[qq.front()]=0;qq.pop();}
    if(newGateInfo.size()>10){
        Int nums=0;
        for(int ii=1;ii<newGateInfo.size();ii++){
            if(vis[newGateInfo[ii].gateId]){
                nums+=ReduceFind(newGateInfo[ii].gateId);
            }
        }
        // cout<<newGateInfo.size()<<" Reduce: "<<nums<<endl;
        for(int i=1;i<newGateInfo.size();){
            if(vis[newGateInfo[i].gateId]==0){
                swap(newGateInfo[i],newGateInfo.back());
                newGateInfo.pop_back();
            }else i++;
        }
        for(auto &i:allVis) isVis[i]=0;allVis.clear();
    }
    for(auto &i:newGateInfo) {vis[i.gateId]=0;} 

    if(backLevel==1) return backLevel;
    GateId backId=0,backTimeStemp=0;
    for(int i=1;i<newGateInfo.size();i++){
        auto &iGateInfo=gateInfos.getGateInfo(newGateInfo[i].gateId);
        if(iGateInfo.level==backLevel&&iGateInfo.timeStemp>backTimeStemp){
            backId=i;backTimeStemp=iGateInfo.timeStemp;
        }
    }
    swap(newGateInfo[1],newGateInfo[backId]);
    return backLevel;
}
Int Solver::getGateLbd(GateId gateId){
    Int lbd=0,a=0;
    Vec<Pin> &pins=gateInfos.getGateInfo(gateId).pins;
    for(auto &pin:pins){
        if(gateInfos.getGateInfo(pin.gateId).level==HAVE_NO_LEVEL) {a=1;continue;}
        if(vis[gateInfos.getGateInfo(pin.gateId).level])continue;
        vis[gateInfos.getGateInfo(pin.gateId).level]=1;
        lbd++;
    }
    for(auto &pin:pins){
        if(gateInfos.getGateInfo(pin.gateId).level==HAVE_NO_LEVEL) {continue;}
        vis[gateInfos.getGateInfo(pin.gateId).level]=0;
    }
    return lbd+a;
}
Int Solver::getGateLbd(){
    Int lbd=0;
    for(auto &level:levels){
        if(vis[level])continue;
        else vis[level]=1,lbd++;
    }
    for(auto &level:levels) vis[level]=0;
    return lbd;
}
void Solver::rmGate(GateId rmGateId){
    GateId backGateId=gateInfos.getGateInfoSize()-1;
    auto &rmGateInfo=gateInfos.getGateInfo(rmGateId);
    hasVal[rmGateId]=false;
    if(rmGateInfo.isremove){
        GateId lNode=unknowListL[rmGateId],rNode=unknowListR[rmGateId];
        DecInfo &decInfo=decisions[unknowListLevel[rmGateId]-1];
        if(rmGateId==decInfo.unknowPos){
            if(lNode!=rmGateId)decInfo.unknowPos=lNode;else decInfo.unknowPos=HAVE_NO_LEVEL;
        }
        if(lNode!=rmGateId&&rNode!=rmGateId){
            unknowListR[lNode]=rNode,unknowListL[rNode]=lNode;
        }else if(lNode!=rmGateId&&rNode==rmGateId){
            unknowListR[lNode]=lNode;
            decInfo.unknowR=lNode;
        }else if(rNode!=rmGateId&&lNode==rmGateId){
            unknowListL[rNode]=rNode;
            decInfo.unknowL=rNode;
        }else{
            decInfo.unknowL=decInfo.unknowR=HAVE_NO_LEVEL;
        }
        
    }else{ unknowGate.erase(rmGateId);rmGateInfo.isremove=true;}
    gateInfos.rmGate(rmGateId);
}
void Solver::ConflicAnalysis(){
    if(decisions.back().level==1) {isUnsat=1;return ;}
    GateId backLevel=FindConflicGate();
    GateId uip=newGateInfo.front().gateId;
    Pin uipPin={uip,_getInv(newGateInfo.front().wathchVal)};
    returnLevel(backLevel);
    addNode(uipPin);
    if(backLevel==1) return ;
    
    if(parameter.LearnGateNum&&newGateInfo.size()<parameter.LearGateLenLimit){
        if(iter>=parameter.MaxDeletHalfClause){
            int t=(gateInfos.getGateInfoSize()-gatesNums)/2;
            bool delOne=false;
            while(t>0){
                auto gateId=gateInfos.getRmGateGateId();
                if(delOne&&gateInfos.getGateInfo(gateId).pinsNum<8) break;
                delOne=true;
                rmGate(gateId);
                t--;
            }
            iter=0;
        }
        if(gateInfos.getGateInfoSize()<gatesNumLimit){
            auto newGate=gateInfos.addGate(newGateInfo);
            hasVal[newGate]=1;
            gateInfos.addGateAtherInfo(newGate,getGateLbd(),conflicNums);
            unknowGate.insert(newGate);
        }
        else if(parameter.LearnGateNum){
            GateId rmGateId=gateInfos.getRmGateGateId();
            rmGate(rmGateId);
            auto newGate=gateInfos.addGate(newGateInfo);
            hasVal[newGate]=1;
            gateInfos.addGateAtherInfo(newGate,getGateLbd(),conflicNums);
            unknowGate.insert(newGate);
        }
        
    }
        // cout<<newGateInfo.size()<<endl;
        for(auto i:newGateInfo) {
            if(i.gateId!=uip) {
                gateInfos.addTopology(uip,i.gateId);
            }
        }
    // }
    
}   
void Solver::check(std::ofstream& ofs){
    bool isSat=true;
    bool isAllAssignment=true;
    Print(deliA+">>"+"NOW ASSIGNMENT CHECKING");
    for(GateId gateId=piNum;gateId<gateInfos.getGateLimit();gateId++){
        auto &gateInfo=gateInfos.getGateInfo(gateId);
        if(gateInfo.val==NOT_EXIT) continue;
        if(gateInfo.val==HAVE_NO_VAL){isAllAssignment=false;continue;}
        else if(checkGate(gateInfo.gateId)){continue;}

        PrintGateInfo(gateId,COLORS::red);
        isSat=false;
    }
    if(isAllAssignment==false&&isSat) {ofs<<"check=UNKNOW"<<endl;Print("check: Not All Gate Has Assignment",COLORS::blue);}
    else if(isSat){ Print("check: SAT",COLORS::green);ofs<<"check=SAT"<<endl;}
    else {Print("check: UNSAT",COLORS::red);ofs<<"check=UNSAT"<<endl;}
    Print(deliA+">>"+"CHECKING END");
}
void Solver::PrintGateInfo(GateId gateId,Int color){
    auto &gateInfo=gateInfos.getGateInfo(gateId);
    Print("ERROR GATE INFO: ",color);
    Print("\tOUTPUT INFO: ",color);
    Print("\t\t GateId: "+to_string(gateInfo.gateId)+" Value: "+to_string(gateInfo.val)+" Watch val: ["+to_string(gateInfo.getGateWatchVal())+"] "+"Level: "+to_string(gateInfo.level)+" TimeStemp: "+to_string(gateInfo.timeStemp),color);
    Print("\tINPUT INFO: ",color);
    for(auto ipin:gateInfo.pins){
        if(ipin.gateId==gateId )continue;
        auto &igateInfo=gateInfos.getGateInfo(ipin.gateId);
        Print("\t\t GateId: "+to_string(igateInfo.gateId)+" Value: "+to_string(igateInfo.val)+" Watch val: ["+to_string(gateInfo.getPin(igateInfo.gateId).wathchVal)+"] "+"Level: "+to_string(igateInfo.level)+" TimeStemp: "+to_string(igateInfo.timeStemp),color);
    }
}
void Solver::PrintLevelInfo(DecInfo &decInfo){
    if(decInfo.l==decInfo.pos)
        Print("decision gate ID: ["+to_string(decInfo.decSign.gateId)+"] ");
    else
        Print("decision gate ID: ["+to_string(assigned[assignedNum-1].gateId)+"] ");
    Print("level: "+to_string(decInfo.level),COLORS::green);
}
bool Solver::checkGate(GateId gateId){
    
    auto &gateInfo=gateInfos.getGateInfo(gateId);
    int watchNum=0,none=0,nowatchNum=0;
    for(auto ipin:gateInfo.pins){
        if(ipin.gateId==gateId )continue;
        auto &igateInfo=gateInfos.getGateInfo(ipin.gateId);
        if(igateInfo.val==HAVE_NO_VAL){
            none++;
        }
        else if(igateInfo.val==gateInfo.getPin(ipin.gateId).wathchVal){
            watchNum++;
        }else nowatchNum++;   
    }
    if(gateInfo.gateId>=gatesNums||gateInfo.val==gateInfo.getGateWatchVal()){if(nowatchNum+none!=0) return true;}
    else{  if(nowatchNum==0) return true;}
    return false;
}

void Solver::draw(Str Title){
    Print(deliA+">>"+"Begin Drawing",COLORS::yellow);

    Str AndType="[shape=circle,style=filled,fillcolor=red,height=\"0.3\"]";
    Str AndTypeNoneVal="[shape=circle,style=filled,fillcolor=gold,height=\"0.3\"]";
    Str AndTypeVal1="[shape=circle,style=filled,fillcolor=white,height=\"0.3\"]";
    Str Connections="";
    Str arrowType="[];",invarrowTyp="[arrowhead=odot];";
    Str noneVal="";
    Str TrueVal="";
    Str FalseVal="";
    for(int i=0;i<gatesNums;i++){
        auto gateInfo=gateInfos.getGateInfo(i);
        if(gateInfo.val==HAVE_NO_VAL){
            noneVal=noneVal+","+to_string(i);
        }else if(gateInfo.val==1){
            TrueVal=TrueVal+","+to_string(i);
        }
        else FalseVal=FalseVal+","+to_string(i);
    }
    if(noneVal.size())
    {noneVal[0]='\t';    noneVal=noneVal+AndTypeNoneVal+";\n";}
    if(TrueVal.size())
    {TrueVal[0]='\t';    TrueVal=TrueVal+AndTypeVal1+";\n";}
    if(FalseVal.size())
    {FalseVal[0]='\t';    FalseVal=FalseVal+AndType+";\n";}
    
    for(int i=piNum;i<gatesNums;i++){
        auto gateInfo=gateInfos.getGateInfo(i);
        auto pins=gateInfo.pins;
        for(auto i:pins){
            Str tmp="\t";
            if(i.gateId==gateInfo.gateId) continue;
            Str st=to_string(i.gateId),en=to_string(gateInfo.gateId);
            tmp=st+"->"+en;
            if(i.wathchVal==0){
                tmp=tmp+invarrowTyp;
            }else tmp=tmp+arrowType;
            Connections=Connections+tmp+"\n";
            
        }
    }
    Str graph="digraph 0{\n"+noneVal+TrueVal+FalseVal+Connections+"}\n";
    Str dotPath=parameter.DrawDir+Title+".dot";
    Str svgPath=parameter.DrawDir+Title+".svg";
    ofstream ofs(dotPath,ios::out);
    ofs<<graph<<endl;
    ofs.close();
    Str order="dot -Tsvg  -o "+svgPath+" "+dotPath;
    int ans=system(order.c_str());
    if(ans==0){Print("RUN SUCCESSFUL!");}
    else {Print("RUN UNSUCCESSFUL!");}
}


} // namespace CircuitSAT
} // namespace WorkSpace


#pragma once

#include"Circuit/Circuit.h"
#include"Config/Config.h"
#include"Common/Random.h"
#include"Common/Timer.h"
#define HAVE_NO_LEVEL 0xffffffff
#define HAVE_NO_CANDIDATE 10000000
#define _getVal(x) ((x)&(SignalVal)1)
#define _getInv(x) (SignalVal)(((x)&(SignalVal)1)^(SignalVal)1)
#define _getGate(x) ((x)>>1)
namespace WorkSpace
{
    const SignalVal HAVE_NO_VAL=2;
    const SignalVal NOT_EXIT=3;
    static int high=0;
namespace CircuitSAT{
struct Parameter{
    LongInt ConflicNum=0;
    LongInt TimeLimit=0;
    LongInt IterLimit=0;
    Int LearnGateNum=0;
    Int LearGateLenLimit=0;
    Int MaxDeletHalfClause=0;
	LongInt MaxReduceGateActVal=0;

    bool isDraw=false;

    Str DrawDir;
    Str FileName;
    Str OutPutFile;
    void init(Config &config){
        ConflicNum=std::stoll(config.configOptions[Config::ConfigOptions::ConflicNum]);
        TimeLimit=std::stoll(config.configOptions[Config::ConfigOptions::TimeLimit]);
        IterLimit=std::stoll(config.configOptions[Config::ConfigOptions::IterTimes]);
        LearnGateNum=std::stoi(config.configOptions[Config::ConfigOptions::LearnGateLimit]);
        LearGateLenLimit=std::stoi(config.configOptions[Config::ConfigOptions::LearGateLenLimit]);
        MaxDeletHalfClause=std::stoi(config.configOptions[Config::ConfigOptions::MaxDeletHalfClause]);
        MaxReduceGateActVal=std::stoll(config.configOptions[Config::ConfigOptions::MaxReduceGateActVal]);

        isDraw=std::stoi(config.configOptions[Config::ConfigOptions::isDraw]);

        DrawDir=config.configOptions[Config::ConfigOptions::DrawDir];
        FileName=config.configOptions[Config::ConfigOptions::FileName];
        OutPutFile=config.configOptions[Config::ConfigOptions::OutputFile];
    }
};
enum class Status{
    SAT,
    UNSAT,
    BCPSAT,
    BCPCONFLIT,
    TIMEOUT,
    UNKWON
};

class GateInfo{
    public:
    GateId gateId;
    public :
    SignalVal val = HAVE_NO_VAL;
    GateId level=HAVE_NO_LEVEL;
    GateId  timeStemp=0;
    double activeVal=0;
    LongInt lbd=1000000;
    Int creatTime=0;
    bool isremove=0;

    public:
    GateId pinsNum=0;
    Vec<Pin> pins;
    public:
    inline void init(GateId _gateId){
        gateId=_gateId;
    }
    inline void reset(){
        val=HAVE_NO_VAL;
        level=HAVE_NO_LEVEL;
        timeStemp=0;
    }
    inline void initGateInfo(Vec<GateId> &inputs,SignalVal oWatchVal,GateId _activeVal){
        activeVal=_activeVal;
        if(!inputs.size()) return ;
        pinsNum=inputs.size()+1;
        pins.resize(pinsNum);  
        pins[0].gateId=gateId;
        pins[0].wathchVal=oWatchVal;
        for(int i=1;i<=inputs.size();i++){
            Pin &pin=pins[i];
            SignalVal iSignalVal=inputs[i-1];
            GateId iGateId=_getGate(inputs[i-1]);
            pin.gateId=iGateId;
            pin.wathchVal=(SignalVal)_getInv(inputs[i-1]);
        }   
    }
    inline void initGateInfo(Vec<Pin> &_pins){
        val=0,isremove=0,level=1;
        pinsNum=_pins.size();
        pins.resize(pinsNum);
        pins=_pins;return ;
    }
    //-------------------------------------
    inline LongInt getGateAcitveVal(){
        LongInt ans=((long long)lbd<<high)+pinsNum;
        return ans;
    }
    //--------------------------------------todo
    inline SignalVal getGateWatchVal(){
        if(pins[0].gateId==gateId) return pins[0].wathchVal;
        else return 0;
    }
    inline Pin& getPin(GateId _gateId){
        for(int i=0;i<pins.size();i++){ if(pins[i].gateId==_gateId) return pins[i];}
        Print("ERROR: Gate["+to_string(gateId)+"] want to find not exit pin: "+to_string(
            _gateId),COLORS::red,MODELS::HighLight);
        for(int i=0;i<pins.size();i++){ cout<<pins[i].gateId<<" ";}cout<<endl;

        return pins[0];
    }   
    ~GateInfo(){
        pins.clear();
    }
};
class GateInfos{
private:
    GateId gateInfoNum=0,pis=0,gateSize=0;
    Vec<GateInfo> gateinfos;
    Vec<GateId> insertIds;

    Vec<Vec<GateId>> source;

    Vec<Arr<Vec<GateId>,2>> watchVec;
    Vec<Vec<pair<Pin,Int>>> watchVectId;
    Vec<GateId> watchPointNum;

    Vec<Arr<Vec<Pin>, 2>> backImpLists;

    set<pair<LongInt,GateId>> gateLBDS;

    double activate_bump = 2;
    
public:
    inline void reduceNodeWeight(){
        for(int i=0;i<gateInfoNum;i++) gateinfos[i].activeVal/=500;
    }
    inline void ActiveBump(){
        activate_bump *= activate_bump;
    }
    inline double ReturnActivate(){
        return activate_bump;
    }
    //init
    void init(GateId gatesNum,GateId _pis,GateId gatesNumLimit){
        gateInfoNum=gatesNum;
        gateSize=gatesNum;
        pis=_pis;

        backImpLists.resize(gateInfoNum);
        gateinfos.resize(gatesNumLimit);
        watchVectId.resize(gatesNumLimit);
        insertIds.reserve(gatesNumLimit-gatesNum);
        for(int i=gatesNum;i<gatesNumLimit;i++){
            insertIds.push_back(i);
            gateinfos[i].val=NOT_EXIT;
        }
        watchPointNum.resize(gatesNumLimit,0);

        watchVec.resize(gateInfoNum);
        source.resize(gateInfoNum);

        for(GateId id=0;id<gatesNum;id++){
            watchVectId[id].resize(2);
            getGateInfo(id).init(id);
        }
        for(GateId id=gatesNum;id<gatesNumLimit;id++){
            watchVectId[id].resize(2);
            getGateInfo(id).pins.reserve(10);
        }
    }
    inline void initGateInfo(GateId gateId,Vec<GateId> &inputs,SignalVal oWatchVal,GateId activeVal){
        GateInfo &gateInfo=getGateInfo(gateId);
        gateInfo.initGateInfo(inputs,oWatchVal,activeVal);
        watchPointNum[gateId]=0;
        if(!inputs.size()) return ;
        for(auto &signal:inputs){
            addBackImp(_getGate(signal),_getVal(signal),gateId,oWatchVal);
        }
        for(GateId &i=watchPointNum[gateId];i<2;i++){
            Pin& pin=gateInfo.pins[i];
            watchVectId[gateId][i]={pin,watchVec[pin.gateId][pin.wathchVal].size()};
            watchVec[pin.gateId][pin.wathchVal].push_back(gateId);
        }
    }
    //backInpList
public:
inline Vec<Pin> & getBackImpList(GateId gateId,SignalVal _val){return backImpLists[gateId][_val];}
    inline void addBackImp(GateId gateId,SignalVal _val,GateId &impGateId,SignalVal &impVal){
        backImpLists[gateId][_val].push_back({impGateId,impVal});
    }
    inline void rmBackImp(GateId gateId, GateId _gateid,SignalVal _val){
        auto &backList=backImpLists[gateId][_val];
        int i=backList.size()-1;
        while(i>=0&&backList[i].gateId!=_gateid){i--;}
        swap(backList[i],backList.back());
        backList.pop_back();
    }

    //Pointer Manage
public:
    inline Vec<GateId>& getWatchList(GateId gateId,SignalVal watchVal){return watchVec[gateId][watchVal];}
    inline void rmGateWatchPoint(GateId gateId,GateId oldGateId)    {
        watchPointNum[gateId]--;
        GateId watchPointBackId=watchPointNum[gateId];
        if(watchVectId[gateId][0].first.gateId==oldGateId) {
            swap(watchVectId[gateId][0],watchVectId[gateId][watchPointBackId]);}
        auto &pointWatchList=watchVectId[gateId][watchPointBackId];
        Int pos=pointWatchList.second;
        Pin pin=pointWatchList.first;

        auto &bucket=watchVec[pin.gateId][pin.wathchVal];
        GateId backGateId=bucket.back();
        Int backid=(watchVectId[backGateId][0].first.gateId!=oldGateId);
        Int &backPos=watchVectId[backGateId][backid].second;

        swap(bucket[pos],bucket[backPos]);
        backPos=pos;
        bucket.pop_back();
    }
    inline void addGateWatchPoint(GateId gateId,GateId newGateId){
        GateInfo &gateInfo=getGateInfo(gateId);
        Pin pin=gateInfo.getPin(newGateId);
        watchVectId[gateId][watchPointNum[gateId]++]={pin,watchVec[pin.gateId][pin.wathchVal].size()};
        watchVec[pin.gateId][pin.wathchVal].push_back(gateId);
    }
    inline void changGateWatchPoint(GateId gateId,GateId oldGateId,GateId newGateId){
        rmGateWatchPoint(gateId,oldGateId);
        addGateWatchPoint(gateId,newGateId);
    }
    inline GateId findNewWatchPoint(GateId gateId,GateId point){
        GateInfo &gateInfo=getGateInfo(gateId);
        int id2=(watchVectId[gateId][0].first.gateId==point);
        GateId point2=watchVectId[gateId][id2].first.gateId;
        GateId newPoint=point;
        for(auto &pin:gateInfo.pins){
            if(pin.gateId==point||pin.gateId==point2) continue;
            auto &t=getGateInfo(pin.gateId);
            if(t.val==pin.wathchVal) continue;
            
            if(t.val==2) newPoint=pin.gateId;
            else {newPoint=pin.gateId;break;}
        }
        if(newPoint!=point) {return newPoint;}
        else {return point;} 
    }
    inline GateId getAnotherPoint(GateId gateId,GateId point){
        int id=(watchVectId[gateId][0].first.gateId==point?1:0);
        GateId point2=watchVectId[gateId][id].first.gateId;
        return point2;
    }
private:
    inline Arr<GateId,2> getPoint(GateId gateId){
        return {watchVectId[gateId][0].first.gateId,watchVectId[gateId][1].first.gateId};
    }
    inline Int getPointId(GateId gateId,GateId point){
        return (watchVectId[gateId][0].first.gateId!=point);
    }
    //Source Topology Manage
public:
    inline void addTopology(GateId st,GateId en){source[st].push_back(en);}
    inline void removeTopology(GateId st){source[st].pop_back();}
    inline Vec<GateId>& getGateSourceFa(GateId gateId){ return source[gateId];}
    //Gate Manage
public:
    inline GateInfo& getGateInfo(GateId gateId){ return gateinfos[gateId];}
    inline bool isRemove(GateId gateId){
        return  gateinfos[gateId].isremove;
    }
    inline void vitrualRemoveGate(GateId gateId){
        gateinfos[gateId].isremove=true;
    }
    inline void vitrualAddGate(GateId gateId){
        gateinfos[gateId].isremove=false;
    }
    inline void resetGateLBD(GateId gateId,Int lbd){
        GateInfo &gateInfo=getGateInfo(gateId);
        if(gateId<gateInfoNum){gateInfo.lbd=lbd;return ;}
        gateLBDS.erase({gateInfo.getGateAcitveVal(),gateId});
        gateInfo.lbd=lbd;
        gateLBDS.insert({gateInfo.getGateAcitveVal(),gateId});
    }
    inline GateId getRmGateGateId(){
        GateId gateId=(*gateLBDS.rbegin()).second;
        return gateId;
    }
    inline GateId addGate(Vec<Pin> &pins){
        GateId newGateId=insertIds.back();
        insertIds.pop_back();
        GateInfo &newGate=gateinfos[newGateId];
        SignalVal owatchVal=0;
        gateSize++;
        newGate.init(newGateId);
        newGate.initGateInfo(pins);
        for(auto &pin:pins)addBackImp(pin.gateId,_getInv(pin.wathchVal),newGateId,owatchVal);
        
        watchPointNum[newGateId]=0;
        for(int i=0;i<2;i++){
            Pin& pin=pins[i];
            watchVectId[newGateId][watchPointNum[newGateId]++]={pin,watchVec[pin.gateId][pin.wathchVal].size()};
            watchVec[pin.gateId][pin.wathchVal].push_back(newGateId);
        }
        return newGateId;
    }
    inline void addGateAtherInfo(GateId gateId,Int lbd,Int creatTime){
        GateInfo &gateInfo=getGateInfo(gateId);
        gateInfo.lbd=lbd;
        gateInfo.creatTime=creatTime;
        gateLBDS.insert({gateInfo.getGateAcitveVal(),gateId});

    }
    inline void rmGate(GateId rmGateId){
        GateInfo rmGateInfo=getGateInfo(rmGateId);
        insertIds.push_back(rmGateId);
        gateLBDS.erase({rmGateInfo.getGateAcitveVal(),rmGateId});
        auto watchList=watchVectId[rmGateId];
        for(auto &point:watchList){rmGateWatchPoint(rmGateId,point.first.gateId);}
        for(auto &pin:rmGateInfo.pins){
            rmBackImp(pin.gateId,rmGateId,_getInv(pin.wathchVal));
        }
        --gateSize;
        getGateInfo(rmGateId).val=NOT_EXIT;
        // Print("Has Rm Gate: "+to_string(rmGateId));
        return ;
    }
    
    //getNum
    inline GateId getUncertainStatusGateNum(){return gateInfoNum;}
    inline GateId getGateInfoSize(){return gateSize;}
    inline GateId getGateLimit(){return gateinfos.size();}

    //reset:
    inline void resetPoint(GateId gateId){
        source[gateId].clear();
        if(watchPointNum[gateId]==0) return;
        if(watchVectId[gateId][0].first.gateId==gateId){
            swap(watchVectId[gateId][0],watchVectId[gateId][1]);
        }
        if(watchVectId[gateId].back().first.gateId!=gateId)
            changGateWatchPoint(gateId,watchVectId[gateId][watchPointNum[gateId]-1].first.gateId,gateId);
        // Pin otherPoint=watchVectId[gateId].front().first;
        // if(getGateInfo(otherPoint.gateId).val!=HAVE_NO_VAL){
        //     GateId point=findNewWatchPoint(otherPoint.gateId,gateId);
        //     changGateWatchPoint(gateId,otherPoint.gateId,point);
        // }
    }
};
class Solver{
    public:
    struct DecInfo{
        GateId level;
        Pin decSign;
        GateId l=0,pos=0;//左闭右开
        GateId unknowL=0,unknowR=0,unknowPos;
        // Vec<Pin> assigned;
        DecInfo(int _level,Pin _decSign,GateId num):level(_level),decSign(_decSign){
            l=pos=num;
            unknowL=unknowPos=unknowR=HAVE_NO_LEVEL;
        }
    };
    // base info
    private:
        AigerGraph &graph;
        GateId gatesNums,gatesNumLimit;
        GateId piNum;
        GateId poNum;
        SignalVal rootVal;       
        Parameter parameter;
    
        GateInfos gateInfos;
        Vec<DecInfo> decisions;

        LongInt iter=0,allIter=0;
        GateId conflicNums=0;
        bool isUnsat=0;
        Timer timer;
    private:
        int limit=1000,lubyIter=1,restartTimes=0;;
        Vec<LongInt> luby;
		inline void init(){
			luby.resize(limit+1);
			luby[1]=luby[2]=1;
			for(int i=3,t=3,val=2;i<=limit;i++){
				if(i==t){luby[i]=val;t<<=1;t^=1;val<<=1;}
				else luby[i]=luby[i-val+1];
				luby[i]*=100;
			}
		}
    private:
        Arr<Vec<GateId>,2> ActiveCout;
    private:
        GateId assignedNum=0;
        Vec<Pin> assigned;
        Vec<GateId> unknowListL,unknowListR,unknowListLevel;
        set<pair<LongInt,GateId>,std::greater<pair<long long,GateId>>> noneGate;
        unordered_set<GateId> unknowGate;
        Vec<bool> hasVal;
        inline void addNode(Pin pin){
            if(hasVal[pin.gateId]==true)return ;
            hasVal[pin.gateId]=true,assigned[assignedNum++]=pin;
        }
        inline void addUnknowList(DecInfo &decInfo, GateId &gateId){
            unknowListL[gateId]=unknowListR[gateId]=gateId;
            unknowListLevel[gateId]=decInfo.level;
            if(decInfo.unknowL==HAVE_NO_LEVEL)decInfo.unknowL=gateId;
            else swap(unknowListR[decInfo.unknowR],unknowListL[gateId]);
            decInfo.unknowR=gateId;
        }
    private:
        Vec<GateId> vis,noneAssign,noneAssignPos,isVis;
        Vec<Pin> newGateInfo;
        Vec<GateId> levels;
        Vec<GateId> watchList;
        Vec<Pin> nearPoint;
        Vec<Pin> candidate;
        Vec<GateId> allVis;
        inline void assignGate(GateId gateid){
            swap(noneAssignPos[gateid],noneAssignPos[noneAssign.back()]);
            swap(noneAssign[noneAssignPos[gateid]],noneAssign[noneAssignPos[noneAssign.back()]]);
            noneAssign.pop_back();
            // noneGate.erase({gateInfos.getGateInfo(gateid).activeVal,gateid});
        }
        inline void disAssignGate(GateId gateid){
            noneAssignPos[gateid]=noneAssign.size();
            noneAssign.push_back(gateid);
            // noneGate.insert({gateInfos.getGateInfo(gateid).activeVal,gateid});
        }
    private:
        inline bool isPI(GateId gateId){return gateId<piNum;}

    private:
        Status BCP();
        Status Transfer(Pin pin,DecInfo& decInfo);
        void Recovery();
        void rmGate(GateId rmGateId);
        void ConflicAnalysis();
        Int getGateLbd(GateId gateId);
        Int getGateLbd();
        Int ReduceFind(GateId gateId);
        GateId FindConflicGate();
        Pin getNewDecLevel();
        void returnLevel(GateId Level);


    //check 
    private:
        void check(std::ofstream& ofs);
        void PrintLevelInfo(DecInfo &decInfo);
        void PrintGateInfo(GateId gateId,Int color=COLORS::black);
        bool checkGate(GateId gateId);
    //draw
    private:
        void draw(Str Title);
    //time
    private:
        inline bool isTimeOut(){return iter>=1;}

    public:
        Solver(AigerGraph &_graph,Config &config);
        Status cdcl();
        Status dpll();// todoList


        void run(Config &config);
}; // class Solver

} // namespace CircuitSAT
} // namespace WorkSpace


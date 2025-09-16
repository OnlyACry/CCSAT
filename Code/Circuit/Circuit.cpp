#include"Circuit/Circuit.h"

namespace WorkSpace{
namespace CircuitSAT{
void AigerGraph::addGateInit(Vec<GateId> &signals){
    assert(!isInvSignal(signals[0]));
    GateId gateId=getSignalGateId(signals[0]);
    Gate &gate=getGate(gateId);

    gate.inputs().resize(signals.size()-1);
    for(GateId i=1;i<signals.size();i++){
        GateId signal=signals[i];
        GateId signalGateId=getSignalGateId(signal);
        gate.inputs()[i-1]=signal-2;
        getGate(signalGateId).outputs().push_back(gateId);
    }
}
void AigerGraph::addPI(GateId signal){
    assert(!isInvSignal(signal));
    GateId gateId=getSignalGateId(signal);
    PIs.push_back(gateId);
}
void AigerGraph::addPO(GateId signal){
    GateId gateId=getSignalGateId(signal);
    if(isInvSignal(signal)) getGate(gateId).outputs().push_back(HAVE_INV);
    else getGate(gateId).outputs().push_back(HAVE_NO_INV);
    POs.push_back(gateId);
}

} // namespace CircuitSAT
} // namespace WorkSpace




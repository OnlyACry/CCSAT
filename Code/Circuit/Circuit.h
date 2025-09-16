#pragma once
#include"Config/TypeDef.h"
#ifndef CIRCUIT_H
#define CIRCUIT_H

#define NULL_INDEX 0xffffffff
#define HAVE_INV 0xfffffffe
#define HAVE_NO_INV 0xfffffffd

#define INPUT_NUM 2u
using namespace std;
namespace WorkSpace{
namespace CircuitSAT{
enum GateType {
	AND,
	INVERT,
	XOR
};
class Gate{
private:
    GateId gateId{NULL_INDEX};
    Vec<GateId> m_inputs=Vec<GateId>(INPUT_NUM,0u);
    Vec<GateId> m_outputs;
public:
    const bool is_input() const { return m_inputs[0] == NULL_INDEX; }
	const bool is_output() const { return m_outputs.size() == 1 && (m_outputs[0] == HAVE_INV || m_outputs[0] == HAVE_NO_INV); }

	const GateId &get_id() const { return gateId; }
	GateId &id() { return gateId; }

	const Vec<GateId> &get_inputs() const { return m_inputs; }
	Vec<GateId> &inputs() { return m_inputs; }

	const Vec<GateId> &get_outputs() const { return m_outputs; }
	Vec<GateId> &outputs() { return m_outputs; }



};
class AigerGraph{
private:
	GateId	gate_nums=0u;
	GateId	pis_nums=0u;
	GateId	pos_nums=0u;
	Vec<Gate>	gates;
	Vec<GateId>	gateId;
	Vec<GateId>	PIs;
	Vec<GateId>	POs;
public:
	void SpaceInit(GateId gateNums,GateId PIs_num,GateId POs_num){
		gates.resize(gateNums);
		for(GateId id=0;id<gates.size();id++){
			gates[id].id()=id;
		}
		PIs.reserve(PIs_num);
		POs.reserve(POs_num);
		gate_nums=gateNums;
		pis_nums=PIs_num;
		pos_nums=POs_num;
	}
	Gate& getGate(GateId gate_id){return gates[gate_id];}

	GateId getPIsNum(){return PIs.size();}
	GateId getPOsNum(){return POs.size();}
	GateId getGatesNum(){return gates.size();}
	Vec<GateId>& getPIs(){return PIs;};
	Vec<GateId>& getPOs(){return POs;};
public:
	void addGateInit(Vec<GateId> &signals);
	void addPI(GateId signal);
	void addPO(GateId signal);
	inline GateId getSignalGateId(GateId signal){return ((signal>>1)-1);}
	inline bool isInvSignal(GateId signal){return (signal&1);}


};
} // namespace CircuitSAT
} // namespace WorkSpace
#endif



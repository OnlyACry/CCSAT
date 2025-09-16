#include "Solver/Solver.h"
#include "Circuit/Parser.h"
using namespace std;
using namespace WorkSpace;
using namespace CircuitSAT;
int main(int argc, char* argv[]){
    AigerGraph gate;
    Parser  parser;
    Config config;
    config.parse(argc,argv);
    ifstream ifs;
    ifs.open(config.configOptions[Config::ConfigOptions::AigerFile],ios::in);
    if(parser.parse(ifs,gate)==false){return 0;}
    Solver s=Solver(gate,config);
    s.run(config);
    return 0;
}

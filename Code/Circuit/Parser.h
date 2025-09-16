#pragma once
#include"Circuit/Circuit.h"
#include"Common/StringUtil.h"
#ifndef PARSER_H
#define PARSER_H


namespace WorkSpace{
namespace CircuitSAT{
class Parser{
public:
    bool parse(std::istream &is,AigerGraph & graph);
    //bool parse(std::istream&is,AigerGraph & graph);
};
} // namespace CircuitSAT
} // namespace WorkSpace
#endif



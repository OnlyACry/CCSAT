#pragma once

#include <set>
#include <map>
#include <cmath>
#include <ctime>
#include <array>
#include <queue>
#include <thread>
#include <chrono>
#include <cassert>
#include <vector>
#include <string>
#include <random>
#include <utility>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <memory>

//using namespace std;

namespace WorkSpace {


using Int = int;
using UInt = unsigned int;
using LongInt = long long;
using Real = double;
using Bool = bool;
using Str = std::string;
using ULongInt = unsigned long long;
using Millisecond = long long;

using GateId =UInt;
using SignalId=UInt;
using GateVal = long long;
using SignalVal=Int ;
typedef struct  PIN
{
    GateId gateId;
    SignalVal wathchVal;
    bool operator<(const PIN b) const  
    {  
        return this->gateId < b.gateId;  
    }  
    bool operator>(const PIN b) const  
    {  
        return this->gateId > b.gateId;  
    }  
} Pin;
const Str deliA="====================";
const Str deliB="--------------------";

template<typename T, size_t Size>
using Arr = std::array<T, Size>;

template<typename T>
using Vec = std::vector<T>;

template<typename T>
using Set = std::set<T>;

template<typename T>
using MultiSet = std::multiset<T>;

template<typename T>
using HashSet = std::unordered_set<T>;

template<typename Key, typename Value>
using Map = std::map<Key, Value>;

template<typename Key, typename Value>
using HashMap = std::unordered_map<Key, Value>;

template<typename T> 
using SharePtr = std::shared_ptr<T>;

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

template<typename NewType, typename OldType>
constexpr NewType sCast(OldType obj) { return static_cast<NewType>(obj); }

inline long long sqr(long long x) { return 1ll * x * x;}

enum COLORS{
    grey=30,
    red,
    green,
    yellow,
    blue,
    purple,
    cyan,
    white,
    black,
    grey_1,
    background_grey,
    background_red,
    background_green,
    background_yellow,
    background_blue,
    background_purple,
    background_cyan,
    background_white,
    background_black,
    background_grey_1,
};
enum MODELS{
    Normal=0,
    HighLight=1,
    UnderLine=4,
    ReverseDisplay=7

};
inline void Print(Str _str,Int _color=COLORS::black,Int _model=MODELS::Normal){
	std::cout<< "\033["<<_model<<";"<<_color<<"m"<<_str<<"\033[0m"<<std::endl;
}
} // namespace WorkSpace

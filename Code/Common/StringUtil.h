#pragma once
#include<iostream>
#include<string>
#include"Config/TypeDef.h"
#ifndef STRING_UTIL_DEF
#define STRING_UTIL_DEF

namespace WorkSpace{
class StringUtil{
    public:
    void filter_str(Str&str,char ch){
        str.erase(std::remove_if(str.begin(),str.end(),[&](char c){return c==ch;}),str.end());
    }
    void string_split_by_delimiter(const Str& _str,Vec<Str>&arr,const char delimiter){
        Str str=_str;
        if(_str.back()!=delimiter) str+=delimiter;
        Int pos=0;
        for(int i=0;i<str.size();i++){
            if(str[i]==delimiter){
                arr.push_back(str.substr(pos,i-pos));
                pos=i+1;
            }
        }
    }
};
} // namespace WorkSpace
#endif

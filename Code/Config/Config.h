#pragma once
#include"Config/TypeDef.h"
#include"Common/StringUtil.h"
#define VNAME(name) (#name)

namespace WorkSpace
{
	//#define __DEBUG__ CMAKE_DEBUG_INFO
	#define __DEBUG__ 1
struct Config{
	enum ConfigOptions {
		HomeDir,
		AigerFile,
		FileName,
		OutputFile,
		DrawDir,
		TimeLimit,
		ConflicNum,
		IterTimes,
		LearnGateLimit,
		LearGateLenLimit,
		MaxDeletHalfClause,
		MaxReduceGateActVal,
		isDraw,
        Size
	};

	static constexpr char delim = '=';

	const HashMap<Str, ConfigOptions> ConfigOptionKeys = {
		{"-home",HomeDir},
		{"-aiger", AigerFile},
		{"-FileName", FileName},
		{"-output", OutputFile},
		{"-draw", DrawDir},
		{"-t", TimeLimit},
		{"-c", ConflicNum},
		{"-iter",IterTimes},
		{"-learnGateNum",LearnGateLimit},
		{"-learGateLenLimit",LearGateLenLimit},
		{"-maxDeletHalfClause",MaxDeletHalfClause},
		{"-maxReduceGateActVal",MaxReduceGateActVal},
		{"-isdraw",isDraw}
	};

	const HashMap<Str, ConfigOptions> OutputName = {
		{"",HomeDir},
		{"", AigerFile},
		{"File Name", FileName},
		{"", OutputFile},
		{"", DrawDir},
		{"Time Limit(s)", TimeLimit},
		{"Conflic Limit", ConflicNum},
		{"",IterTimes},
		{"LearnGate Num Limit",LearnGateLimit},
		{"LearnGate Len Limit",LearGateLenLimit},
		{"MaxIter DeletClause",MaxDeletHalfClause},
		{"MaxVal ReduceAct",MaxReduceGateActVal},
		{"",isDraw}

	};



	const Arr<Str, ConfigOptions::Size> DefaultConfigOptions = {
		getHomeDir(),// HomeDir
		DefaultConfigOptions[ConfigOptions::HomeDir]+"Data/aagFile/ctrl_miter.aag",// aigFile
		"",// Filename
		"", // OutPut File
		DefaultConfigOptions[ConfigOptions::HomeDir]+"Draw/",// DrawDir
		"100",// TimesLimit
		"100000000",// ConflicNum
		"100000000",// IterTimes
		"10000",// LearnGateLimit
		"100",// LearGateLenLimit
		"100000",//MaxDeletHalfClause
		"10000000",//MaxReduceGateActVal
		"0"
	};

	Arr<Str, ConfigOptions::Size> configOptions;
	Str getHomeDir(){
		const Str filePath=__FILE__;
		Vec<Str> arr;
		StringUtil stringUtil;
		stringUtil.string_split_by_delimiter(filePath,arr,'/');
		arr.pop_back();arr.pop_back();arr.pop_back();
		Str homePath="/";
		for(auto i:arr) if(i=="")continue;else homePath=homePath+i+"/";
		return homePath;
	}
	Str getFileName(Str _str){
		Vec<Str> arr;
		StringUtil stringUtil;
		stringUtil.string_split_by_delimiter(_str,arr,'/');
		_str=arr.back();
		arr.clear();
		stringUtil.string_split_by_delimiter(_str,arr,'.');
		return arr[0];
	}
	// Provides an entry using the specified placement options
	Str getOption(ConfigOptions option) const {
    	return configOptions[option];
	}
	void writeConfigToFile(std::ofstream& ofs) const {
		if (!ofs.is_open()) {
			std::cerr << "Error: Output file stream is not open!" << std::endl;
			return;
		}

		for (const auto& [key, option] : OutputName) {
			std::string value = getOption(option);
			if(key=="") continue;
			ofs << key << "=" << value << std::endl;
			
		}
	}

	void print_platform() {
		#ifdef _MSC_VER
			Print("Microsoft Visual C++ compiler",COLORS::red);
		#elif defined(__GNUC__)
			Print("GNU Compiler Collection (GCC)",COLORS::red);
		#elif defined(__clang__)
			Print("Clang compiler",COLORS::red);
		#elif defined(__MINGW32__)
			Print("MinGW compiler",COLORS::red);
		#endif
		#ifdef __APPLE__
			Print("Apple platform",COLORS::red);
		#elif defined(__linux__)
			Print("Linux platform",COLORS::red);
		#else
			Print("Unknown platform",COLORS::red);
		#endif
	}
	void parse(int argc, char* argv[]) {
		Print("--------------------------Run Paltform Info--------------------------",COLORS::black);
		
		Print("Compile Time: "+(Str)__TIME__+" "+(Str)__DATE__,COLORS::red); 
		print_platform();
		Print("DEBUG INFO: "+to_string(__DEBUG__),COLORS::red); 
		Print("Config File: "+(Str)__FILE__,COLORS::red); 
		Print("=====================================================================",COLORS::black);

		Vec<Str> strs(2);
		for (int i = 1; i < argc; i+=2) {
			strs[0] = argv[i];
			strs[1] = argv[i + 1];
			auto index = ConfigOptionKeys.find(strs[0]);
			if (index == ConfigOptionKeys.end()) continue;
			/* cout << "parse key : " << strs[0] << " , value : " << strs[1] << " , index : " << index->second << endl; */
			configOptions[index->second] = strs[1];
		}
		for (int i = 0; i < ConfigOptions::Size; i++) {
			if (configOptions[i].empty()) {
				configOptions[i] = DefaultConfigOptions[i];
			}
		}
		configOptions[ConfigOptions::FileName]=getFileName(configOptions[ConfigOptions::AigerFile]);
		if(configOptions[ConfigOptions::OutputFile]=="")
			configOptions[ConfigOptions::OutputFile]=configOptions[ConfigOptions::HomeDir]+"Output/"+configOptions[ConfigOptions::FileName]+".txt";
		
		Print("Project Direction: "+configOptions[ConfigOptions::HomeDir],COLORS::blue);
		Print("Miter  File Path: "+configOptions[ConfigOptions::AigerFile],COLORS::blue);
		Print("Output File Path: "+configOptions[ConfigOptions::OutputFile],COLORS::blue);
		Print("---------------------------------------------------------------------",COLORS::black);

	}
};
    
} // namespace WorkSpace

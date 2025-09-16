#!/bin/bash

RunFileName="cirsat"
WorkDir=$(readlink -f $(dirname "$0")/../)
SearchDirName="aagFile"
# DataDir=$(readlink -f $(dirname "$0")/../../)/Data/"$SearchDirName"
DataDir=${WorkDir}/Data/"$SearchDirName"  
ResDir=${WorkDir}/Result
Output=$WorkDir/Output
LogDir=$WorkDir/Log

if [ ! -d "$LogDir" ]; then
    mkdir "$LogDir"
fi
if [ ! -d "$ResDir" ]; then
    mkdir "$ResDir"
fi
if [ ! -d "$Output" ]; then
    mkdir "$Output"
fi

log="1"

# fileType="*miter.aag"
isdraw=0

# -------------------------Config-------------------------
declare -A config
config['-t']="1"   # Time Limit  
config['-c']="10000000000"       # Conflic Num Limit
config['-learnGateNum']="10000" # Learn Gate Num Limit
config['-learGateLenLimit']="100" # Learn Gate Len Limit
config['-maxDeletHalfClause']="100000" # Learn Gate Num Limit
config['-maxReduceGateActVal']="10000000" # Learn Gate Len Limit
config['-isdraw']="0" #isDraw
number=1
parameter=($@)
while [ "$number" -lt $# ]; do
    val="${parameter[$number]}"
    if [ "${parameter[$number-1]}" = "-file" ] ; then
        fileType="$val"
        number=$((number + 2))
        continue 
    elif [ "${parameter[$number-1]}" = "-log" ] ; then
        log="$val"
        number=$((number + 2))
        continue 
    elif [ "${parameter[$number-1]}" = "-DirName" ] ; then
        SearchDirName="$val"
        DataDir=${WorkDir}/Data/"$SearchDirName" 
        # DataDir=$(readlink -f $(dirname "$0")/../../)/Data/"$SearchDirName" 
        number=$((number + 2))
        continue 
    fi
    configOption="${parameter[$number-1]}"
    config[$configOption]="$val"
    number=$((number + 2))
done


#=========================================================
#-------------------------Ans-----------------------------
Ans=(
    "GateNum"
    "ANS"
    "check"
    "RunTime"
    "All Iter"
    "Conflic Num"
    "Learnt Gate Count"
    "LearnGate Len Limit"
    "LearnGate Num Limit"
    "Conflic Limit"
    "Time Limit(s)"
    "MaxIter DeletClause"
    "MaxVal ReduceAct"
)

#=========================================================


# buildDir="$WorkDir/Code/build"
# if [ ! -d "$buildDir" ]; then
#     mkdir "$buildDir"
#     cd "$buildDir" && cmake ../
# fi


# cd "$buildDir" && cmake ../ && make
RunFile=$WorkDir/Code/build/$RunFileName
CURRENT_TIME=$(date '+[%Y%m%d]%H_%M_%S')

FindOrderType="$DataDir -name $fileType"
files=$(find $FindOrderType)

ResFile=$ResDir/$CURRENT_TIME.csv
GlobalResFile=$ResDir/result.csv


TitleList="File Name"
for title in "${Ans[@]}";
do
    TitleList=$TitleList,$title
done

echo "$TitleList" > "$ResFile"

if [ ! -f "$GlobalResFile" ]; then
    echo "$TitleList" > "$GlobalResFile"
fi

echo "$CURRENT_TIME"
title=""
if [ "$files" = "" ]; then
    exit 0
fi

files=$(du -h $(echo $files) | sort -h | awk '{print $2}')


for file in $files; 
do 
    aigerFile=$file 
    aigFile=${file%%.*}.aig
    fileName=$(basename "${aigerFile}")

    OutPutFile=$Output/${fileName%%.*}.txt
    if [ -f "$OutPutFile" ] ; then
        rm "$OutPutFile"
    fi
    LogFile=$LogDir/${fileName%%.*}.log
   
    # if [ ! -f "$aigFile" ] ; then
    #     convertOrder="$aigerPath $aigerFile $aigFile"
    #     $convertOrder
    # fi
    
    order="${RunFile} -aiger $aigerFile -output $OutPutFile "

    if [ "${config["-isdraw"]}" = "1" ];
    then
        drawPath=$WorkDir/Draw/${fileName%%.*}.svg 
        if [ -f "$drawPath" ] ; then
            continue 
        fi
        order="$order -isdraw 1"
        echo "$order"
        $order
        continue 
    fi


    for i in "${!config[@]}";
    do
        order="$order $i ${config[$i]}"
    done
    
    echo -e "\033[31m$order > $LogFile\033[0m"
    if [ "$log" == "1" ] ; then
        ($order) > "$LogFile"
    else 
        $order
    fi
    AnsLine=$fileName
    for table in "${Ans[@]}";
    do
        # echo $table
        # val=$(awk -F "$table" '/=/{print $1}' "$OutPutFile")
        # if [ "$table" = "abcCheck" ] ; then
        #     checkOrder="$demoFile $aigFile"
        #     ($checkOrder) >> "$LogFile"
        #     checkInfo=$(sed -n "/equivalent/p" $LogFile)
        #     val=$($checkOrder | awk -F "are " '{print $2}' | awk -F '.' '{print $1}' )
        #     echo $val
        #     if [ "$val" = "equivalent" ] ;then
        #         val="UNSAT"
        #     fi 

        #     AnsLine=$AnsLine,check
        #     continue 
        # fi
        val=$(sed -n "/$table/p" $OutPutFile | awk -F '=' '{print $2}' )

        if [ "$val" = "" ] ;then
            val="None"
        fi 
        AnsLine=$AnsLine,$val
    done
    echo "$AnsLine" >> "$ResFile"
    echo "$AnsLine" >> "$GlobalResFile"
    cat "$OutPutFile"
done

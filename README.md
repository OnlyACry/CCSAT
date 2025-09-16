# CCSAT

### `Code/run.sh`运行脚本使用

- `run.sh` 运行脚本各个参数默认值

  ```shell
  '-t'="1"   # Time Limit  
  '-c'="10000000000"       # Conflic Num Limit
  '-learnGateNum'="10000" # Learn Gate Num Limit
  '-learGateLenLimit'="100" # Learn Gate Len Limit
  '-maxDeletHalfClause'="100000" # Learn Gate Num Limit
  '-maxReduceGateActVal'="10000000" # Learn Gate Len Limit
  '-isdraw'="0" 	#isDraw  "1"是执行画图程序执行画图程序，不执行代码,默认不执行
  '-file'="*miter.aag" #文件名
  '-log'="1"  #是否打印log 0则输出终端，1则输出到log文件中
  '-DirName'="aagFile"  #搜寻文件夹
  ```

- `run.sh` 运行例子

  - 运行ISCAS85算例c开头的文件

  ```shell
  ./run.sh -file "*.aag" -t 2 -log 1 -learnGateNum 10000 -DirName "benchmark/ISCAS85"
  ```

  - 输出控制台

  ```shell
  ./run.sh -file "*.aag" -t 20 -log 0 -learnGateNum 10000 
  ```

  - 对每个文件输出`.dot`图和`.svg`图到`Draw`文件夹下

  ```shell
  ./run.sh -isdraw 1
  ```



  


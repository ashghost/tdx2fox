# 飞狐指标公式插件 <-桥接-> 通达信指标公式插件

- [飞狐指标公式插件 <-桥接-> 通达信指标公式插件](#%e9%a3%9e%e7%8b%90%e6%8c%87%e6%a0%87%e5%85%ac%e5%bc%8f%e6%8f%92%e4%bb%b6--%e6%a1%a5%e6%8e%a5--%e9%80%9a%e8%be%be%e4%bf%a1%e6%8c%87%e6%a0%87%e5%85%ac%e5%bc%8f%e6%8f%92%e4%bb%b6)
  - [功能](#%e5%8a%9f%e8%83%bd)
  - [文件目录](#%e6%96%87%e4%bb%b6%e7%9b%ae%e5%bd%95)
  - [编译依赖](#%e7%bc%96%e8%af%91%e4%be%9d%e8%b5%96)
  - [编译](#%e7%bc%96%e8%af%91)
  - [使用](#%e4%bd%bf%e7%94%a8)
    - [部署](#%e9%83%a8%e7%bd%b2)
    - [调用](#%e8%b0%83%e7%94%a8)
  - [拓展函数](#%e6%8b%93%e5%b1%95%e5%87%bd%e6%95%b0)

## 功能

使飞狐指标公式可以调用通达信DLL插件

## 文件目录

```
│  .gitignore
│  Cppfile.json (cpp-builder配置)
│  dlls.ini (通达信库加载配置)
│  README.md (本文件)
|
├─dist (输出目录)
│
├─include
│  ├─fox
│  │      FoxFunc.h (飞狐插件头文件)
│  │
│  └─tdx
│         PluginTCalcFunc.h (通达信插件头文件)
│
├─msvs (VS项目文件)
│  │  tdx2fox.sln
│  │  tdx2fox.vcxproj
│  │  tdx2fox.vcxproj.filters
│
└─src
   |  export.def (dll导出函数)
   |  ext_functions.cpp (拓展函数)
   |  main.cpp (主函数)
```

## 编译依赖

* `Windows`平台
* 支持`c++11`的编译器

  * `static_assert`
  * `std::array`
  * `std::shared_ptr`

## 编译

* `Cpp-Builder`
  * `cppb`

* `Visual Stdio`
  * `msvs\tdx2fox.sln`

## 使用

**请确保飞狐[CALCINFO, CALCPARAM](include/fox/FoxFunc.h)结构二进制兼容**

### 部署

1.假设库名为`TDX2FOX.dll`

2.将库放入飞狐插件指定目录,如`C:\Fox\FmlDLL\TDX2FOX.dll`

3.在库所在的目录下放入[dlls.ini](./dlls.ini),此文件与通达信配置一致,参考通达信

### 调用

假设挂载通达信的库在DLL3上,`dlls.ini`如下
```
[BAND]
band1=
band2=
band3=C:\new_tdx\T0002\dlls\tdxdll.dll
band4=
band5=
band6=
band7=
band8=
band9=
band10=
```

* 通达信代码
```
A:TDXDLL3(1, H, L, C);
```

* 飞狐代码
```
A:"TDX2FOX@TDXDLL3"(1, H, L, C);
```

## 拓展函数

* `PERIOD`

  取得周期类型.

  结果从0到13,依次分别是1/5/15/30/60分钟,日/周/月,多分钟,多日/季/年,5秒线/多秒线,13以上为自定义周期

  注: **不支持 5秒线/季/年/自定义周期** , 参考[PERIOD](src/ext_functions.cpp)

  通达信
  ```
  A:PERIOD;
  ```

  飞狐
  ```
  A:"TDX2FOX@PERIOD()";
  ```

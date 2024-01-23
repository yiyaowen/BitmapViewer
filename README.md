# BitmapViewer 位图工具

BitmapViewer 是一个使用 C 语言开发的位图查看、处理工具。

![](https://github.com/yiyaowen/BitmapViewer.Image/blob/main/test/result/code_page/about.bmp)

## 灰度变换

1. 提取彩色图灰度值（经验公式、伽马校正）
2. 单像素变换（取反、对数、伽马）
3. 直方图处理（均衡化、规定化）

## 空域滤波

1. 模糊卷积核（盒式、高斯）
2. 锐化卷积核（拉普拉斯、索贝尔）
3. 非线性滤波（中值）

## 频域滤波

1. 功率谱提取（DFT、FFT）
2. 相位谱提取（DFT、FFT）
3. 传递函数处理（陷波、提升、同态滤波）

## 多图操作

1. 算数运算（+，-，×，÷）
2. 频谱重建图像（指定功率谱、相位谱）

## 模拟噪声

1. 高斯噪声
2. 瑞利噪声
3. 爱尔兰拟合（伽马噪声，暂不支持）
4. 指数噪声
5. 均匀噪声
6. 椒盐（冲激）噪声

## 项目构建

按照常规 CMake 使用流程进行构建即可。例如使用 CMake + MSBuild 工具组合如下：

```bat
> cd Path/to/BitmapViewer
> mkdir build && cd build
> cmake ..
> msbuild BitmapViewer.sln
```

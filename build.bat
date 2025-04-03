@echo off
echo 清理构建目录...
if exist build (
    rmdir /s /q build
)

echo 创建构建目录...
mkdir build
cd build

echo 生成构建文件...
cmake ..

echo 开始构建...
cmake --build .

echo 完成！

pause 
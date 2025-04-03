# PID调参工具

这是一个基于Qt开发的PID调参工具，用于通过串口与PID控制器通信，实时调整PID参数并显示目标值和实际值的曲线图。

## 功能特点

1. 串口通信：支持选择串口和波特率
2. PID参数设置：可实时调整Kp、Ki、Kd、目标值
3. 实时波形显示：显示目标值和实际值的实时波形图
4. 交互式图表：支持缩放、平移等操作

## 通信协议

### 发送格式
- 帧头：0xA5
- 数据长度：0x0A（固定长度）
- 命令1：0xBA（固定值）
- 命令2：
  - 0x14：设置Kp值
  - 0x15：设置Ki值
  - 0x16：设置Kd值
  - 0x17：设置目标值
- 数据：4字节浮点数
- 校验和：从帧长开始到数据结束的和
- 帧尾：0x5A

### 接收格式
- 帧头：0xA5
- 数据长度：（可变）
- 命令1：0xAB（固定值）
- 命令2：
  - 0x12：实际值
  - 0x13：目标值确认
- 数据：4字节浮点数
- 校验和：从帧长开始到数据结束的和
- 帧尾：0x5A

## 使用方法

1. 选择正确的串口和波特率
2. 点击"打开串口"按钮连接设备
3. 调整PID参数（Kp、Ki、Kd）和目标值
4. 观察波形图，分析控制效果
5. 必要时可使用"数据发送"区域发送自定义数据

## 构建项目

```bash
# 创建构建目录
mkdir build && cd build

# 使用CMake生成构建文件
cmake ..

# 编译
cmake --build .
```

## 依赖项

- Qt 5.12+ 或 Qt 6.0+
- QtSerialPort
- QtCharts 
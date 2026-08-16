# DataAcqHMI — 工业数据采集上位机（Qt6 示例）

> 展示 **Qt 多线程 + 串口/Modbus 数据采集 + 实时曲线** 能力的开源示例。
> 作者：邱友军（qiuyoujun）｜ Qt/C++ 10 年，主导过工业现场数据采集上位机项目。

## 这个项目讲什么

工业现场的下位机（PLC / 仪表）通常通过 **RS-485 + Modbus RTU** 暴露寄存器。
本上位机示例演示如何**稳定、不掉帧**地把这些数据采上来并实时呈现：

```
┌────────────┐   RS-485      ┌──────────────┐  信号/槽     ┌────────────┐
│ 下位机/PLC │ ────────────▶ │ SerialWorker │ ───────────▶ │ MainWindow │
│ (Modbus)   │   Modbus RTU  │ (工作线程)   │  DataPoint   │ (UI 线程)  │
└────────────┘               └──────────────┘              └────────────┘
                                 ▲                          │
                                 │ QTimer 周期轮询          │ QChart 实时曲线
                                 └──────────────────────────┘
```

关键设计（真实项目沉淀）：

| 痛点 | 解法 |
| --- | --- |
| 串口阻塞导致界面卡死 | 采集放在 **独立 QThread 工作线程**，UI 永不被阻塞 |
| 协议解析散落各处 | 收敛到 `ProtocolParser`（CRC16 / 帧解析可单测） |
| 高频数据内存爆炸 | **环形缓冲** `DataBuffer`，只保留最近 N 点 |
| 跨线程数据传递出错 | 全程 **信号/槽（队列连接）**，无裸共享内存 |

## 技术亮点

- **Qt6 + C++17**，CMake 构建
- **QSerialPort**：串口配置、读写、超时控制
- **Modbus RTU**：手写 CRC16 与读保持寄存器帧解析（零第三方依赖）
- **QThread + 信号槽**：采集与界面彻底分离
- **Qt Charts**：`QLineSeries` 实时滚动曲线
- **环形缓冲**：内存恒定，长时间运行不泄漏

## 目录结构

```
DataAcqHMI/
├── CMakeLists.txt
├── src/
│   ├── main.cpp                     # 入口：装配线程与界面
│   ├── core/
│   │   ├── datapoint.h              # 数据点模型
│   │   ├── protocolparser.h/.cpp    # Modbus RTU CRC16 + 帧解析
│   │   └── serialworker.h/.cpp      # 串口采集工作线程
│   └── ui/
│       ├── databuffer.h/.cpp        # 环形缓冲（Model 层）
│       └── mainwindow.h/.cpp        # 主界面 + 实时曲线
```

## 构建与运行

依赖：Qt 6.2+（Core / Widgets / SerialPort / Charts）

```bash
# Windows
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.7.0/msvc2022_64"
cmake --build build --config Release
./build/Release/DataAcqHMI.exe

# Linux / macOS
cmake -S . -B build
cmake --build build
./build/DataAcqHMI
```

> 没有真实下位机也能验证：界面会显示"未连接"。如需联调，可用 Modbus 从机模拟器
> （如 QModMaster / 串口回环）监听对应 COM 口。

## 真实案例脱敏版

本仓库是作者落地的**工业数据采集上位机**的**教学脱敏版**，保留了多线程采集、
Modbus 协议栈、实时曲线等核心架构，去掉了客户专有点位表与业务逻辑。
完整企业级方案（多通道、断线重连、数据落库、报警联动）可通过猿急送联系作者。

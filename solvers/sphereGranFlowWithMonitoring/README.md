# sphereGranFlowWithMonitoring 求解器

## 概述

`sphereGranFlowWithMonitoring` 是一个基于 `sphereGranFlow` 的DEM求解器，增加了监控框功能。该求解器在每个时间步迭代之前统计指定长方体区域内的颗粒数量，如果数量为0则自动停止模拟。

## 功能特性

- 继承自 `sphereGranFlow` 的所有功能
- 实时监控指定区域内的颗粒数量
- 当监控区域内无颗粒时自动停止模拟
- 支持通过配置文件或默认值设置监控区域
- 使用Kokkos并行计算优化性能

## 编译

确保在主构建目录中重新运行CMake以包含新求解器：

```bash
cd build
cmake ..
make -j$(nproc)
```

或者只编译这个求解器：

```bash
cd build
make -j$(nproc) sphereGranFlowWithMonitoring
```

## 使用方法

### 基本用法

在案例根目录下运行：

```bash
sphereGranFlowWithMonitoring
```

### 配置监控框

可以通过两种方式配置监控框：

#### 方法1：在settingsDict中配置

在您的案例目录下的`settingsDict`文件中添加以下内容：

```cpp
monitoringBox
{
    minPoint (-0.1 -0.1 -0.1);  // 最小坐标点
    maxPoint ( 0.1  0.1  0.1);  // 最大坐标点
}
```

#### 方法2：使用默认值

如果不配置监控框，将使用默认值：
- 最小点: (-0.1, -0.1, -0.1)
- 最大点: ( 0.1,  0.1,  0.1)

## 输出信息

求解器会定期输出以下信息：
- 当前时间和迭代次数
- 总颗粒数
- 监控框内的颗粒数

每10次迭代或当监控框内颗粒数为0时会输出统计信息。

示例输出：
```
Time = 0.001, Iteration = 100, Total particles = 1000, Particles in monitoring box = 50
Time = 0.002, Iteration = 200, Total particles = 980, Particles in monitoring box = 45
...
No particles in monitoring box. Stopping simulation.
```

## 停止条件

模拟将在以下情况下停止：
1. 监控框内颗粒数为0且总颗粒数大于0
2. 系统中完全没有颗粒
3. 达到预设的最大迭代次数

## 应用场景

1. **颗粒流出模拟**：监测容器出口处的颗粒流动，当无颗粒流出时停止
2. **分离过程**：监测特定区域的颗粒分离情况
3. **填充过程**：监测目标区域的填充状态
4. **实验验证**：与实验数据对比，当达到特定条件时停止

## 示例

参考`tutorials/sphereGranFlow/`目录下的案例，只需将运行命令从`sphereGranFlow`改为`sphereGranFlowWithMonitoring`即可使用此功能。

## 测试

运行测试脚本来验证安装：

```bash
./solvers/sphereGranFlowWithMonitoring/test_solver.sh
```

## 注意事项

- 监控框应该在物理上有意义的区域设置
- 确保监控框大小适合您的模拟场景
- 可以根据需要调整输出频率（修改代码中的`% 10`部分）
- 监控计算使用Kokkos并行化，性能已优化

## 更多信息

详细的技术文档和开发指南请参考 [DEVELOPMENT.md](DEVELOPMENT.md)。
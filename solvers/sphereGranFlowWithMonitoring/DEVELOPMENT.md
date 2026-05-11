# sphereGranFlowWithMonitoring 求解器开发文档

## 概述

`sphereGranFlowWithMonitoring` 是一个基于 `sphereGranFlow` 的DEM（离散元方法）求解器，增加了智能监控功能。该求解器在每个时间步迭代之前统计指定长方体区域内的颗粒数量，如果数量为0则自动停止模拟。

## 主要特性

1. **继承原有功能**：完全继承 `sphereGranFlow` 的所有功能，包括：
   - 球形颗粒的DEM模拟
   - 颗粒插入机制
   - 移动几何体支持
   - 多种接触力模型

2. **智能监控**：
   - 实时监控指定区域内的颗粒数量
   - 当监控区域内无颗粒时自动停止模拟
   - 定期输出统计信息

3. **灵活配置**：
   - 支持通过配置文件设置监控区域
   - 提供默认监控区域
   - 可调整输出频率

## 技术实现

### 核心算法

```cpp
// 在每个时间步之前执行
if (useMonitoringBox) {
    // 1. 获取颗粒位置数据
    const auto& positions = sphParticles.dynPointStruct().pointPosition();
    
    // 2. 使用Kokkos并行计算统计监控框内的颗粒数
    Kokkos::parallel_reduce("countParticlesInBox",
        policy(0, numActive),
        LAMBDA_HD(pFlow::uint32 i, pFlow::uint32& count) {
            if (monitoringBox.isInside(positionsView(i))) {
                count++;
            }
        },
        particlesInBox);
    
    // 3. 检查停止条件
    if (particlesInBox == 0 && totalParticles > 0) {
        break; // 停止模拟
    }
}
```

### 性能优化

- 使用Kokkos并行归约算法高效统计颗粒数量
- GPU加速计算（如果可用）
- 最小化主机-设备数据传输

## 文件结构

```
solvers/sphereGranFlowWithMonitoring/
├── CMakeLists.txt                    # 构建配置
├── sphereGranFlowWithMonitoring.cpp  # 主程序文件
├── createDEMComponents.hpp           # DEM组件创建头文件
├── README.md                         # 用户指南
├── DEVELOPMENT.md                    # 开发文档（本文件）
├── settingsDict.example              # 示例配置文件
└── test_solver.sh                    # 测试脚本
```

## 编译和安装

### 前提条件

- CMake 3.10+
- Kokkos库
- phasicFlow依赖库

### 编译步骤

```bash
# 进入构建目录
cd build

# 配置CMake
cmake ..

# 编译新求解器
make -j$(nproc) sphereGranFlowWithMonitoring
```

### 验证安装

```bash
# 运行测试脚本
./solvers/sphereGranFlowWithMonitoring/test_solver.sh

# 或直接运行帮助
./solvers/sphereGranFlowWithMonitoring/sphereGranFlowWithMonitoring -h
```

## 使用方法

### 基本用法

在案例根目录下运行：

```bash
sphereGranFlowWithMonitoring
```

### 配置监控框

#### 方法1：配置文件方式

在 `settingsDict` 文件中添加：

```cpp
monitoringBox
{
    minPoint (-0.1 -0.1 -0.1);  // 最小坐标点
    maxPoint ( 0.1  0.1  0.1);  // 最大坐标点
}
```

#### 方法2：使用默认值

如果不配置，将使用默认监控框：
- 最小点: (-0.1, -0.1, -0.1)
- 最大点: ( 0.1,  0.1,  0.1)

### 输出信息

求解器会定期输出：
```
Time = 0.001, Iteration = 100, Total particles = 1000, Particles in monitoring box = 50
Time = 0.002, Iteration = 200, Total particles = 980, Particles in monitoring box = 45
...
No particles in monitoring box. Stopping simulation.
```

## 应用场景

1. **颗粒流出模拟**：监测容器出口处的颗粒流动，当无颗粒流出时停止
2. **分离过程**：监测特定区域的颗粒分离情况
3. **填充过程**：监测目标区域的填充状态
4. **实验验证**：与实验数据对比，当达到特定条件时停止

## 注意事项

1. **监控框选择**：
   - 应选择物理上有意义的区域
   - 确保监控框大小适合模拟场景
   - 避免监控框过大或过小

2. **性能考虑**：
   - 监控计算在每个时间步执行
   - 对于大规模模拟，考虑调整输出频率
   - Kokkos并行计算已优化性能

3. **停止条件**：
   - 仅在监控框内无颗粒且总颗粒数>0时停止
   - 如果系统中完全没有颗粒也会停止
   - 达到最大迭代次数也会停止

## 扩展开发

### 添加更多监控功能

可以扩展代码以支持：
- 多个监控区域
- 不同形状的监控区域（球体、圆柱等）
- 基于其他条件的停止准则（速度、温度等）

### 修改输出频率

在代码中找到：
```cpp
if (Control.time().currentIter() % 10 == 0 || particlesInBox == 0)
```

修改 `% 10` 为其他值来调整输出频率。

## 故障排除

### 编译错误

如果遇到编译错误：
1. 确保所有依赖库已正确安装
2. 检查CMake配置
3. 清理构建目录后重新编译

### 运行时错误

1. 检查 `settingsDict` 文件格式
2. 验证监控框坐标是否合理
3. 查看日志文件获取详细错误信息

## 贡献指南

欢迎贡献代码和改进建议：
1. Fork项目
2. 创建功能分支
3. 提交更改
4. 创建Pull Request

## 许可证

本项目遵循GNU General Public License v3或更高版本。

## 联系方式

如有问题或建议，请联系开发团队。

---

**最后更新**: 2026年4月28日
**版本**: 1.0

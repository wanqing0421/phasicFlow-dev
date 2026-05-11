# sphereGranFlowWithMonitoring 快速参考

## 🚀 快速开始

```bash
# 编译
cd build && make -j$(nproc) sphereGranFlowWithMonitoring

# 运行（在案例目录中）
sphereGranFlowWithMonitoring
```

## 📋 配置监控框

在 `settingsDict` 文件中添加：

```cpp
monitoringBox
{
    minPoint (-0.1 -0.1 -0.1);  // 最小坐标
    maxPoint ( 0.1  0.1  0.1);  // 最大坐标
}
```

## 🔍 功能说明

- ✅ 每个时间步前统计监控框内颗粒数
- ✅ 颗粒数为0时自动停止模拟
- ✅ 每10次迭代输出统计信息
- ✅ 使用Kokkos并行优化性能

## 📊 输出示例

```
Time = 0.001, Iteration = 100, Total particles = 1000, Particles in monitoring box = 50
Time = 0.002, Iteration = 200, Total particles = 980, Particles in monitoring box = 45
No particles in monitoring box. Stopping simulation.
```

## 🎯 应用场景

- 颗粒流出监测
- 分离过程控制
- 填充状态检测
- 实验条件匹配

## 📁 文件位置

```
求解器源码: solvers/sphereGranFlowWithMonitoring/
可执行文件: build/solvers/sphereGranFlowWithMonitoring/sphereGranFlowWithMonitoring
文档: solvers/sphereGranFlowWithMonitoring/README.md
```

## 🔧 常用命令

```bash
# 查看帮助
sphereGranFlowWithMonitoring -h

# 运行测试
./solvers/sphereGranFlowWithMonitoring/test_solver.sh

# 清理重新编译
cd build && rm -rf solvers/sphereGranFlowWithMonitoring && make sphereGranFlowWithMonitoring
```

## ⚠️ 注意事项

1. 监控框应设置在物理有意义的区域
2. 确保监控框大小适合模拟场景
3. 默认监控框: (-0.1,-0.1,-0.1) 到 (0.1,0.1,0.1)
4. 可通过修改代码中的 `% 10` 调整输出频率

## 📚 更多信息

- 用户指南: README.md
- 开发文档: DEVELOPMENT.md
- 项目总结: PROJECT_SUMMARY.md
- 配置示例: settingsDict.example

---
**版本**: 1.0 | **日期**: 2026-04-28

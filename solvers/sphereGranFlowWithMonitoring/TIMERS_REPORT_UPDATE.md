# 颗粒数量报告间隔更新说明

## 更新内容

将颗粒数量打印的时间间隔从固定的每10次迭代，改为与 `settingsDict` 中的 `timersReportInterval` 配置保持一致。

## 修改前后对比

### 修改前
```cpp
// 固定每10次迭代输出一次
if (Control.time().currentIter() % 10 == 0 || particlesInBox == 0) {
    REPORT(0)<<"Time = "<<Control.time().currentTime()
        <<", Iteration = "<<Control.time().currentIter()
        <<", Total particles = "<<totalParticles
        <<", Particles in monitoring box = "<<particlesInBox
        <<END_REPORT;
}
```

### 修改后
```cpp
// 使用 settingsDict 中的 timersReportInterval 配置
if (Control.time().timersReportTime() || particlesInBox == 0) {
    REPORT(0)<<"Time = "<<Control.time().currentTime()
        <<", Iteration = "<<Control.time().currentIter()
        <<", Total particles = "<<totalParticles
        <<", Particles in monitoring box = "<<particlesInBox
        <<END_REPORT;
}
```

## 配置方法

在 `settingsDict` 文件中设置 `timersReportInterval`：

```cpp
timeControl
{
    startTime       0;
    endTime         10;
    dt              1e-5;
    
    // 控制timer报告的时问间隔（秒）
    timersReportInterval    0.01;  // 每0.01秒报告一次
    
    writeInterval   100;
}
```

## 工作原理

`timersReportTime()` 方法会：
1. 读取 `settingsDict` 中的 `timersReportInterval` 值
2. 根据当前时间和该间隔判断是否应该报告
3. 返回 `true` 表示应该报告，`false` 表示不报告

这样可以确保：
- 所有 timer 报告（包括颗粒数量）使用相同的时间间隔
- 用户可以通过配置文件统一控制报告频率
- 报告基于时间而非迭代次数，更加科学

## 优势

1. **统一配置**: 所有报告使用相同的间隔配置
2. **灵活控制**: 通过修改 `timersReportInterval` 即可调整报告频率
3. **时间基准**: 基于物理时间而非迭代次数，更适合不同时间步长的模拟
4. **一致性**: 与其他 timer 报告保持一致的行为

## 示例

### 示例1：频繁报告
```cpp
timersReportInterval    0.001;  // 每0.001秒报告一次
```

### 示例2：稀疏报告
```cpp
timersReportInterval    0.1;    // 每0.1秒报告一次
```

### 示例3：默认值
如果不设置，默认值为 `0.04` 秒。

## 注意事项

- 即使在不报告的时刻，如果 `particlesInBox == 0`，仍然会输出信息（用于检测停止条件）
- 第一次迭代（iter=0）不会报告
- 报告间隔是基于物理时间，不是迭代次数
- 实际报告时刻可能会有小的误差（±0.55*dt）

## 相关文件

- 源代码: `solvers/sphereGranFlowWithMonitoring/sphereGranFlowWithMonitoring.cpp`
- 配置示例: `solvers/sphereGranFlowWithMonitoring/settingsDict.example`
- Time控制: `src/phasicFlow/repository/Time/timeControl.hpp`

---

**更新日期**: 2026-04-29  
**版本**: 1.1

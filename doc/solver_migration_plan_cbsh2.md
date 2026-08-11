# SkyMAPF 迁移 CBSH2-RTC 初步方案

## 1. 任务理解

SkyMAPF 是围绕 MAPF（Multi-Agent Path Finding，多智能体路径规划）搭建一个可复用、可扩展的算法与工程框架。

选择 Jiaoyang Li 的一个较成熟 MAPF 项目，学习其算法结构和工程组织方式，并将其中适合 SkyMAPF 的部分抽象成通用模块。

因此，本次迁移应重点关注：

- 如何拆分 MAPF solver 的核心模块；
- 如何设计 SkyMAPF 的通用 solver 接口；
- 如何统一输入、输出和 benchmark 形式；
- 如何让后续 CBS、CBSH2、EECBS、LNS 等算法都能接入；
- 如何避免只针对某一个项目做硬编码。

---

## 2. 参考项目选择

本次初步选择 Jiaoyang Li 的 CBSH2-RTC 作为主要参考项目。

CBSH2-RTC 是一个面向 MAPF 的 optimal solver，核心基于 Conflict-Based Search（CBS），并集成了多种 CBS 改进技术，包括：

- prioritizing conflicts；
- bypassing conflicts；
- high-level admissible heuristics，例如 CG、DG、WDG；
- symmetry reasoning techniques，例如 rectangle reasoning、target reasoning、corridor reasoning；
- mutex propagation；
- disjoint splitting。

MAPF 的核心问题是：在给定地图中，为多个 agent 从各自起点移动到目标点规划路径，同时避免它们之间发生路径冲突。

选择 CBSH2-RTC 的原因包括：

1. 它是 Jiaoyang Li 的 MAPF 相关项目，具有较强代表性；
2. 它直接面向 Multi-Agent Path Finding，与 SkyMAPF 的项目方向高度一致；
3. 它基于 CBS 这一经典 MAPF 框架，适合作为 SkyMAPF 基础 solver 的参考；
4. 它包含多种可逐步迁移的 CBS 改进技术，适合形成从 basic CBS 到 advanced CBS-family solver 的开发路线；
5. 它使用 USC Research License，而 SkyMAPF 使用 MIT License，因此迁移时不应直接复制代码，而应参考算法思想和模块结构，在 SkyMAPF 中重新实现通用框架。

---

## 3. 迁移目标

本次迁移不是复刻 CBSH2-RTC，也不是直接搬运其源代码，而是参考它的算法结构，为 SkyMAPF 设计一个更通用的 MAPF solver 模块。

短期目标是先实现 basic CBS 的框架，包括：

- Solver 统一接口；
- MAPFInstance 输入表示；
- Solution 输出表示；
- Path 路径表示；
- Conflict 冲突检测；
- Constraint 约束表示；
- LowLevelPlanner 单智能体路径搜索；
- HighLevelSearch 多智能体冲突处理流程。

暂时不迁移的内容包括：

- 不直接复制 CBSH2-RTC 源代码；
- 不一开始实现所有高级优化；
- 不把代码写死成只能支持 CBSH2-RTC；
- 不优先考虑商业部署和复杂工业接口。

---

## 4. 通用框架设计

SkyMAPF 后续不应该只支持某一个具体算法，而应该支持多个 solver 接入。

因此可以先设计统一的 solver 接口：

```cpp
class Solver {
public:
    virtual ~Solver() = default;
    virtual Solution solve(const MAPFInstance& instance) = 0;
};
````

不同算法都可以作为不同 solver 接入，例如：

```cpp
class CBSSolver : public Solver {};
class CBSH2Solver : public Solver {};
class EECBSSolver : public Solver {};
class LNSSolver : public Solver {};
```

这样设计的好处是：后续新增算法时，只需要新增一个 solver，而不需要重写整个 SkyMAPF 框架。

可以理解为：

```text
task = 要解决什么类型的 MAPF 问题
solver = 用什么算法解决
map + scenario = 输入数据
solution = 输出路径结果
benchmark = 评估算法表现
```

---

## 5. Basic CBS 的核心流程

CBS 的基本思想可以理解为：先给每个 agent 单独规划路径，再检查路径之间是否有冲突。如果有冲突，就加入约束并重新规划，直到找到无冲突解。

Basic CBS 的流程如下：

1. 为每个 agent 生成一条初始路径；
2. 检查所有路径之间是否存在 conflict；
3. 如果不存在 conflict，则返回当前 solution；
4. 如果存在 conflict，则根据冲突生成新的 constraint；
5. 创建新的 CBS search node；
6. 对受影响的 agent 重新规划路径；
7. 将新的 node 放入 high-level search queue；
8. 重复以上过程，直到找到无冲突解或达到时间限制。

其中：

* LowLevelPlanner 负责单个 agent 的路径规划；
* HighLevelSearch 负责处理多个 agent 之间的冲突；
* Conflict 表示两个 agent 之间的碰撞；
* Constraint 表示某个 agent 在某个时间不能走到某个位置或经过某条边。

---

## 6. 分阶段迁移路线

CBSH2-RTC 包含较多高级优化，因此 SkyMAPF 不适合在第一阶段完整复刻。更合理的方式是分阶段推进。

### Stage 1: Basic CBS Framework

第一阶段先实现 basic CBS 的通用框架，包括：

* Solver 统一接口；
* MAPFInstance 输入；
* Solution 输出；
* Path 表示；
* Conflict 检测；
* Constraint 表示；
* LowLevelPlanner 单智能体路径规划；
* HighLevelSearch 多智能体冲突处理。

### Stage 2: CBS Improvement Techniques

在 basic CBS 跑通后，再逐步加入 CBSH2-RTC 中较基础的改进技术，例如：

* prioritizing conflicts；
* bypassing conflicts；
* WDG heuristic。

### Stage 3: Symmetry Reasoning

最后再考虑更复杂的 symmetry reasoning 技术，例如：

* rectangle reasoning；
* generalized rectangle reasoning；
* target reasoning；
* corridor reasoning；
* mutex propagation；
* disjoint splitting。

这些功能应作为可选模块加入，而不是一开始写死在 solver 中。

---

## 7. 与 Time-Series-Library 风格的关系

Time-Series-Library 的特点是把多个时间序列任务放在一个统一框架中，例如 forecasting、imputation、anomaly detection、classification 等任务都可以用统一的 task、model、dataset、config 方式组织。

SkyMAPF 未来也可以采用类似思路，不是把每个算法做成零散代码，而是形成统一的 MAPF 实验框架。

对应关系可以理解为：

| Time-Series-Library | SkyMAPF        |
| ------------------- | -------------- |
| task                | MAPF 任务类型      |
| model               | solver 算法      |
| dataset             | map + scenario |
| config              | 参数配置           |
| experiment          | benchmark 实验   |

例如 SkyMAPF 未来可以支持：

```bash
skymapf --task standard_mapf --solver cbs --map warehouse.map --scenario test.scen
skymapf --task standard_mapf --solver cbsh2 --map warehouse.map --scenario test.scen
skymapf --task lifelong_mapf --solver lns --map warehouse.map --scenario test.scen
```

这样 SkyMAPF 就可以从一个单一项目逐渐发展成一个统一的 MAPF 算法库。

---

## 8. 下一步计划

接下来可以分三步推进。

### 第一阶段：项目结构梳理

阅读 CBSH2-RTC 的项目结构，重点整理：

* 输入数据格式；
* solver 入口；
* conflict 表示；
* constraint 表示；
* low-level planner；
* high-level CBS search；
* 输出结果格式。

### 第二阶段：SkyMAPF 通用接口设计

结合 SkyMAPF 当前结构，设计基础数据结构和接口，包括：

* MAPFInstance；
* Solver；
* Solution；
* Path；
* Conflict；
* Constraint；
* LowLevelPlanner。

### 第三阶段：Basic CBS skeleton

先实现一个基础 CBS 框架，包括：

1. 为每个 agent 生成初始路径；
2. 检测路径之间的冲突；
3. 根据冲突生成约束；
4. 对受影响 agent 重新规划；
5. 重复直到找到无冲突解。

在 basic CBS 跑通之后，再逐步加入 CBSH2-RTC 中的高级优化，例如 conflict prioritization、bypassing、high-level heuristics 和 symmetry reasoning。

---

## 9. 小结

本次迁移的核心不是复制 CBSH2-RTC，而是参考其成熟 MAPF solver 的结构，为 SkyMAPF 搭建一个可扩展的 solver 框架。

短期目标是完成 basic CBS 的通用框架设计；中期目标是逐步加入 CBSH2-RTC 相关优化；长期目标是让 SkyMAPF 支持更多 MAPF solver、benchmark 和未来 learning-based 方法。

---

## 10. 当前迁移状态（2026-08-11）

原型提交中的 `src/core/solver/*.hpp` 没有直接作为正式接口保留。当前实现将其概念映射为：

| 原型概念 | 正式接口 |
| --- | --- |
| `Solver` | `skymapf::solver::ISolver` |
| `Solution` | `SolveResult`、`Plan`、`SolutionMetrics` |
| `Path` | `solution::AgentPath` |
| `Conflict` | `search::Conflict`、`ConflictDetector` |
| `Constraint` | `search::Constraint`、`ConstraintTable` |

Stage 1 已完成可运行闭环，包括 Space-Time A*、Basic CBS、统一验证器、
Benchmark Runner、MovingAI 输入以及跨进程 Protocol v1。

Stage 2 从可插拔冲突选择开始。当前支持保持检测顺序的 `FirstDetected`
和确定性全局最早的 `Earliest`，默认使用后者；后续 cardinal、
semi-cardinal、bypass 和 WDG 必须继续通过同一选择边界扩展，不能把
CBSH2-RTC 的实现细节写死进基础数据模型。运行记录同时拆分高层和低层
搜索节点计数，以便比较优化发生在哪一层。

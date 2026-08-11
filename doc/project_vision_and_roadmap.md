# SkyMAPF 项目设想、现状与路线图

> 更新时间：2026-08-11
> 当前开发基线：`dev` / `3dc1a09`

## 0. 阅读说明

这份文档提供的是研究背景、现有基础和可以继续探索的方向，不是一份要求逐项照做的
施工清单。后续工作可以先从运行现有系统、阅读社区资料和代表性论文开始，再根据实际
问题提出方案。

开始研究前至少应做到：

- 从最新 `dev` 完成一次构建和全部测试；
- 运行 Basic CBS 示例并读懂一次 RunRecord 的产生过程；
- 阅读 Protocol、Validator、Runner 和 CBS 现有实现；
- 从第 9 节的社区入口了解 MAPF 当前算法、benchmark 和评测方式；
- 选择一个大小合适的问题，先写一页调研与设计说明，再开始编码。

本文路线也不是固定结论。如果社区已有更好的协议、数据组织或评测方法，可以通过来源、
实验或原型提出调整。

开发仍需守住少量底线：Solver 结果必须由框架验证；canonical metrics 不能由算法
自报替代；不能通过修改 Validator 迎合错误结果；参考第三方项目时必须检查许可证，
不得直接复制不兼容代码。

## 1. 项目定位

SkyMAPF 的目标不是只实现若干 MAPF（Multi-Agent Path Finding）算法，
而是建设一个面向研究、开发和公平评测的统一基础框架。

项目希望为不同算法 family 提供共同的：

- 实例表示与数据导入；
- Solver 接口与跨语言执行协议；
- 结果表示、独立验证和标准指标；
- 本地开发、兼容性测试和算法准入流程；
- 可复现的公开评测与结果发布能力。

研究者可以使用 SkyMAPF 的 C++ SDK、Python SDK，或者只实现通用进程协议
来开发算法。算法成熟并通过兼容性测试和代码审查后，可以纳入官方算法集合。
官方评测端使用相同的公开配置、实例、验证器和指标运行所有算法，保证结果可比较。

长期形态可以概括为：

```text
统一协议
  ↓
核心数据结构、验证器、指标与 Runner
  ↓
C++ SDK / Python SDK / 外部进程适配
  ↓
研究者开发并本地验证算法
  ↓
兼容性测试与代码审查
  ↓
官方算法集合与公开 Runner
  ↓
结果存储、定期评测与 Website 展示
```

当前建设重点是前五层。公开服务器和 Website 是后续消费者，不应反过来决定
核心协议和评测语义。

## 2. 核心设计原则

### 2.1 框架拥有评测语义

Solver 只负责产生候选结果。框架负责：

- 判断结果结构是否合法；
- 验证路径、冲突和任务语义；
- 重新计算 canonical metrics；
- 记录资源消耗、运行环境和 provenance；
- 生成可发布的 RunRecord。

Solver 自报的成功状态、代价和节点数都不能替代框架验证。

### 2.2 算法接口与评测协议分离

项目同时提供三种接入方式：

1. C++ Solver 直接实现核心 `ISolver` 接口；
2. Python Solver 使用 Python SDK；
3. 任意语言程序通过 JSON stdin/stdout 协议接入。

协议是最小互操作边界，SDK 是让研究者更方便地实现协议和复用核心能力，二者不能
绑定为只有某一种语言或构建系统才能参加评测。

### 2.3 统一公共层，允许 family 专用扩展

不同 MAPF family 可以拥有各自的内部结构和优化，例如 CBS 的约束树、LNS 的邻域、
learning-based 方法的模型配置，但下列边界应保持统一：

- Problem / Instance；
- SolveRequest；
- RawSolveResult；
- ValidationResult；
- canonical metrics；
- RunRecord；
- Solver metadata 和执行限制。

family 专用统计可以作为 diagnostics 扩展，不能改变核心指标的定义。

### 2.4 可复现优先于排行榜数量

正式比较至少应固定并记录：

- 数据集和实例版本；
- Solver 名称、版本和 commit；
- 完整配置；
- 随机种子；
- 时间、节点和内存限制；
- 操作系统、CPU 等运行环境；
- 重复次数及聚合方法；
- 协议、验证器和指标版本。

在这些内容稳定前，不急于建设公开排行榜。

### 2.5 参考成熟算法，但不复制受限实现

CBSH2-RTC 等成熟项目可用于理解算法结构、论文语义和工程拆分。SkyMAPF 应在自己的
统一接口下重新实现所需能力，避免复制与 MIT 许可不兼容的源代码，也避免把框架写死
为某一个参考项目的镜像。

## 3. 目标架构

### 3.1 协议层

Protocol v1 定义四类文档：

- `skymapf.problem.v1`：可移植问题描述；
- `skymapf.solve-request.v1`：问题与运行限制；
- `skymapf.raw-solve-result.v1`：Solver 提交的非可信结果；
- `skymapf.run-record.v1`：框架验证后可发布的正式记录。

第一种语义 profile 是 `movingai_standard_v1`，规定二维矩形网格、四邻接移动、等待、
顶点冲突、反向边冲突和 stay-at-goal 行为。

### 3.2 核心层

核心层负责：

- world、task、agent、instance 和 plan 数据结构；
- MovingAI 等外部格式到统一实例的转换；
- 冲突、约束、reservation 和低层搜索抽象；
- 路径验证和标准指标；
- 单次 Benchmark Runner 和 RunRecord；
- Solver registry 与进程适配。

### 3.3 SDK 层

C++ SDK 提供可安装的头文件、库、CMake package 和协议资源。

Python SDK 分成两层：

- 纯 Python 协议层：开发外部 Solver 时不需要编译 C++；
- 可选的 `skymapf._core`：逐步开放高性能 C++ 数据结构、算法和分析能力。

### 3.4 算法层

官方算法分为：

- correctness baseline：结构简单、语义明确，用于验证框架；
- reference implementation：与论文语义对齐，可作为正式比较基线；
- optimized implementation：面向性能，但必须与 reference 结果保持语义一致；
- external/community solver：通过协议和兼容性测试参与评测。

每个算法都应声明支持的 problem profile、objective、限制、确定性和引用信息。

### 3.5 评测服务层（后续）

公开 Runner 最终负责：

- 检测算法、配置、数据集或框架版本是否发生变化；
- 只在有必要时创建新的评测任务；
- 对新增或变化的算法运行完整公共 suite；
- 对随机算法执行多次重复实验；
- 保存原始 RunRecord 和聚合结果；
- 通过稳定 API 或构建产物供 Website 展示。

“每天一次”可以作为调度频率，但不意味着每天无条件重复全部实验。

## 4. 当前已经完成的内容

### 4.1 Protocol v1-draft

已经提供规范、JSON Schema、变更记录以及合法/非法 Golden Fixtures。

当前协议能够表达：

- Standard MAPF 问题；
- sum of costs 或 makespan objective；
- wall-time、随机种子和节点限制；
- success、infeasible、timeout、unsupported、error；
- 路径、Solver metadata、开放 diagnostics；
- 验证结果、资源、provenance 和 RunRecord。

C++ 参考 codec 已实现严格字段和语义检查，并提供稳定的实例 fingerprint。

### 4.2 核心验证、指标与 Runner

当前 Validator 会检查：

- Agent 路径是否缺失、重复或多余；
- 起点和终点；
- 坐标边界和障碍；
- 等待或四邻接移动是否合法；
- vertex conflict；
- edge conflict；
- stay-at-goal 语义。

Runner 会独立计时、捕获 Solver 异常、验证候选计划、重新计算 sum of costs 和
makespan，并生成框架拥有的 RunRecord。

### 4.3 Basic CBS 纵向闭环

项目已经实现一个最朴素的、面向正确性的 Basic CBS：

- Space-Time A* 低层搜索；
- CBS constraint tree；
- vertex / edge conflict 分支；
- 对受影响 Agent 重新规划；
- 时间和高层节点限制；
- 确定性的冲突选择；
- 高层和低层 expanded/generated node diagnostics。

冲突选择目前支持：

- `FirstDetected`：保持检测器遍历顺序；
- `Earliest`：在所有冲突中选择全局最早冲突，当前默认策略。

该实现是 correctness baseline，还不是 CBSH2-RTC 的高性能复现。

### 4.4 MovingAI 数据兼容

已经能够读取标准 `.map` 和 `.scen` 文件，并用场景前 `k` 行构造 `k` Agent 的统一
实例。这使项目可以开始使用社区常见数据集，而不需要为每个算法重新编写导入逻辑。

### 4.5 外部 Solver 执行

`ExternalProcessSolver` 已经支持：

- JSON request 写入 stdin；
- 从 stdout 读取唯一 JSON result；
- 将 stderr 作为诊断日志；
- POSIX 硬 wall-time timeout；
- 子进程组终止；
- 输出大小限制；
- 非零退出、信号、非法 JSON 和身份不一致映射；
- 将成功路径送入统一 Validator 和指标流程。

已经通过 C++ Runner 启动 Python Solver 并返回 C++ 验证的端到端测试。

### 4.6 C++ SDK

当前可以安装：

- `SkyMAPF::core`；
- 公共头文件；
- `SkyMAPFConfig.cmake`；
- Protocol schemas 和规范；
- 版本信息。

独立 CMake consumer 已通过安装后 `find_package(SkyMAPF CONFIG REQUIRED)` 测试。

### 4.7 Python SDK

纯 Python SDK 已提供：

- `Problem`、`Agent`、`SolveOptions`；
- `SolveRequest` 严格解析；
- `AgentPath` 和 `RawSolveResult`；
- `@solver(...)` 应用入口；
- stdin/stdout 处理；
- unsupported 和异常状态映射；
- JSON 类型与协议约束检查。

wheel 已验证同时包含纯 Python 包和可选的 `skymapf._core` 原生扩展。

### 4.8 工程验证

当前自动测试覆盖：

- Basic CBS；
- Benchmark Runner；
- MovingAI 导入；
- Protocol codec 与 Golden Fixtures；
- 外部进程 timeout 和错误处理；
- Python SDK；
- C++ 安装后独立 consumer；
- C++ Runner → Python Solver → C++ Validator 端到端链路。

开发 CI 已包含核心测试、Python SDK 测试和 wheel 构建验证。

## 5. 当前尚未完成的内容

### 5.1 Protocol 尚未冻结

Protocol 当前是 `v1-draft`。还需要通过更多算法和数据集验证字段是否足够，再冻结 v1。
冻结后不能随意改变 required fields、状态含义和 canonical metrics。

### 5.2 还没有批量 Experiment Runner

当前 Runner 负责单实例、单次运行。尚缺少：

- suite 和实例集合；
- 多 Solver、多配置矩阵；
- 重复实验；
- 并发调度；
- 失败重试；
- 结果目录和 manifest；
- 聚合统计与算法间比较。

### 5.3 还没有兼容性测试和准入流程

虽然底层测试能力已具备，但研究者还不能运行一条统一命令生成兼容性报告。算法的
metadata、配置、支持范围、许可证和代码审查要求也未形成正式 manifest。

### 5.4 算法基线仍然较少

当前有 Prioritized Planning 和 Basic CBS，但尚缺少：

- 与论文设置充分对齐的 reference baselines；
- conflict cardinality classification；
- bypass；
- CG / DG / WDG heuristic；
- rectangle、target、corridor 等 symmetry reasoning；
- EECBS、LNS 等其他 family；
- 对第三方官方实现的兼容适配和对照实验。

### 5.5 资源隔离仍不完整

外部进程已有 wall-time 和输出限制，但正式公开评测还需要：

- memory limit；
- CPU affinity / core limit；
- 容器或沙箱；
- 峰值内存测量；
- 环境 fingerprint；
- 跨平台一致的终止语义。

### 5.6 还没有结果数据库、公开 Runner 和 Website

当前没有服务器任务队列、定时评测、结果数据库和 Website API。这些内容明确放在核心
评测链路稳定之后建设。

## 6. 下一阶段路线图

### P4：Compatibility Kit 与算法准入基础

目标：让研究者能够证明自己的 Solver 符合 SkyMAPF 协议和基础运行要求。

计划内容：

1. 定义 `solver-manifest`：名称、版本、commit、语言、入口、配置、支持 profile、许可和引用；
2. 提供 `skymapf conformance` 或等价命令；
3. 测试合法请求、非法请求、unsupported、timeout、崩溃和超大输出；
4. 对标准小实例执行 Validator 和 canonical metrics 检查；
5. 检查固定 seed 下的可重复性声明；
6. 生成机器可读兼容性报告；
7. 提供 C++ 和 Python Solver starter templates。

完成标准：一个仓库外的示例 Solver 能通过单条命令生成 conformance report，并被
C++ Runner 接受；故意破坏协议或路径时测试必须失败。

### P5：Batch Experiment Runner

目标：从单次 RunRecord 扩展到可复现的实验集合。

计划内容：

1. 定义 dataset/suite manifest；
2. 定义 Solver × config × instance × seed 的 run matrix；
3. 建立稳定 run id 和去重键；
4. 支持重复运行、失败恢复和有限并发；
5. 将每次运行保存为不可变 RunRecord；
6. 计算均值、中位数、标准差、成功率和置信区间；
7. 输出 JSON/CSV summary 和算法对比表。

完成标准：同一 manifest 重跑不会无条件重复已有结果；新增一个算法时能只补充必要
run，并重新生成全体算法的可比较 summary。

### P6：Protocol v1 冻结

目标：在至少两个语言、多个算法 family 和批量实验中验证协议后发布稳定 v1。

计划内容：

1. 审查 problem、request、raw result 和 run record；
2. 明确 objective、timeout、infeasible 和 invalid solution 的边界；
3. 固定 diagnostics 与 canonical metrics 的权威性；
4. 完善 schema conformance tests；
5. 发布兼容性政策和 v1 → v2 演进规则。

完成标准：Golden Fixtures、C++ codec、Python codec 和 JSON Schema 对相同输入有一致
结论，且批量 Runner 只依赖冻结后的公共字段。

### P7：Reference Algorithms 与 CBS family 演进

目标：建立能够支撑论文对比的算法基线集合。

建议顺序：

1. 完善 Basic CBS 的测试实例和论文语义对照；
2. cardinal / semi-cardinal / non-cardinal conflict classification；
3. conflict prioritization；
4. bypass；
5. CG、DG、WDG heuristic；
6. symmetry reasoning 作为可选模块；
7. 引入 EECBS 或 LNS 作为第二种重要 family；
8. 与公开官方实现做同实例、同限制的交叉检查。

每项优化必须提供开关、独立测试和消融配置，保证可以比较 `basic` 与 `enhanced`，而
不是不断修改同一个无法复现实验语义的 Solver。

### P8：公开 Runner 基础设施

目标：在公开配置、公开源码的服务器环境中定期产生权威结果。

计划内容：

1. 容器化 Solver 与 Runner；
2. 固定 CPU、memory、timeout 和环境镜像；
3. 建立算法/配置/数据/框架版本变更检测；
4. 有变化时创建增量任务，无变化时跳过；
5. 多次运行随机算法并保存原始样本；
6. 建立结果签名、审计日志和失败重跑机制；
7. 提供 Website 可消费的稳定产物或 API。

完成标准：从公开 commit 和 manifest 可以重新生成任意排行榜条目，服务器结果可追溯
到具体 RunRecord 和执行环境。

### P9：Website 集成

目标：展示算法、数据集、配置、单次结果、聚合结果和历史变化。

Website 不重新计算指标，只读取 Runner 发布的权威数据。展示层至少应允许用户查看：

- 比较使用了哪些实例和限制；
- 每个数值来自哪些 runs；
- Solver 版本、配置和 commit；
- 验证状态和失败原因；
- 均值之外的分布和重复次数；
- 历史版本之间的变化。

## 7. 建议的近期执行顺序

接下来建议严格按以下顺序推进：

```text
1. Solver Manifest
2. Compatibility / Conformance Kit
3. C++ 与 Python starter solver
4. Dataset / Suite Manifest
5. Batch Experiment Runner
6. 结果聚合与对比报告
7. Protocol v1 冻结
8. 更多 reference algorithms
9. 公开 Runner
10. Website
```

近期不建议直接开始服务器或排行榜。先通过 Compatibility Kit 和 Batch Runner 固化
“谁可以运行、运行什么、如何验证、如何比较、如何复现”，后面的自动化和 Website
才会建立在稳定数据之上。

## 8. 项目成功标准

当以下场景能够成立时，SkyMAPF 才真正从算法仓库成为 benchmark 基础设施：

1. 第三方研究者不修改核心仓库即可开发 C++ 或 Python Solver；
2. 同一个 Solver 可在本地和官方 Runner 使用相同协议运行；
3. 非法结果无法进入正式指标；
4. 不同算法共享完全相同的数据、限制、验证和指标；
5. 每个公开结果都能追溯到实例、配置、seed、commit、环境和原始 RunRecord；
6. 新增算法只产生必要的增量评测，不浪费已有计算；
7. Website 只是权威评测结果的展示层，而不是另一套评测实现；
8. 算法 family 可以持续扩展，但公共协议和公平比较原则保持稳定。

## 9. 研究入口与可选方向

### 9.1 先了解社区在做什么

下面的入口不是“官方标准答案”，而是用来观察社区如何定义问题、发布代码、组织
benchmark 和展示进展。调研时应比较它们的目标与 SkyMAPF 的差异。

- [MAPF.info](https://mapf.info/)：社区资料入口，可查看
  [软件实现](https://mapf.info/index.php/Main/Software)、
  [benchmark](https://mapf.info/index.php/Main/Benchmarks)、
  [论文目录](https://mapf.info/index.php/Main/Publications) 和竞赛信息。
- [Moving AI MAPF Benchmarks](https://movingai.com/benchmarks/mapf.html)：当前最常用的
  网格 MAPF 地图与 scenario 来源，也是 SkyMAPF 已开始兼容的数据格式。
- [MAPF Tracker](https://pathfinding.ai/projects/tracker/)：关注如何保存历史实验数据、
  比较不同 Solver，以及如何描述整个领域的性能进展。这与 SkyMAPF 的长期 benchmark
  目标最接近，值得重点分析。
- [Foundations of Multi-Agent Path Finding](https://arcs-group.github.io/mapf/)：集中展示
  CBSH2-RTC、EECBS、PBS、MAPF-LNS2、symmetry reasoning、heuristics 和 learning-guided
  planning 等算法方向，并链接论文与作者实现。
- [MAPF Arena](https://mapf.dev/)：一个较新的浏览器评测与排行榜尝试，可研究 WASM
  Solver 上传、可视化和在线评测的优缺点，但不应直接假设其接口适合 SkyMAPF。
- [League of Robot Runners](https://www.leagueofrobotrunners.org/)：关注大规模、在线和
  lifelong MAPF 的竞赛环境，可帮助判断未来是否需要超出 Standard MAPF 的 profile。

建议首先阅读 *Multi-Agent Pathfinding: Definitions, Variants, and Benchmarks*，明确
MAPF 的不同冲突定义、目标函数和 benchmark 假设。之后再选择一个算法论文，并把论文、
作者代码、MAPF.info 描述和实际实验配置相互核对。

### 9.2 可以研究的问题

以下问题都有价值，但不规定具体实现方式。可以选择其中一个，也可以在调研后提出新的
问题：

- 现有 Protocol v1 是否足以描述社区常用 Solver？哪些差异属于配置，哪些需要新的
  problem profile？
- MAPF Tracker 如何定义一次公平比较？SkyMAPF 应借鉴什么，又应避免什么？
- 如何设计最小 Solver manifest，使算法可以被本地 Runner 和未来公开 Runner 识别？
- Moving AI benchmark 的 instance selection、Agent 数量增长和场景分类应怎样进入
  suite，而不会让实验者任意挑选有利实例？
- Basic CBS 与论文中的 CBS 在语义和统计上是否一致？可以选择一种 CBS 改进，先复现
  论文动机和小规模现象，再讨论是否纳入框架。
- 外部官方实现能否通过 adapter 接入，而不修改其算法主体？兼容成本来自数据、进程、
  配置还是结果语义？
- 随机算法应该重复多少次、报告哪些统计量，才能避免只比较单个均值？
- 公共 Runner 如何识别“结果已经存在”，只对新增算法、配置或数据执行增量评测？
- Python SDK 还缺哪些真正影响算法开发的能力，哪些绑定只是为了 API 数量而没有价值？

### 9.3 适当大小的开发成果

一次阶段工作可以是：

- 一份有来源、有对照实验的调研结论，加一个验证关键假设的原型；
- 一个能够接入真实外部 Solver 的 adapter；
- 一项 CBS 改进及其开关、测试和消融结果；
- 一个小型 benchmark suite 定义和可重复运行脚本；
- 一个协议或 SDK 缺口的端到端修复；
- 对现有评测网站的数据模型与 SkyMAPF RunRecord 的实证比较。

不鼓励只增加空类、复制第三方目录、只写无法执行的计划，或者一次声称实现完整公开
平台。研究结论应落到代码、实验、fixture 或可审查的协议修改中的至少一种。

### 9.4 开展研究时建议回答的问题

具体方案可以自行决定，但最好能够回答：

1. 研究了什么问题，为什么值得做？
2. 参考了哪些论文、网站或作者实现？
3. 社区已有方案的假设是什么，与 SkyMAPF 是否一致？
4. 自己做了什么选择，有哪些没有解决？
5. 用什么实例和指标验证？结果能否复现？
6. 是否影响 Protocol、Validator、公平性或已有算法？

代码仍需通过现有测试；新增行为应有能够失败的测试，而不只是成功截图。一个 PR 保持
一个主要研究问题即可，具体分支名、类名和内部结构由实现方案决定。

## 10. 可以考虑的几个小方向

为了避免研究范围过大，第一次开发可以从下面选一个小切口。这里仅限定问题规模，不预设
答案和具体类设计。

### A. 对照一个真实 Solver

从 MAPF.info 选择一个有论文和作者代码的 Solver，回答：它的输入、输出、配置、状态和
指标能否映射到 SkyMAPF Protocol v1？选择 2—3 个 Moving AI 小实例做实际运行。如果
不能直接接入，实现最薄的一层 adapter，并记录所有语义差异。

这个方向可以帮助验证协议是否真的“通用”，比继续凭想象增加字段更有价值。

### B. 复现一项 CBS 改进的动机

从 conflict prioritization、bypass、CG/DG/WDG 或一种 symmetry reasoning 中选择一项。
先找到能体现问题的小实例，比较当前 Basic CBS 的 constraint tree 和节点统计，再提出
最小实现。第一阶段不要求达到作者代码性能，但要证明现象、语义和实验可重复。

### C. 研究现有 MAPF 进展网站

重点比较 MAPF Tracker、MAPF.info、Moving AI 和 MAPF Arena：

- 它们保存的是论文结果、用户上传结果还是平台重跑结果？
- 是否公开实例、硬件、限制、版本、seed 和原始 runs？
- 如何处理不同年份硬件与代码变化？
- 排行榜数值能否独立复现？
- SkyMAPF 的 RunRecord 和未来公开 Runner 应借鉴什么？

成果可以是一份数据模型对照，加一个把 SkyMAPF RunRecord 转成展示数据的小原型。

### D. 做一个最小可复现实验集合

从 Moving AI 选择少量不同结构地图，明确 scenario、Agent 数量、objective、timeout 和
seed，比较 Prioritized Planning 与 Basic CBS。重点不是跑很多数据，而是研究 suite 的
选择是否中立、重复运行如何组织、哪些原始信息必须保存。

### E. 改善研究者接入体验

邀请一个没有参与框架开发的人尝试编写 Python 或 C++ Solver，记录实际遇到的障碍。
从其中选择一个真实问题修复，例如 SDK 缺少的数据、错误信息不清楚、示例无法独立构建
或外部进程诊断不足。不要在没有使用证据前大规模增加绑定。

### F. 其他问题

也可以不选上述方向。新的问题大致说明社区现状、SkyMAPF 当前缺口、准备如何验证以及
一个合适范围内能够交付什么即可。

## 11. 相关文档

- [Protocol v1 规范](../protocol/v1/spec.md)
- [Benchmark 语义](benchmark_semantics_v1.md)
- [外部 Solver 协议](external_solver_protocol_v1.md)
- [Python SDK](python_sdk.md)
- [CBSH2-RTC 迁移计划](solver_migration_plan_cbsh2.md)

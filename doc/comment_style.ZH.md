# SkyMAPF 注释规范

## 1. 文档目的

本文档用于规定 SkyMAPF 项目中的注释与文档书写方式。

制定这份规范的目标是：
- 让公开 API 更容易理解；
- 保持生成文档风格统一；
- 减少含糊、重复或无信息量的注释；
- 将接口文档与设计讨论分离。

SkyMAPF 的定位是一个可复用的库，因此注释的首要任务是帮助使用者理解接口语义、使用约束和行为边界。

---

## 2. 语言规范

### 2.1 对外文档
所有对外公开的代码注释都应使用英文。

包括但不限于：
- `include/` 下公开头文件中的注释；
- 导出类、函数、枚举、结构体的 API 文档；
- 预期会被文档工具提取和展示的注释。

### 2.2 内部说明
以下内容可以使用中文：
- 临时开发备注；
- `docs/` 中的设计草稿；
- 开发阶段的简短内部注释。

但稳定的公开 API 注释中不应出现中文。

### 2.3 避免混用语言
同一个正式的 API 注释块中，不要中英文混写。

---

## 3. 文档层次划分

SkyMAPF 中的文档分为三个层次：

### 3.1 API 注释
用于公开类、函数、枚举和重要类型。  
这类注释主要回答“这个接口是什么、怎么用”。

### 3.2 实现注释
用于 `.cpp` 文件或私有实现细节。  
这类注释主要回答“为什么这样写”，而不是把代码再翻译一遍。

### 3.3 设计文档
放在 `docs/` 中。  
这类文档用于说明更高层次的内容，例如架构设计、不变量、数据格式、坐标系统、场景语义和设计动机。

---

## 4. 公开 API 注释要求

所有公开 API 注释都应使用兼容 Doxygen 的风格。

推荐使用块注释形式：

```cpp
/**
 * @brief 简要说明。
 *
 * 如有必要，可补充更长的说明。
 *
 * @param ...
 * @return ...
 * @throws ...
 * @pre ...
 * @post ...
 */
```

对于非常简单的声明，可以使用单行形式：

```cpp
/// Returns the number of agents.
std::size_t agent_count() const noexcept;
```

### 4.1 最低要求
`include/` 中的每一个公开类型或函数，至少应具备：
- 一句简要说明；
- 对非显然参数的解释；
- 对返回值含义的说明（若返回值语义不明显）。

### 4.2 推荐使用的标签
在适用时，建议使用以下标签：
- `@brief`
- `@param`
- `@return`
- `@throws`
- `@pre`
- `@post`
- `@note`
- `@warning`

### 4.3 摘要行要求
`@brief` 应满足：
- 简短；
- 直接；
- 语义清晰；
- 尽量体现动作或职责。

好的例子：
- `@brief Loads a world from a JSON file.`
- `@brief Returns whether the cell is blocked.`

不好的例子：
- `@brief World function.`
- `@brief Check.`
- `@brief Getter for world.`

---

## 5. 公开注释需要回答的问题

公开注释在必要时应尽量说明以下内容：
- 这个类型或函数表示什么；
- 输入是什么；
- 返回什么；
- 需要满足哪些约束；
- 可能出现什么错误；
- 是否存在所有权、引用或生命周期方面的假设。

对于库代码而言，语义说明比实现细节更重要。

---

## 6. 按代码元素划分的规则

### 6.1 文件

公开头文件应在开头添加简短的文件注释。

示例：

```cpp
/**
 * @file world.hpp
 * @brief Defines the immutable world representation used by SkyMAPF.
 */
```

不建议使用很长的装饰性横幅注释。

### 6.2 命名空间

只有在命名空间具有明确语义分组意义时，才需要添加说明。

示例：

```cpp
/**
 * @brief Core data structures and algorithms for SkyMAPF.
 */
namespace skymapf {
}
```

### 6.3 类与结构体

每个公开类或结构体都应说明：
- 它表示什么；
- 它是可变的还是不可变的；
- 是否存在重要不变量。

示例：

```cpp
/**
 * @brief Immutable grid-based world description.
 *
 * A world stores dimensions, blocked cells, and metadata required
 * to validate agent motions and construct scenarios.
 */
class World {
public:
    ...
};
```

### 6.4 函数

函数注释应描述行为，而不是重复函数名。

示例：

```cpp
/**
 * @brief Returns whether the given position lies within world bounds.
 *
 * @param pos The position to test.
 * @return True if the position is valid in this world; false otherwise.
 */
bool contains(const Position& pos) const;
```

### 6.5 构造函数

当构造函数会建立不变量或验证输入时，应补充说明。

示例：

```cpp
/**
 * @brief Constructs a world with the specified dimensions.
 *
 * @param width Number of cells along the x-axis.
 * @param height Number of cells along the y-axis.
 * @throws std::invalid_argument If either dimension is zero.
 */
World(int width, int height);
```

### 6.6 枚举

每个公开枚举都应说明：
- 这个枚举在分类什么；
- 每个枚举值分别表示什么。

示例：

```cpp
/// Describes the occupancy state of a cell.
enum class CellState {
    Free,      ///< The cell is traversable.
    Blocked,   ///< The cell is occupied by an obstacle.
    Unknown    ///< The state is unspecified.
};
```

### 6.7 类型别名

只有当类型别名具有明确的领域语义时，才需要注释。

好的例子：
```cpp
/// Unique identifier for an agent.
using AgentId = std::uint32_t;
```

不好的例子：
```cpp
// alias for vector
using Vec = std::vector<int>;
```

---

## 7. 实现注释规则

实现注释应说明：
- 为什么需要这一步；
- 这里处理了什么边界情况；
- 维护了什么不变量；
- 为什么采用了不那么直观的算法或实现方式。

不要写那种只是把代码翻译成英文的注释。

不好的例子：
```cpp
// increment i
++i;
```

不好的例子：
```cpp
// if blocked return false
if (blocked) {
    return false;
}
```

好的例子：
```cpp
// Layer 0 is reserved for the ground plane and is not traversable.
++layer;
```

好的例子：
```cpp
// We validate bounds before obstacle insertion so the world remains
// internally consistent even if later checks throw.
```

---

## 8. 不需要注释的内容

以下内容通常不应添加注释：
- 显而易见的 getter/setter；
- 很简单的控制流程；
- 已经非常清楚的标准库用法；
- 与代码表达完全重复的说明。

应避免的例子：
- `// constructor`
- `// default constructor`
- `// set value`
- `// loop through vector`

注释应增加信息量，而不是制造噪音。

---

## 9. SkyMAPF 需要特别说明的语义

以下概念在相关代码或文档中应明确说明：

### 9.1 坐标语义
需要明确：
- 坐标顺序，例如 `(x, y, z)`；
- 二维是否视为三维的特殊情况；
- 原点约定；
- 各坐标轴的含义。

### 9.2 时间语义
需要明确：
- 一个动作是否恰好消耗一个 tick；
- 是否允许 wait；
- 是否允许对角移动；
- 冲突如何定义。

### 9.3 世界不变量
需要明确：
- world 在构造后是否可变；
- obstacle 是否允许重叠；
- 阻塞单元是否在加载时就验证。

### 9.4 所有权与生命周期
需要明确：
- 函数返回的是值、引用、指针还是视图；
- 返回引用会在何时失效；
- 被调用方是否会保存外部数据的引用。

### 9.5 错误处理
需要明确：
- 非法输入是抛异常还是其他方式处理；
- 解析错误是否可恢复；
- 校验是在早期完成还是延后进行。

---

## 10. 格式约定

### 10.1 语气
使用中性、准确、技术性的表达。

### 10.2 句式
公开 API 注释尽量使用完整句子。  
对于很短的枚举值或成员说明，可以使用短语。

### 10.3 标点
使用标准标点。  
完整句子建议以句号结尾。

### 10.4 行宽
注释行宽应适中，便于在头文件中阅读。  
避免特别长的一整行说明。

---

## 11. 示例模板

### 11.1 公开类

```cpp
/**
 * @brief Represents a fixed world together with a set of agent tasks.
 *
 * A scenario binds a world and one or more task definitions under a common
 * movement and validation model.
 */
class Scenario {
public:
    ...
};
```

### 11.2 公开函数

```cpp
/**
 * @brief Loads a scenario from a JSON file.
 *
 * @param path Path to the input file.
 * @return Parsed scenario object.
 * @throws std::runtime_error If the file cannot be read or the content is invalid.
 *
 * @pre The file must conform to the SkyMAPF scenario schema.
 */
Scenario load_scenario(const std::filesystem::path& path);
```

### 11.3 返回引用的 getter

```cpp
/**
 * @brief Returns the world associated with this scenario.
 *
 * The returned reference remains valid for the lifetime of the scenario object.
 */
const World& world() const noexcept;
```

### 11.4 枚举

```cpp
/// Defines how a cell participates in path planning.
enum class CellKind {
    Free,       ///< Traversable cell.
    Blocked,    ///< Occupied by an obstacle.
    Reserved    ///< Temporarily unavailable due to planning constraints.
};
```

---

## 12. 代码评审检查表

在合并代码前，建议检查以下内容：

- 所有公开头文件都使用英文注释；
- `include/` 中所有公开类型与函数都有文档说明；
- 注释描述的是语义，而不是语法；
- Doxygen 标签使用一致；
- 非显然参数和返回值都已解释；
- 重要不变量已经说明；
- 冗余或显然的注释已经删除。

---

## 13. 最终原则

好的注释应当让库更容易被正确使用。

对于 SkyMAPF：
- 公开注释服务于使用者；
- 实现注释服务于维护者；
- 设计文档服务于解释系统本身。

当拿不准时，宁可少写一些，也要保证信息密度足够高。

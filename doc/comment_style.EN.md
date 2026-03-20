# SkyMAPF Comment Style Guide

## 1. Purpose

This document defines the comment and documentation style used in SkyMAPF.

The goals are:

- keep public APIs easy to understand;
- make generated documentation consistent;
- reduce ambiguous or redundant comments;
- separate API documentation from design discussion.

SkyMAPF is intended to be a reusable library. Therefore, comments should
primarily help users understand interfaces, constraints, and semantic meaning.

---

## 2. Language Policy

### 2.1 Public documentation

All public-facing code comments must be written in English.

This includes:

- comments in public headers under `include/`;
- API documentation for exported classes, functions, enums, and structs;
- comments intended to appear in generated documentation.

### 2.2 Internal notes

Chinese may be used in:

- temporary development notes;
- design drafts in `docs/` or personal notes;
- short internal comments during active implementation.

However, Chinese should not appear in stable public API comments.

### 2.3 Mixed-language comments

Do not mix Chinese and English in the same formal API comment block.

---

## 3. Documentation Levels

SkyMAPF uses three levels of documentation:

### 3.1 API comments

Used for public classes, functions, enums, and important types.
These comments should explain what an interface does and how to use it.

### 3.2 Implementation comments

Used inside `.cpp` files or private implementation sections.
These comments should explain why something is done, not restate the code.

### 3.3 Design documents

Used in `docs/`.
These documents explain higher-level architecture, invariants, data formats,
coordinate systems, scenario semantics, and design rationale.

---

## 4. Required Style for Public API Comments

All public API comments should use Doxygen-compatible style.

Preferred block form:

```cpp
/**
 * @brief Brief summary.
 *
 * Optional longer explanation.
 *
 * @param ...
 * @return ...
 * @throws ...
 * @pre ...
 * @post ...
 */
```

Single-line form is acceptable only for very simple declarations:

```cpp
/// Returns the number of agents.
std::size_t agent_count() const noexcept;
```

### 4.1 Minimum requirement

Every public type or function in `include/` must have at least:

- a short summary;
- parameter descriptions when parameters are nontrivial;
- return value description when the meaning is not obvious.

### 4.2 Recommended tags

Use these tags when applicable:

- `@brief`
- `@param`
- `@return`
- `@throws`
- `@pre`
- `@post`
- `@note`
- `@warning`

### 4.3 Summary line

The `@brief` line should be:

- short;
- direct;
- action-oriented;
- semantically meaningful.

Good:

- `@brief Loads a world from a JSON file.`
- `@brief Returns whether the cell is blocked.`

Bad:

- `@brief World function.`
- `@brief Check.`
- `@brief Getter for world.`

---

## 5. What Public Comments Must Explain

Public comments should answer the following questions whenever relevant:

- What does this type or function represent?
- What are the inputs?
- What does it return?
- What constraints must hold?
- What can go wrong?
- What ownership or lifetime assumptions exist?

For library code, semantic meaning is more important than implementation detail.

---

## 6. Rules by Code Element

### 6.1 Files

Public header files should begin with a short file comment.

Example:

```cpp
/**
 * @file world.hpp
 * @brief Defines the immutable world representation used by SkyMAPF.
 */
```

Avoid large banner comments with decorative symbols.

### 6.2 Namespaces

Document namespaces only when they carry meaningful semantic grouping.

Example:

```cpp
/**
 * @brief Core data structures and algorithms for SkyMAPF.
 */
namespace skymapf {
}
```

### 6.3 Classes and structs

Every public class or struct must document:

- what it represents;
- whether it is mutable or immutable;
- any important invariants.

Example:

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

### 6.4 Functions

Function comments should describe behavior, not repeat the function name.

Example:

```cpp
/**
 * @brief Returns whether the given position lies within world bounds.
 *
 * @param pos The position to test.
 * @return True if the position is valid in this world; false otherwise.
 */
bool contains(const Position& pos) const;
```

### 6.5 Constructors

Document constructors when they establish invariants or validate input.

Example:

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

### 6.6 Enums

Every public enum should describe:

- what the enum classifies;
- what each enumerator means.

Example:

```cpp
/// Describes the occupancy state of a cell.
enum class CellState {
    Free,      ///< The cell is traversable.
    Blocked,   ///< The cell is occupied by an obstacle.
    Unknown    ///< The state is unspecified.
};
```

### 6.7 Type aliases

Document type aliases only when they carry domain meaning.

Good:

```cpp
/// Unique identifier for an agent.
using AgentId = std::uint32_t;
```

Bad:

```cpp
// alias for vector
using Vec = std::vector<int>;
```

---

## 7. Rules for Implementation Comments

Implementation comments should explain:

- why a step exists;
- which edge case is being handled;
- what invariant is being preserved;
- why a less obvious algorithmic choice was made.

Do not write comments that simply translate code into English.

Bad:

```cpp
// increment i
++i;
```

Bad:

```cpp
// if blocked return false
if (blocked) {
    return false;
}
```

Good:

```cpp
// Layer 0 is reserved for the ground plane and is not traversable.
++layer;
```

Good:

```cpp
// We validate bounds before obstacle insertion so the world remains
// internally consistent even if later checks throw.
```

---

## 8. What Should Not Be Commented

Do not comment:

- obvious getters and setters;
- trivial control flow;
- standard library usage that is already clear;
- comments that repeat the exact meaning of the code.

Examples to avoid:

- `// constructor`
- `// default constructor`
- `// set value`
- `// loop through vector`

Comments should add information, not noise.

---

## 9. Invariants and Semantics Specific to SkyMAPF

The following concepts must be documented clearly wherever relevant:

### 9.1 Coordinate semantics

Always clarify:

- coordinate order, such as `(x, y, z)`;
- whether 2D is treated as a special case of 3D;
- origin convention;
- axis meaning.

### 9.2 Time semantics

Always clarify:

- whether one action consumes exactly one tick;
- whether wait actions are allowed;
- whether diagonal movement is allowed;
- how collisions are defined.

### 9.3 World invariants

Always clarify:

- whether worlds are mutable after construction;
- whether obstacles may overlap;
- whether blocked cells are validated at load time.

### 9.4 Ownership and lifetime

Always clarify:

- whether functions return by value, reference, pointer, or view;
- when returned references become invalid;
- whether the callee stores references to external data.

### 9.5 Error handling

Always clarify:

- whether invalid input throws exceptions;
- whether parsing errors are recoverable;
- whether validation is eager or deferred.

---

## 10. Formatting Conventions

### 10.1 Tone

Use neutral, precise, technical language.

### 10.2 Sentence style

Use full sentences for public API comments.
Fragments are acceptable only for very short enum/member descriptions.

### 10.3 Punctuation

Use standard punctuation.
End full-sentence descriptions with periods.

### 10.4 Line length

Prefer moderate line lengths for readability in headers.
Avoid extremely long comment lines.

---

## 11. Example Templates

### 11.1 Public class

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

### 11.2 Public function

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

### 11.3 Getter returning a reference

```cpp
/**
 * @brief Returns the world associated with this scenario.
 *
 * The returned reference remains valid for the lifetime of the scenario object.
 */
const World& world() const noexcept;
```

### 11.4 Enum

```cpp
/// Defines how a cell participates in path planning.
enum class CellKind {
    Free,       ///< Traversable cell.
    Blocked,    ///< Occupied by an obstacle.
    Reserved    ///< Temporarily unavailable due to planning constraints.
};
```

---

## 12. Review Checklist

Before merging code, check the following:

- All public headers use English comments.
- All public types and functions in `include/` are documented.
- Comments describe semantics, not syntax.
- Doxygen tags are used consistently.
- Nontrivial parameters and return values are explained.
- Important invariants are documented.
- Redundant or obvious comments have been removed.

---

## 13. Final Principle

Good comments make the library easier to use correctly.

For SkyMAPF:

- public comments should help users;
- internal comments should help maintainers;
- design documents should help explain the system.

When in doubt, prefer fewer comments with higher information density.
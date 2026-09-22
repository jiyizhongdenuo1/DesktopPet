# 模板：作用与适用场景

## 一、作用

模板把「类型」和「编译期常量」变成参数，由编译器在使用点生成具体代码。它值钱的地方只有四件事：

| 价值 | 说明 | 缺了它的后果 |
|---|---|---|
| **零开销抽象** | 编译期实例化，可内联，无虚表、无间接调用 | 为了通用性付出运行时开销 |
| **类型安全** | 编译期检查，替代 `void*` / 宏 | 类型错误推迟到运行时 |
| **代码复用** | 一套逻辑，多种类型 | 每种类型抄一遍，改一处要改 N 处 |
| **编译期计算** | 把工作从运行时挪到编译期 | 运行时的分支与计算开销 |

判断一个模板是否值得存在，就看它是否提供了上面**至少一项**，并且**实例化次数 ≥ 2**。都不满足，它只是一层噪音。

## 二、适用场景

满足以下任一条，模板才是合理的：

### 1. 类型是「数据结构的一部分」

容器类：装什么由类型参数决定，不同 `T` 就是不同容器。

### 2. 同一套算法作用于多种类型

算法 / 工具函数：逻辑与类型无关，只需类型作参数。

### 3. 类型或容量编译期确定，且是热路径

需要内联、需要避免间接调用时，模板是唯一能做到零开销的手段。

### 4. 需要编译期计算或校验

`constexpr`、`if constexpr`、`static_assert`、类型萃取。

**共同前提：会有 ≥ 2 种实例化。** 复用次数不足时，模板只有缺点。

## 三、不适用场景

| 场景 | 为什么不需要模板 |
|---|---|
| 只有一种类型 | 没有复用，只承担缺点 |
| 需要运行时替换实现 | 那是虚接口的职责 |
| 需要一个变量装多种类型 | 需要类型擦除 / 虚接口 |
| 需要把实现藏进 `.cpp` | 模板必须头文件可见 |
| 编译时间已经很紧张 | 每个实例化都要完整编译一遍 |
| 类型需要出现在跨模块公开 API 上 | 模板参数会泄漏进签名，ABI 不友好 |

## 四、正反例子

### 正面例子

#### 例 1：环形缓存容器（类型和容量都是数据结构的一部分）

**先看语义**：固定容量，写满后覆盖最旧的数据，写入 O(1)，不需要搬移元素。

```
容量 5，依次写入 A B C D E F G

初始    [_][_][_][_][_]     writePos=0
写 A    [A][_][_][_][_]     writePos=1
写 B    [A][B][_][_][_]     writePos=2
写 E    [A][B][C][D][E]     writePos=0    ← 到尾巴了，回绕到头部
写 F    [F][B][C][D][E]     writePos=1    ← 覆盖最旧的 A
写 G    [F][G][C][D][E]     writePos=2    ← 覆盖 B
```

**完整实现**：

```cpp
template <typename T, std::size_t N>
class TRingBuffer
{
public:
    static_assert(N > 0, "capacity must be positive");

    void Push(const T& v)
    {
        m_arr[m_s32WritePos] = v;
        m_s32WritePos = (m_s32WritePos + 1) % static_cast<INT32>(N);   // 回绕
        if (m_s32Count < static_cast<INT32>(N))
        {
            ++m_s32Count;                                              // 计数封顶
        }
    }

    template <typename Fn>
    void ForEach(Fn&& fn) const
    {
        // 写满时数据顺序是 [writePos, N) + [0, writePos)，不能简单从头遍历
        const INT32 s32Start = (m_s32Count < static_cast<INT32>(N)) ? 0 : m_s32WritePos;

        for (INT32 i = 0; i < m_s32Count; ++i)
        {
            if (!fn(m_arr[(s32Start + i) % static_cast<INT32>(N)]))
            {
                break;
            }
        }
    }

    INT32 Count() const { return m_s32Count; }

    void Clear() { m_s32WritePos = 0; m_s32Count = 0; }

private:
    std::array<T, N> m_arr;
    INT32            m_s32WritePos = 0;
    INT32            m_s32Count    = 0;
};
```

**两种实例化**：

```cpp
TRingBuffer<ST_NOTE_DATA, 500>  noteBuf;    // 实例化 1：笔记缓存
TRingBuffer<SCategoryInfo, 100> cateBuf;    // 实例化 2：分类缓存 —— 复用从这里开始
```

**为什么合理**：

| 判据 | 这个例子 |
|---|---|
| 类型是数据结构的一部分 | ✅ `T` 就是"装什么" |
| 容量是数据结构的一部分 | ✅ `N` 决定回绕点和内存占用 |
| 实例化 ≥ 2 次 | ✅ 笔记 + 分类 |
| 编译期确定 | ✅ `std::array` 可栈上分配，无动态内存 |
| 逻辑与类型无关 | ✅ 回绕、封顶、遍历顺序都不依赖 `T` |

**注意锁不在模板里**：并发控制留在外层（缓存类自己持有读写锁），模板只管存。这样它才能被复用到不需要加锁的场景，职责也更单一。

#### 例 2：算法作用于多种类型

```cpp
template <typename T>
constexpr const T& Clamp(const T& v, const T& lo, const T& hi)
{
    return (v < lo) ? lo : ((hi < v) ? hi : v);
}
```

**为什么合理**：逻辑与类型完全无关，可被无数类型复用。

#### 例 3：热循环里避免间接调用

```cpp
template <typename Fn>
void ForEach(Fn&& fn) const          // 回调类型编译期确定，可内联
{
    for (INT32 i = 0; i < m_s32Count; ++i)
    {
        if (!fn(m_arr[i]))
        {
            break;
        }
    }
}
```

对比 `std::function` 版本：一次类型擦除 + 可能的堆分配。

**为什么合理**：回调在热循环里每条数据都调一次，零开销有实际价值。

#### 例 4：编译期校验

```cpp
template <typename T, std::size_t N>
class TRingBuffer
{
    static_assert(N > 0, "capacity must be positive");
};
```

**为什么合理**：把错误挡在编译期，且不产生任何运行时开销。

### 反面例子

#### 例 1：只用一次

```cpp
template <typename T>
class TNoteCache { /* ... */ };

TNoteCache<ST_NOTE_DATA> g_noteCache;   // 唯一实例化
```

**为什么不合理**：复用收益为 0，而缺点全都要承担——实现暴露在头文件、编译变慢、报错难读、改一行全量重编译。

#### 例 2：把接口模板化

```cpp
template <typename T>
class IDataProvider
{
public:
    virtual void ForEach(const std::function<bool(const T&)>& fn) const = 0;
};

class CNoteBusiness
{
    std::shared_ptr<IDataProvider<ST_NOTE_DATA>> m_pProvider;   // 类型被迫写死
};
```

**为什么不合理**：每个 `T` 都是不同类型，`shared_ptr<Base>` 装不了；类型泄漏进业务类；无法运行时注入实现。

#### 例 3：模板基类 + 虚函数

```cpp
template <typename T, std::size_t N>
class TCacheBase
{
public:
    virtual ~TCacheBase() = default;
    virtual void Push(const T& v) = 0;
};

class CNoteDataCache : public TCacheBase<ST_NOTE_DATA, 500> { /* ... */ };
```

**为什么不合理**：每个实例化都是不同的基类，无法统一持有；虚函数开销也白付了。这是最糟的组合。

#### 例 4：把实现拆到 `.cpp`

```cpp
// TCache.h
template <typename T>
class TCache
{
public:
    void Push(const T& v);        // 只有声明
};
```

```cpp
// TCache.cpp
template <typename T>
void TCache<T>::Push(const T& v) { /* ... */ }   // T 未知，不生成任何代码
```

```cpp
// main.cpp
TCache<NoteData> c;
c.Push(x);      // ❌ undefined reference to TCache<NoteData>::Push
```

**为什么不合理**：实例化发生在使用方，使用方必须看到定义。想藏实现就选错了工具。

#### 例 5：把运行时策略塞成模板参数

```cpp
template <typename T, typename EvictPolicy>
class TCache { /* ... */ };

TCache<ST_NOTE_DATA, LRU>  a;
TCache<ST_NOTE_DATA, FIFO> b;    // 换个策略就是另一个类型，配置切不了
```

**为什么不合理**：策略需要在运行时按配置切换 → 应该用虚接口，不是模板参数。

### 正反对照

| 维度 | 正面 | 反面 |
|---|---|---|
| 实例化次数 | ≥ 2 | 1 |
| 类型的角色 | 数据结构的一部分 | 实现细节泄漏到接口 |
| 绑定时机 | 编译期确定 | 需要运行时选择 |
| 实现位置 | 头文件可见（可接受） | 想藏进 `.cpp` |
| 报错成本 | 小模板，报错可读 | 大模板，报错几百行 |
| 策略 | 编译期固定 | 需要运行时切换 |

## 五、常见误用

1. **为了"避免虚函数开销"把接口模板化** —— 通常得不偿失，虚表开销远比想象中小。
2. **只用一次也上模板** —— 只承担缺点，没有收益。
3. **模板基类 + 虚函数混用** —— 每个实例化是不同类型，无法统一持有。
4. **把实现拆到 `.cpp`** —— 链接失败，或被迫显式实例化（类型写死）。
5. **模板参数出现在跨模块公开 API 上** —— ABI 不友好，调用方被迫知道实现细节。
6. **策略用模板参数** —— 需要运行时切换时切不了。

## 六、复用次数 ≠ 调用次数

模板的复用收益 = **不同实例化的数量**，不是调用点的数量。

| | 调用点 | 实例化 | 模板收益 |
|---|---|---|---|
| 一个类被 100 处使用，但只有一种类型 | 100 | **1** | **0** |
| 一个类被 3 处使用，但有两种类型 | 3 | **2** | **有** |

「调用一百次」不等于「复用一百次」。这是判断模板是否划算时最容易踩的坑。

同一件事在两种语境下答案相反：

| 语境 | `TCache<K,V>` 被实例化几次 | 模板划算吗 |
|---|---|---|
| STL / folly 这类通用库 | 成千上万 | ✅ |
| 单个项目，只有一种缓存 | 1 | ❌ |
| 单个项目，后来出现第二种缓存 | 2 | ✅ |

所以判断点是「**我这个系统里会有几种类型**」，而不是「这个组件本身通用不通用」。

## 七、速查表

| 问题 | 答「是」则 |
|---|---|
| 会有 ≥ 2 种类型 / 容量？ | 模板 ✅ |
| 类型是数据结构的一部分？ | 模板类 ✅ |
| 类型只是操作的参数？ | 模板函数 ✅ |
| 需要在热路径内联、要零开销？ | 模板 ✅ |
| 需要编译期计算 / 校验？ | 模板 ✅ |
| 需要运行时替换实现？ | 虚接口 ❌ |
| 需要一个变量装多种类型？ | 类型擦除 / 虚接口 ❌ |
| 需要把实现藏进 `.cpp`？ | 模板 ❌ |
| 只有一种类型？ | 不用模板 ❌ |
| 只是数量多、类型没多？ | 不用模板 ❌ |

## 八、结论

**模板用编译时间、二进制体积、封装性和调试体验，换运行时的零开销与类型安全。**

这笔交易只在「**类型编译期确定** + **是热点** + **实例化 ≥ 2 次**」时才划算。

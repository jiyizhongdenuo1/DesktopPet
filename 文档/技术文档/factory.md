# 工厂模式：作用与适用场景

## 一、作用

工厂模式把「用什么具体类型、怎么构造」从调用方剥离出来。它值钱的地方只有三件事：

| 价值 | 说明 | 缺了它的后果 |
|---|---|---|
| **解耦** | 调用方不依赖具体产品类 | 换一个产品类，所有调用方都要改 |
| **多态** | 运行时决定创建哪个实现 | 无法按配置 / 平台 / 环境切换实现 |
| **收拢复杂装配** | 创建过程有分支、要注入依赖、要绑回调或资源 | 同样的装配代码散落在多处 |

判断一个工厂是否值得存在，就看它是否提供了上面**至少一项**。三项都不提供，它只是一层转发。

## 二、适用场景

满足以下任一条，工厂才是合理的：

### 1. 产品类不唯一

- 同一接口有多个实现，需要运行时选择
- 平台差异：Windows / Linux 两套实现
- 测试替身：真实现 / 假实现之间切换

### 2. 创建过程有分支或复杂装配

- 构造参数需要按配置 / 环境计算
- 创建时要注入多个依赖、绑定回调、注册资源
- 创建后还需要初始化步骤（登记到注册表、设置状态）

### 3. 创建逻辑需要集中并被多处复用

- 同一套创建规则被多个调用点使用
- 创建规则需要统一修改

## 三、不适用场景

| 场景 | 为什么不需要工厂 |
|---|---|
| 产品类唯一、创建方式统一 | 工厂只是 `make_shared<T>()` 的转发 |
| 只在一处创建 | 没有复用，收拢无从谈起 |
| 工厂没有接口、没有多态继承关系 | 调用方仍依赖具体产品类，没有解耦 |
| 创建后立刻销毁工厂 | 说明工厂既无状态也无存在价值 |
| 数量变多但创建方式不变 | 需要的是「注册表 / 宿主」，不是工厂 |

## 四、正反例子

### 正面例子

#### 例 1：产品类不唯一（平台差异）

```cpp
class IThread
{
public:
    virtual ~IThread() = default;
    virtual void Start() = 0;
};

class CWinThread   : public IThread { /* ... */ };
class CPosixThread : public IThread { /* ... */ };

class CThreadFactory
{
public:
    static std::shared_ptr<IThread> Create()
    {
#ifdef _WIN32
        return std::make_shared<CWinThread>();
#else
        return std::make_shared<CPosixThread>();
#endif
    }
};
```

**为什么合理**：调用方只依赖 `IThread`，平台差异被收拢在一处。换平台不改调用方。

#### 例 2：测试替身切换

```cpp
class CThreadFactory
{
public:
    // 测试时注入同步执行的假线程
    void SetImpl(std::function<std::shared_ptr<IThread>()> fn)
    {
        m_fnCreate = std::move(fn);
    }

private:
    std::function<std::shared_ptr<IThread>()> m_fnCreate;
};
```

**为什么合理**：产品类在运行时才决定，工厂是唯一的切换点。

#### 例 3：创建过程需要复杂装配

```cpp
std::shared_ptr<CDataSaveThread> MakeDataSaveThread()
{
    auto pSave    = std::make_shared<CDataSave>(std::make_unique<NOTE_FILE_HEADER>());
    auto pRWMgr   = std::make_shared<CDataRWMgr>(pSave, g_ConfigManager->GetDataPath());
    auto pHandler = std::make_shared<CDynsDataSaveThreadHandler>();
    pHandler->Attach(pRWMgr);
    return std::make_shared<CDataSaveThread>("DataSave Thread", pHandler, 1000);
}
```

**为什么合理**：创建不只是 `new`，还要注入依赖、绑定资源。装配规则收在一处，调用方不必重复。

#### 例 4：创建规则被多处复用

```cpp
// 多个模块都需要同一种线程
auto pThread = CThreadFactory::CreateDataSaveThread();
```

**为什么合理**：同一套创建规则有多个调用点，改规则只需改一处。

### 反面例子

#### 例 1：唯一实现 + 纯转发

```cpp
class CThreadFactory
{
public:
    std::unique_ptr<CThread> Create(const QString& strName,
                                    std::shared_ptr<CThreadHandler> pHandle,
                                    INT32 s32Interval)
    {
        if (pHandle == nullptr)
        {
            return nullptr;
        }
        return std::make_unique<CThread>(strName, pHandle, s32Interval);
    }
};
```

```cpp
factory.Create(strName, pHandler, 1000);   // 调用方写法
std::make_shared<CThread>(strName, pHandler, 1000);   // 等价写法
```

**为什么不合理**：工厂没有做任何事。调用方依然要 `#include "CThread.h"` 才能拿到 `CThread`，一点没解耦。

#### 例 2：只在一处创建，用完即弃

```cpp
void CAppSystem::CreateThread()
{
    CThreadFactory factory;                              // 局部对象

    d_ptr->m_vecpThreadHanders = factory.ReleaseHandlers();
    d_ptr->m_vecpThread        = factory.ReleaseThreads();
}                                                        // 工厂随即销毁
```

**为什么不合理**：只调用一次、没有状态、造完就扔。工厂在这里只是个搬运工。

#### 例 3：把配置表塞进工厂

```cpp
void CThreadFactory::InitThread()
{
    struct
    {
        QString                         m_strName;
        std::shared_ptr<CThreadHandler> m_pHandler;
        INT32                           m_s32Interval;
    } st_TaskArr[] =
    {
        {"AppLog Thread",    m_vecpThreadHanders[E_THREAD_LOG],       1000},
        {"AppSystem Thread", m_vecpThreadHanders[E_THREAD_SYSTEM],    1000},
        {"DataSave Thread",  m_vecpThreadHanders[E_THREAD_DYNC_DATA], 1000},
    };
    // ...
}
```

**为什么不合理**："这个应用需要哪些线程"是**应用层的知识**，被塞进了 Infrastructure 的工厂里。工厂成了决策的藏身处，而不是创建工具。

#### 例 4：抽象是空的

```cpp
template <typename T>
class IFactory
{
public:
    explicit IFactory() = default;
    ~IFactory() = default;
    // virtual T* Create();      ← 被注释掉了
};

class CThreadFactory : public IFactory<CThread> { /* ... */ };
```

**为什么不合理**：继承了一个没有任何虚函数的基类，既没有多态，也无法通过基类指针使用；`~IFactory()` 还非虚。这种继承只是形式。

#### 例 5：用工厂代替注册表 / 宿主

```cpp
// 场景：线程从 3 个涨到 8 个，创建方式完全没变

// ❌ 再加一层工厂 —— 创建没变复杂，只是数量多了
class CThreadFactory { /* 更大的配置表 */ };

// ✅ 把"需要哪些"提成数据，给宿主加能力
struct SThreadSpec
{
    E_THREAD_ID                     eId;
    QString                         strName;
    INT32                           s32Interval;
    std::shared_ptr<CThreadHandler> pHandler;
};

VOID CThreadHost::InitThreads(const std::vector<SThreadSpec>& vecSpecs);
VOID CThreadHost::StartThreads();
VOID CThreadHost::StopThreads();
```

**为什么不合理**：数量增长带来的是「持有 / 查找 / 启停」的复杂度，属于宿主职责；创建本身没有变复杂，工厂解决不了这些问题。

### 正反对照

| 维度 | 正面 | 反面 |
|---|---|---|
| 产品类 | 多种实现，有统一接口 | 只有一种，或没有接口 |
| 调用方依赖 | 只依赖接口 | 依然 `#include` 具体类 |
| 创建逻辑 | 有分支 / 需要装配 | 一行 `make_shared` |
| 调用次数 | 多处复用 | 一处，用完即弃 |
| 配置表位置 | 属于创建规则的一部分 | 把应用层知识藏进实现层 |
| 抽象 | 有虚接口，可替换 | 空基类或纯转发 |

## 五、常见误用

1. **为了"看起来规范"而加工厂** —— 只有一种实现时，工厂是纯噪音。
2. **把配置表塞进工厂** —— "这个应用需要哪些组件"是应用层的知识，放进工厂等于把决策藏进实现层。
3. **返回裸指针 / 转移所有权但签名看不出来** —— 调用方无法判断谁负责释放。
4. **工厂既创建又管理** —— 创建是一次性的，管理是长期的，混在一起职责不清。
5. **用工厂代替线程池** —— 工厂只负责"造"；调度、复用、回收是池的职责。

## 六、数量增长 ≠ 需要工厂

常见误解：组件数量多了就该上工厂。实际上数量增长改变的是**管理复杂度**，不是**创建复杂度**：

| | 3 个 | 30 个 |
|---|---|---|
| 创建一行代码 | 不变 | **不变** |
| 谁持有 | 简单 | 变难 |
| 按用途查找 | 简单 | 变难 |
| 启动 / 停止顺序 | 无所谓 | 变难 |

变的都是「持有 / 查找 / 启停」——那是**注册表 / 宿主**的职责。正确做法是：

- 把"需要哪些组件"提成一份**清单（数据）**，放在应用层；
- 给宿主加"按清单创建 / 统一启停 / 按 ID 查找"的能力；
- **不要新增一层工厂。**

## 七、速查表

| 问题 | 答「是」则 |
|---|---|
| 产品类有多种实现、需要运行时选择？ | 工厂 ✅ |
| 创建过程有分支或复杂装配？ | 工厂 ✅ |
| 创建规则被多处复用？ | 工厂 ✅ |
| 只有一种实现、一种造法？ | 工厂 ❌ |
| 只在一处创建？ | 工厂 ❌ |
| 只是数量变多、造法没变？ | 注册表 / 宿主，不是工厂 |

## 八、结论

**工厂是「产品类不唯一」或「创建过程复杂」时用的。**
**数量增长带来的复杂度属于「管理」，应该加强宿主和清单，而不是加工厂。**

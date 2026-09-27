# 设计模式：分类、适用场景与正反例子

## 一、总览

### 创建型（管"怎么造对象"）

| 模式 | 一句话 | 本项目 |
|---|---|---|
| 工厂方法 | 把"造哪个产品"交给工厂 / 子类 | `CThreadFactory`（未达标，见 `Factory/README.md`） |
| 抽象工厂 | 造"一族"相互配套的产品 | 无 |
| 建造者 | 分步构造复杂对象 | 组合根的装配流程（勉强算） |
| 原型 | 拷贝现有对象来创建 | 无 |
| 单例 | 全局唯一实例 | `CAppSystem`、`CConfigManager`、`CServiceLocator` |

### 结构型（管"怎么组合"）

| 模式 | 一句话 | 本项目 |
|---|---|---|
| 适配器 | 转换接口，让不兼容的双方能协作 | `CMainNoteListViewModel` |
| 桥接 | 抽象与实现分离，各自独立变化 | 端口与适配器更贴切 |
| 组合 | 树形结构里统一处理"单个"与"整体" | 无 |
| 装饰器 | 动态给对象叠加职责 | 无 |
| 外观 | 给子系统一个统一的简化入口 | `CNoteDataService` |
| 享元 | 共享细粒度对象省内存 | 无 |
| 代理 | 用代理控制对目标对象的访问 | 无（pimpl 不是代理） |

### 行为型（管"怎么交互"）

| 模式 | 一句话 | 本项目 |
|---|---|---|
| 责任链 | 请求沿链传递，直到被处理 | 无 |
| 命令 | 把请求封装成对象 | `CDynsDataSaveThreadHandler` |
| 解释器 | 定义文法并解释 | 无 |
| 迭代器 | 顺序访问聚合，不暴露内部结构 | `QAbstractListModel` + QML 视图 |
| 中介者 | 对象间不直接通信，经中介者转发 | 无 |
| 备忘录 | 保存 / 恢复对象状态 | 无 |
| 观察者 | 一对多依赖，状态变化自动通知 | `CConfigManager::RegisterCallback` |
| 状态 | 状态决定行为，状态可切换 | 无 |
| 策略 | 算法可互换 | 无 |
| 模板方法 | 父类定骨架，子类填步骤 | `CThreadHandler` + `CThread` |
| 访问者 | 不改类的前提下加新操作 | 无 |

### GoF 之外

| 模式 | 一句话 | 本项目 |
|---|---|---|
| 依赖注入 | 依赖从外部传入，不在内部创建 | `CNoteDataService`、`CDataRWMgr` |
| 端口与适配器 | 核心定义接口，外部提供实现 | `INoteDataBuffer`、`CThreadHandler` |
| 服务定位器 | 通过全局注册表按需取服务 | `CServiceLocator` |
| 仓储 | 用集合语义封装持久化 | 无 |
| 工作单元 | 一批修改一起提交 | 无 |
| 对象池 | 复用昂贵对象 | 无 |
| 双缓冲 | 读写分离，交换指针 | 无 |
| 空对象 | 用空实现代替 null 判断 | 无 |
| 规格 | 把业务规则封装成可组合对象 | 无 |

### C++ 特有惯用法

| 惯用法 | 一句话 | 本项目 |
|---|---|---|
| RAII | 资源随对象生命周期自动管理 | `shared_ptr` / `unique_ptr` / `shared_lock` |
| pimpl | 隐藏实现、稳定 ABI、隔离编译 | `CNoteDataCache`、`CUserConfig`、`CAppSystem` |
| CRTP | 编译期多态，无虚表 | 无 |
| 类型擦除 | 一个接口装多种类型 | `std::function` |
| NVI | 非虚公有接口 + 私有虚函数 | 无 |
| copy-and-swap | 异常安全的赋值 | 无 |

---

## 二、已用到的模式（详解）

### 2.1 单例 Singleton

**作用**：保证一个类只有一个实例，并提供全局访问点。

**适用场景**：全局唯一资源（线程宿主、配置管理器、服务注册表），生命周期贯穿整个应用。

**不适用**：需要多实例或测试替换 → 用依赖注入；只是"图方便"要全局访问 → 会变成隐藏依赖。

**正例**（局部静态变量，C++11 起线程安全，无需手动加锁）：

```cpp
class CServiceLocator
{
public:
    static CServiceLocator& Instance()
    {
        static CServiceLocator s_instance;
        return s_instance;
    }

    CServiceLocator(const CServiceLocator&) = delete;
    CServiceLocator& operator=(const CServiceLocator&) = delete;

private:
    CServiceLocator() = default;
};
```

**反例**：

```cpp
CAppSystem* CAppSystem::GetInstance()
{
    if (!m_pInstance)                       // 无锁：多线程首次调用可能建出两个
    {
        m_pInstance = new CAppSystem();     // 裸 new，永不释放
    }
    return m_pInstance;
}
```

**本项目**：`CAppSystem`、`CConfigManager`、`CServiceLocator`。

---

### 2.2 观察者 Observer

**作用**：一对多依赖，被观察者状态变化时自动通知所有观察者。

**适用场景**：配置变更要通知多处；数据加载完成后要通知 UI。

**正例**：

```cpp
enum E_BCFUN_TYPE { E_BCFUN_TYPE_THEME = 0, E_BCFUN_TYPE_WINDOW_POS, E_BCFUN_TYPE_MAX };

bool RegisterCallback(E_BCFUN_TYPE eType, std::function<void()> func);

void NotifyChanged(E_BCFUN_TYPE eType)
{
    auto it = m_mapFunc.find(eType);
    if (it != m_mapFunc.end() && it->second)
    {
        it->second();
    }
}
```

**反例**：

- 回调里捕获了生命周期更短的对象 → 通知时悬空
- 持锁调用回调，回调里又去注册 / 注销 → 死锁
- 只支持一个回调却叫"观察者"（观察者的本质是**多个**订阅者）

**本项目**：`CConfigManager::RegisterCallback` + `NotifyConfigChanged`；`CNoteDataService::RegisterNoteModelDataLoadCallback`。
注意 `RegisterCallback` 是"同一类型只能注册一个，重复覆盖"——严格说更像**回调点**，不是完整观察者。

---

### 2.3 模板方法 Template Method

**作用**：父类定义算法骨架，把可变步骤留给子类。

**适用场景**：流程固定、中间某几步因场景而异。

**正例**：

```cpp
class CThreadHandler
{
public:
    virtual void HandleTask() = 0;          // 子类填这一步
};

void CThread::run()                          // 父类定骨架
{
    while (!m_bIsExit)
    {
        WaitForWakeup();
        if (m_bIsExit) { break; }
        m_pTaskHandler->HandleTask();        // 可变步骤
    }
}
```

**反例**：

- 骨架里用 `if (类型 == X)` 分支代替虚函数
- 在构造 / 析构里调用虚函数（此时子类部分尚未构造 / 已析构）

**本项目**：`CThreadHandler::HandleTask()` + `CThread::run()`。

---

### 2.4 适配器 Adapter

**作用**：把不兼容的接口转换成调用方期望的接口。

**适用场景**：跨技术栈边界（C++ ↔ QML）、旧接口接入新系统。

**正例**：

```cpp
QVariant CMainNoteListViewModel::data(const QModelIndex& index, int role) const
{
    const st_NoteModelItem& note = m_vecNote.at(index.row());

    switch (role)
    {
    case RoleNoteContent: return QString::fromStdString(note.m_strContent);
    case RoleNoteLevel:   return note.m_eNoteLevel;
    default:              return QVariant();
    }
}
```

把 C++ 领域数据适配成 QML 能消费的 role。

**反例**：

- 适配层里塞业务逻辑（判断、计算、统计）
- 适配器持有状态并跨线程共享

**本项目**：`CMainNoteListViewModel`（领域数据 → QML role）。

---

### 2.5 外观 Facade

**作用**：为子系统提供统一的简化入口。

**适用场景**：一个操作背后要协调多个子系统，调用方不该知道细节。

**正例**：

```cpp
void CNoteDataService::AddNote(const NOTE_MODEL_ITEM& stModelItem)
{
    ST_NOTE_DATA stData{};
    ConvertUIToDomain(stModelItem, stData);   // 格式转换
    m_pCache->SaveNoteDataCache(stData);      // 写缓存（供显示）
    m_pBuffer->AppendData(stModelItem);       // 写待写队列（供持久化）
}
```

调用方只需 `AddNote`，不必知道背后有三步。

**反例**：

- 外观膨胀成"上帝类"，把子系统的每个方法都转发一遍
- 外观泄漏子系统类型（把内部对象返回出去）

**本项目**：`CNoteDataService`。

---

### 2.6 端口与适配器 Ports & Adapters

**作用**：核心定义接口（端口），外部提供实现（适配器），依赖方向指向核心。

**适用场景**：分层架构的模块边界；需要隔离基础设施（文件、网络、线程、UI）。

**正例**：

```cpp
// 核心（Domain）定义端口
class INoteDataBuffer
{
public:
    virtual ~INoteDataBuffer() = default;
    virtual void AppendData(const NOTE_MODEL_ITEM& stModelItem) = 0;
    virtual INT32 ReadBuffer(INT32 s32GetSize, char* pcBuffer) = 0;
};

// 外部（Infrastructure）实现适配器
class CNoteDataCollect : public INoteDataBuffer { /* ... */ };

// 应用层注入
CNoteDataService(std::shared_ptr<CNoteDataCache> pCache,
                 std::shared_ptr<INoteDataBuffer> pBuffer);
```

**反例**：

- 接口定义在 Infrastructure，核心反向依赖实现层
- 把端口做成模板基类 → 每个实例化是不同类型，无法统一持有
- 端口方法里暴露实现细节（返回内部容器、内部消息名）

**本项目**：`INoteDataBuffer`（出站端口）、`CThreadHandler`（入站端口）。

---

### 2.7 依赖注入 DI

**作用**：依赖从外部传入，类内部不自己创建。

**适用场景**：需要替换实现（测试替身）、需要控制生命周期、需要明确依赖关系。

**正例**：

```cpp
CNoteDataService::CNoteDataService(std::shared_ptr<CNoteDataCache> pCache,
                                   std::shared_ptr<INoteDataBuffer> pBuffer)
    : m_pCache(std::move(pCache))
    , m_pBuffer(std::move(pBuffer))
{
}
```

**反例**：

```cpp
void CMainNoteListViewModel::InitService()
{
    m_pNoteService = g_ServiceLocator.GetNoteService();   // 内部反查全局：隐藏依赖
}
```

**本项目**：构造函数注入（`CNoteDataService`、`CDataRWMgr`）；
反面：`CMainNoteListViewModel::InitService()` 里从全局定位器反查。

---

### 2.8 命令 Command

**作用**：把请求封装成对象，支持排队、延迟执行、分发。

**适用场景**：跨线程传递任务；请求需要排队；请求的发起方与执行方解耦。

**正例**：

```cpp
// 封装请求并入队
ST_DATA_SAVE_EVENT tmpEvent;
tmpEvent.strMsgKey = strKey;
m_queSaveEvent.push(tmpEvent);

// 消费端按 key 分发
auto it = m_mapFunc.find(event.strMsgKey);
if (it != m_mapFunc.end())
{
    it->second(m_pDataSaveRWMgr, event);
}
```

**反例**：

- 命令对象里存裸指针 / 引用，生命周期不明
- 队列无上限，生产快于消费时无限增长
- 用 if-else 链代替分发表，新增命令要改核心

**本项目**：`CDynsDataSaveThreadHandler` 的事件队列 + `m_mapFunc` 分发表。

---

### 2.9 迭代器 Iterator

**作用**：顺序访问聚合元素，不暴露内部结构。

**适用场景**：需要遍历但不想暴露容器；需要在遍历时加保护（锁、边界检查）。

**正例**：

```cpp
// 通过"访问者"遍历，锁在内部持有，调用方拿不到内部缓冲区
void CNoteDataCache::ForEachNote(const std::function<bool(const ST_NOTE_DATA&)>& fn) const
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    for (INT32 i = 0; i < m_s32Count; ++i)
    {
        if (!fn(m_arr[i]))
        {
            break;
        }
    }
}
```

**反例**：

```cpp
// 把内部缓冲区直接交出去：暴露实现 + 调用方无锁读
std::shared_ptr<std::array<ST_NOTE_DATA, 500>> CNoteDataCache::GetCache();
```

**本项目**：`CMainNoteListViewModel`（QML 通过 role 迭代）；反例见 `CNoteDataCache::GetCache()`。

---

### 2.10 服务定位器 Service Locator

**作用**：通过全局注册表按需取服务。

**适用场景**：仅限**模块边界**——在装配点注册一次，在无法注入的边界（如 QML 回调、静态函数）取一次。

**正例**：

```cpp
// 组合根里注册（唯一注册点）
g_ServiceLocator.RegisterNoteCache(noteCache);
g_ServiceLocator.RegisterNoteCollect(noteCollect);
g_ServiceLocator.RegisterNoteService(noteService);
```

**反例**：

- 业务类内部到处反查 → 依赖被隐藏，无法单测
- 用定位器代替依赖注入（这是它被视为反模式的原因）

**本项目**：`CServiceLocator`；反面用法见 2.7 的反例。

---

## 三、其余模式索引

| 模式 | 一句话 | 什么时候才用 |
|---|---|---|
| 抽象工厂 | 造一族相互配套的产品 | 有多套实现族（如多主题、多平台 UI 组件） |
| 建造者 | 分步构造复杂对象 | 构造步骤多且顺序有讲究，或需要多种表示 |
| 原型 | 拷贝现有对象来创建 | 创建代价远大于拷贝，或类型在运行时才知道 |
| 桥接 | 抽象与实现各自独立变化 | 两个维度都会变（如"形状 × 渲染器"） |
| 组合 | 树形结构统一处理单个与整体 | 有真正的树形数据（目录、表达式、UI 组件树） |
| 装饰器 | 动态给对象叠加职责 | 需要按组合方式加功能，且不想改原类 |
| 享元 | 共享细粒度对象省内存 | 大量重复的小对象，内存是瓶颈 |
| 代理 | 控制对目标对象的访问 | 需要延迟加载、访问控制、引用计数、远程代理 |
| 责任链 | 请求沿链传递直到被处理 | 有多个候选处理者，且顺序可配置 |
| 解释器 | 定义文法并解释 | 需要解析小型语言 / 表达式 |
| 中介者 | 对象间不直接通信 | 对象间交互成网状，需要收敛成星形 |
| 备忘录 | 保存 / 恢复对象状态 | 需要撤销 / 快照 |
| 状态 | 状态决定行为 | 有明确状态机，且行为随状态整体变化 |
| 策略 | 算法可互换 | 同一问题有多种算法，需要运行时或配置切换 |
| 访问者 | 不改类的前提下加新操作 | 数据结构稳定，但操作经常新增 |
| 仓储 | 用集合语义封装持久化 | 领域层不想知道存储细节 |
| 工作单元 | 一批修改一起提交 | 多个实体要在一个事务里提交 |
| 对象池 | 复用昂贵对象 | 创建 / 销毁代价高（连接、线程、大缓冲） |
| 双缓冲 | 读写分离，交换指针 | 读多写多且要求读方无锁 |
| 空对象 | 用空实现代替 null 判断 | 到处都在判空，且空行为可以定义 |
| 规格 | 业务规则封装成可组合对象 | 查询 / 筛选条件需要自由组合复用 |

---

## 四、怎么选

### 三条原则

1. **模式解决的是「变化点」**。先确认哪里会变（产品类会变？算法会变？通知关系会变？），再挑对应模式。
2. **先有变化，再上模式**。没有第二个实现、没有第二种策略、没有多观察者——模式就是纯噪音。
3. **模式换来「隔离变化」，代价是「多一层间接」**。这层间接值不值，看变化发生的频率。

### 变化点 → 模式

| 变化点 | 对应模式 |
|---|---|
| 造什么对象会变 | 工厂方法 / 抽象工厂 |
| 构造过程复杂 | 建造者 |
| 算法会变 | 策略 |
| 状态会变 | 状态 |
| 谁被通知会变 | 观察者 |
| 接口不匹配 | 适配器 |
| 要加职责又不想改类 | 装饰器 / 访问者 |
| 访问需要控制 | 代理 |
| 请求需要排队 / 延迟 | 命令 |
| 遍历方式会变 | 迭代器 |
| 两个维度都会变 | 桥接 |
| 对象间交互成网状 | 中介者 |
| 需要撤销 / 快照 | 备忘录 |
| 全局唯一 | 单例 |

### 一条提醒

**不要为了"看起来规范"而上模式。** 判据和工厂、模板是同一套：**它是否隔离了一个真实存在的、会反复发生的变化？** 答案是否，就不该上。

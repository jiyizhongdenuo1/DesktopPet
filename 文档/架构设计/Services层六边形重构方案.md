# Services 层六边形架构重构方案

## 一、当前代码现状分析

### 1.1 现有目录结构

```
Services/
├── Base/                          领域模型（位置正确）
│   ├── DServiceBase.h            ST_NOTE_DATA（便签实体定义）
│   ├── DDataMgrBase.h           NOTE_MODEL_ITEM（UI 模型）
│   ├── INoteDataBuffer.h        缓冲区接口
│   └── datatype.h               基础类型定义
│
├── Module/BusinessModule/        业务层（名不副实）
│   ├── CNoteBusiness/           空壳！无实际逻辑
│   │   ├── CNoteBusiness.h      只有构造/析构声明
│   │   └── CNoteBusiness.cpp    函数体为空
│   └── Config/                  通用配置（不属于业务域）
│       ├── CCommonConfig.h/cpp  管理系统/日志/UI/数据配置
│       └── DConfig.h            配置键名常量
│
├── Module/Adapter/DataManager/   **真正的业务逻辑在这里**（位置错误）
│   ├── CNoteDataService.h/cpp   便签完整业务逻辑：
│   │   - AddNote()              添加便签
│   │   - ConvertUIToDomain()     UI→领域转换
│   │   - LoadFromBuffer()       从文件加载
│   │   - 缓存管理 + 待写队列管理
│   └── CServiceLocator.h/cpp    全局单例服务定位器
│
├── Module/Other/                基础设施（位置正确但命名不明确）
│   ├── DataCache/
│   │   └── CNoteDataCache      缓存实现
│   ├── DataReadWriter/
│   │   ├── CDataRWMgr         文件 IO 管理
│   │   └── CDataSave          底层文件操作
│   ├── TaskThread/
│   │   └── CDynsDataSaveThreadHandler  数据保存线程
│   └── Factory/
│       └── CThreadFactory      线程工厂
│
└── WidgetServices/             ViewModel（UI 与业务桥接）
    └── MainWidget/
        └── CMainNoteListViewModel
```

### 1.2 核心问题

| 问题编号 | 问题 | 说明 | 严重程度 |
|---------|------|------|---------|
| P-01 | 业务逻辑放错位置 | `CNoteDataService` 在 Adapter 层，却包含完整的便签业务逻辑 | 🔴 高 |
| P-02 | 空壳类误导 | `CNoteBusiness` 名字叫"业务"，但没有任何代码 | 🟡 中 |
| P-03 | 配置位置错误 | `CCommonConfig` 放在 BusinessModule 下，但它属于基础设施 | 🟡 中 |
| P-04 | 领域模型散落 | `ST_NOTE_DATA` 在 Base/DServiceBase.h，没有独立领域包 | 🟠 中高 |
| P-05 | 全局耦合 | ServiceLocator 是全局单例，所有模块直接依赖它 | 🟡 中 |
| P-06 | 命名模糊 | `Services`、`Other`、`Module` 语义不清 | 🟢 低 |

### 1.3 当前依赖方向（有问题）

```
Widgets/QML (UI)
    ↓ 调用
Adapter/CNoteDataService  ← ⚠️ 业务逻辑在这（应该只在端口层做适配）
    ↓ 依赖
Other/DataCache, DataReadWriter  ← 基础设施
    ↓ 依赖
Base/DServiceBase.h  ← 领域定义在这（正确）
```

**问题：**
- Adapter 层包含了用例逻辑（Create、Load、Convert）
- 领域实体（ST_NOTE_DATA）是裸结构体，没有封装行为
- 没有仓储接口，直接依赖具体实现

---

## 二、六边形架构目标结构

### 2.1 目标目录结构

```
Services/
├── Domain/                          领域层（纯业务规则，零外部依赖）
│   ├── NoteDomain/                 便签领域
│   │   ├── Entities/
│   │   │   └── CNote.h           便签实体（从 ST_NOTE_DATA 封装）
│   │   ├── ValueObjects/
│   │   │   ├── ENoteType.h       事件类型枚举
│   │   │   ├── EWakeUpLevel.h    唤醒级别枚举
│   │   │   ├── ETimeSpanType.h   时间跨度类型
│   │   │   └── ERemindFrequency.h 提醒频率枚举
│   │   └── RepositoryInterfaces/
│   │       └── INoteRepository.h 仓储接口（纯虚函数）
│   ├── ReminderDomain/            提醒领域（未来扩展）
│   │   ├── Entities/
│   │   │   └── CReminder.h
│   │   └── RepositoryInterfaces/
│   │       └── IReminderRepository.h
│   ├── AIDomain/                  AI 交流领域（未来扩展）
│   └── SharedKernel/              共享内核
│       ├── Datatype.h            基础类型（INT32、BOOL 等）
│       └── CommonDefine.h        通用宏定义
│
├── UseCases/                       用例层（业务流程编排，只依赖 Domain）
│   ├── NoteUseCases/
│   │   ├── CreateNoteUC.h/cpp    创建便签用例
│   │   │   - Execute(note) → 调用 repo.Save()
│   │   ├── DeleteNoteUC.h/cpp    删除便签用例
│   │   ├── UpdateNoteUC.h/cpp    更新便签用例
│   │   ├── ListNotesUC.h/cpp     查询便签列表用例
│   │   └── LoadNotesFromFileUC.h/cpp 从文件加载用例
│   │       - Execute() → 调用 repo.ReadAll() + cache.Update()
│   ├── ReminderUseCases/          （未来提醒功能）
│   └── AIUseCases/                （未来 AI 功能）
│
├── Infrastructure/                基础设施（外部依赖的具体实现）
│   ├── Persistence/
│   │   ├── FileNoteRepo.h/cpp    INoteRepository 的文件实现
│   │   ├── CDataRWMgr.h/cpp      数据读写管理器（从 Other 移入）
│   │   └── CDataSave.h/cpp       底层文件操作（从 Other 移入）
│   ├── Cache/
│   │   └── CNoteDataCache.h/cpp  缓存实现（从 Other 移入）
│   ├── Threading/
│   │   ├── CDynsDataSaveThreadHandler.h/cpp（从 Other 移入）
│   │   ├── CLogThreadHandler.h/cpp（从 Other 移入）
│   │   └── CSystemThreadHandler.h/cpp（从 Other 移入）
│   ├── Factory/
│   │   └── CThreadFactory.h/cpp  工厂模式（从 Other 移入）
│   └── Config/
│       ├── CCommonConfig.h/cpp   配置管理（从 BusinessModule 移入）
│       └── DConfig.h             配置键名常量（从 BusinessModule 移入）
│
├── Ports/                         端口层（内外交互接口）
│   ├── PrimaryPorts/              驱动端端口（被 UI、外部系统调用）
│   │   ├── INotePresenter.h      便签展示接口
│   │   └── IReminderPresenter.h  提醒展示接口（未来）
│   └── SecondaryPorts/            被驱动端端口（调用外部系统）
│       ├── IFileWriter.h          文件写入接口
│       └── ICacheProvider.h      缓存提供接口
│
└── Application/                   应用层（组合根，组装所有依赖）
    ├── CompositionRoot.h/cpp     依赖注入容器
    ├── ServiceLocator.h/cpp      服务定位器（精简版，仅启动时使用）
    └── AppConfig.h/cpp          应用配置加载
```

### 2.2 六边形架构图

```
                    ┌─────────────────────────────────────┐
                    │         六边形边界 (Ports)            │
                    │                                      │
    ┌───────────────┤  Primary Ports (驱动端)               ├───────────────┐
    │ UI / 外部系统  │  • INotePresenter                    │  定时任务      │
    │ Widgets/QML   │  • IReminderPresenter                 │  文件监听      │
    │               │                                       │               │
    │  ┌────────────┴──────────────────────────────────────┴────────────┐  │
    │  │                    Application (应用层)                        │  │
    │  │              CompositionRoot + ServiceLocator                 │  │
    │  └────────────────────────┬─────────────────────────────────────┘  │
    │                           │                                        │
    │  ┌────────────────────────┴─────────────────────────────────────┐  │
    │  │                     UseCases (用例层)                         │  │
    │  │  • CreateNoteUC  • DeleteNoteUC  • LoadNotesFromFileUC       │  │
    │  └────────────────────────┬─────────────────────────────────────┘  │
    │                           │                                        │
    │  ┌────────────────────────┴─────────────────────────────────────┐  │
    │  │                      Domain (领域层)                          │  │
    │  │  • NoteDomain (Entities + ValueObjects + RepoInterfaces)     │  │
    │  │  • ReminderDomain • AIDomain • SharedKernel                  │  │
    │  └────────────────────────┬─────────────────────────────────────┘  │
    │                           │                                        │
    │  ┌────────────────────────┴─────────────────────────────────────┐  │
    │  │                Infrastructure (基础设施层)                     │  │
    │  │  Persistence: FileNoteRepo, CDataRWMgr, CDataSave            │  │
    │  │  Cache: CNoteDataCache                                       │  │
    │  │  Threading: ThreadHandlers                                    │  │
    │  │  Config: CCommonConfig                                        │  │
    │  └──────────────────────────────────────────────────────────────┘  │
    │                                                                       │
    └───────────────────────────────────────────────────────────────────────┘
                            Secondary Ports (被驱动端)
                              • IFileWriter
                              • ICacheProvider
```

---

## 三、核心设计原则

### 3.1 依赖方向规则（不可违反）

```
✅ 允许的依赖：
  UseCases → Domain
  Infrastructure → Domain (实现接口)
  Ports → Domain + UseCases
  Application → 所有层

❌ 禁止的依赖：
  Domain → UseCases (领域不知道用例)
  Domain → Infrastructure (领域不依赖具体实现)
  Infrastructure → UseCase (基础设施不知道业务流程)
```

### 3.2 各层职责定义

#### Domain（领域层）

**职责：** 定义业务概念和规则

**特点：**
- 纯 C++，不依赖 Qt、不依赖文件系统、不依赖数据库
- 只包含实体类、值对象、领域服务、仓储接口
- 所有方法都是纯逻辑，可单元测试

**示例：**
```cpp
// Domain/NoteDomain/Entities/CNote.h
#pragma once
#include "../ValueObjects/ENoteType.h"
#include "../ValueObjects/EWakeUpLevel.h"
#include <string>

class CNote {
public:
    INT64 id = 0;
    std::string title;
    std::string content;
    E_NOTE_EVENT_TYPE eventType = E_NOTE_EVENT_NONE;
    E_NOTE_EVENT_WAKEUP_LEVEL wakeUpLevel = E_NOTE_EVENT_WAKEUP_LEVEL_NORMAL;
    
    bool IsValid() const;           // 领域验证逻辑
    static CNote FromRaw(const ST_NOTE_DATA& raw);  // 工厂方法
    ST_NOTE_DATA ToRaw() const;
};
```

#### UseCases（用例层）

**职责：** 编排单个业务场景的完整流程

**特点：**
- 一个用例类 = 一个公开方法 `Execute()`
- 协调 Domain 对象 + 调用 Repository 接口
- 不包含 UI 逻辑、不包含持久化细节

**示例：**
```cpp
// UseCases/NoteUseCases/CreateNoteUC.h
#pragma once
#include "../../Domain/NoteDomain/Entities/CNote.h"
#include "../../Domain/NoteDomain/RepositoryInterfaces/INoteRepository.h"

class CreateNoteUC {
public:
    explicit CreateNoteUC(std::shared_ptr<INoteRepository> repo);
    
    void Execute(const CNote& note);
    
private:
    std::shared_ptr<INoteRepository> m_repo;
};

// CreateNoteUC.cpp
void CreateNoteUC::Execute(const CNote& note) {
    if (!note.IsValid()) {
        throw std::invalid_argument("Invalid note data");
    }
    m_repo->Save(note);
}
```

#### Infrastructure（基础设施层）

**职责：** 实现领域层定义的接口，处理外部依赖

**特点：**
- 依赖 Qt、文件系统、数据库等
- 实现 Domain 的 Repository 接口
- 包含缓存、线程、配置等横切关注点

**示例：**
```cpp
// Infrastructure/Persistence/FileNoteRepo.h
#pragma once
#include "../../Domain/NoteDomain/RepositoryInterfaces/INoteRepository.h"
#include <memory>
#include "CDataRWMgr.h"

class FileNoteRepo : public INoteRepository {
public:
    explicit FileNoteRepo(std::shared_ptr<CDataRWMgr> dataMgr);
    
    void Save(const CNote& note) override;
    std::vector<CNote> FindAll() override;
    void Delete(INT64 id) override;
    
private:
    std::shared_ptr<CDataRWMgr> m_dataMgr;
};
```

#### Ports（端口层）

**职责：** 定义内外交互的接口契约

**特点：**
- PrimaryPorts：定义供 UI 调用的接口
- SecondaryPorts：定义调用外部的接口
- 接口在 Domain 或 Ports 定义，实现在 Infrastructure 或 UI 层

#### Application（应用层）

**职责：** 启动时组装所有依赖（组合根模式）

**特点：**
- 只在程序启动时运行一次
- 创建所有对象并注入依赖
- ServiceLocator 仅在此层使用，之后传递给各模块

---

## 四、迁移步骤（分阶段实施）

### 阶段一：创建 Domain 层（风险最低）

**目标：** 把散落在 Base 的领域概念封装成正式的领域对象

**步骤：**

1. 创建 `Services/Domain/` 目录
2. 创建 `SharedKernel/`，移动 `datatype.h`
3. 创建 `NoteDomain/Entities/CNote.h`：
   ```cpp
   class CNote {
   public:
       // 字段（从 ST_NOTE_DATA 提取）
       INT64 id;
       std::string title;
       std::string content;
       E_NOTE_EVENT_TYPE eventType;
       // ...
       
       // 行为（新增）
       bool IsValid() const;
       bool IsUrgent() const;
       bool IsImportant() const;
       
       // 转换方法
       static CNote FromModelItem(const NOTE_MODEL_ITEM& item);
       NOTE_MODEL_ITEM ToModelItem() const;
       static CNote FromRawData(const ST_NOTE_DATA& raw);
       ST_NOTE_DATA ToRawData() const;
   };
   ```
4. 创建 `NoteDomain/ValueObjects/`，移动枚举类型
5. 创建 `NoteDomain/RepositoryInterfaces/INoteRepository.h`：
   ```cpp
   class INoteRepository {
   public:
       virtual ~INoteRepository() = default;
       virtual void Save(const CNote& note) = 0;
       virtual std::vector<CNote> FindAll() = 0;
       virtual void Delete(INT64 id) = 0;
       virtual CNote FindById(INT64 id) = 0;
   };
   ```

**验证：** 编译通过，现有功能不受影响

---

### 阶段二：提取 UseCase 层（中等风险）

**目标：** 从 CNoteDataService 抽出业务用例

**当前 CNoteDataService 的职责分析：**

| 方法 | 类型 | 应该去哪 |
|-----|------|---------|
| `AddNote()` | 用例 | → `CreateNoteUC` |
| `ConvertUIToDomain()` | 领域逻辑 | → `CNote.FromModelItem()` |
| `LoadFromBuffer()` | 用例 | → `LoadNotesFromFileUC` |
| `InvalidateCache()` | 基础设施 | → 保留在 Cache 模块 |
| `Register...Callback()` | UI 适配 | → ViewModel |

**步骤：**

1. 创建 `Services/UseCases/NoteUseCases/`
2. 实现 `CreateNoteUC`：
   ```cpp
   void CreateNoteUC::Execute(const NOTE_MODEL_ITEM& modelItem) {
       auto note = CNote::FromModelItem(modelItem);
       if (!note.IsValid()) return;
       m_repo->Save(note);
       
       // 通知缓存失效（通过事件或回调）
       if (m_cacheInvalidator) {
           m_cacheInvalidator->Invalidate();
       }
   }
   ```
3. 实现 `LoadNotesFromFileUC`：
   ```cpp
   void LoadNotesFromFileUC::Execute() {
       auto rawData = m_repo->FindAllRaw();
       for (const auto& data : rawData) {
           auto note = CNote::FromRawData(data);
           m_cache->Update(note);
       }
       // 通知 UI 刷新
       if (m_presenter) {
           m_presenter->OnNotesLoaded(m_cache->GetSnapshot());
       }
   }
   ```
4. 修改 `CNoteDataService` 为薄适配器：
   ```cpp
   class CNoteDataService {  // 改名为 NoteServiceAdapter
   public:
       void AddNote(const NOTE_MODEL_ITEM& item) {
           m_createUC->Execute(item);  // 委托给用例
       }
       
   private:
       std::unique_ptr<CreateNoteUC> m_createUC;
       std::unique_ptr<LoadNotesFromFileUC> m_loadUC;
   };
   ```

**验证：** 运行现有测试，确保行为不变

---

### 阶段三：重组 Infrastructure（低风险）

**目标：** 把 Other 重命名为 Infrastructure，调整子目录

**步骤：**

1. 重命名 `Module/Other/` → `Infrastructure/`
2. 子目录重组：
   ```
   Infrastructure/
   ├── Persistence/          ← DataReadWriter/
   │   ├── FileNoteRepo.h/cpp (新建)
   │   ├── CDataRWMgr.h/cpp
   │   └── CDataSave.h/cpp
   ├── Cache/               ← DataCache/
   │   └── CNoteDataCache.h/cpp
   ├── Threading/           ← TaskThread/
   │   ├── CDynsDataSaveThreadHandler.h/cpp
   │   ├── CLogThreadHandler.h/cpp
   │   └── CSystemThreadHandler.h/cpp
   ├── Factory/             ← Factory/
   │   └── CThreadFactory.h/cpp
   └── Config/              ← 从 BusinessModule/Config/ 移入
       ├── CCommonConfig.h/cpp
       └── DConfig.h
   ```
3. 实现 `FileNoteRepo`（实现 INoteRepository 接口）：
   ```cpp
   void FileNoteRepo::Save(const CNote& note) {
       auto raw = note.ToRawData();
       m_dataMgr->AddOneNoteData(reinterpret_cast<const char*>(&raw), sizeof(raw));
   }
   
   std::vector<CNote> FileNoteRepo::FindAll() {
       // 调用 CDataRWMgr::ReadFromFile()
       // 转换成 vector<CNote>
   }
   ```

**验证：** 编译通过，现有测试通过

---

### 阶段四：清理与优化（最低优先级）

**目标：** 删除冗余代码，完善文档

**步骤：**

1. 删除空壳 `BusinessModule/CNoteBusiness/`
2. 精简 `CServiceLocator`：
   - 只保留启动时注入功能
   - 移除运行时全局访问（改用依赖注入）
3. 更新 CMakeLists.txt，反映新目录结构
4. 补充单元测试：
   - Domain 层：100% 覆盖率（无外部依赖，最容易测）
   - UseCases 层：Mock Repository 测试
5. 更新架构文档

---

## 五、关键决策记录

### 5.1 为什么不用 DDD 的 Aggregate/Entity 区分？

**原因：** 项目规模较小（目前只有便签一个业务域），过度建模会增加复杂度。
**策略：** 目前只用 Entity 表示业务对象，如果后续出现"便签+附件+评论"的聚合关系，再引入 AggregateRoot。

### 5.2 为什么保留 ServiceLocator？

**原因：** Qt/QML 的信号槽机制需要全局访问点；完全 DI 在 QML 中集成成本高。
**策略：** ServiceLocator 限制在 Application 层使用，UseCase 和 Domain 通过构造函数注入依赖。

### 5.3 CNoteDataService 是否删除？

**决策：** 不删除，改为薄适配器（Adapter）。
**原因：** 
- 直接修改 WidgetServices 和 QML 成本高
- 作为过渡层，逐步把逻辑迁移到 UseCase
- 最终可以变成纯粹的 Port 实现

### 5.4 ST_NOTE_DATA 结构体是否保留？

**决策：** 保留作为序列化格式，但不暴露给 UseCase 层。
**原因：**
- 文件存储需要二进制兼容性
- CNote 类提供 ToRawData()/FromRawData() 转换
- 新代码统一使用 CNote，ST_NOTE_DATA 变为内部实现细节

---

## 六、预期收益

### 6.1 可维护性提升

| 维度 | 现在 | 重构后 |
|-----|------|-------|
| 添加新业务（如提醒） | 在 Adapter 层堆砌逻辑 | 新建 Domain + UseCase，清晰隔离 |
| 修改保存逻辑 | 改 CNoteDataService（影响面大） | 改 FileNoteRepo 或对应 UseCase（影响面小） |
| 单元测试 | 困难（依赖 Qt、文件系统） | Domain 层 100% 可测试（纯 C++） |
| 代码定位 | 业务逻辑分散在 Adapter/Base | 按层查找，路径明确 |

### 6.2 扩展性提升

**新功能开发流程（重构后）：**

```
1. 定义需求（如：提醒功能）
   ↓
2. Domain/ReminderDomain/
   - CReminder.h (实体)
   - IReminderRepository.h (接口)
   ↓
3. UseCases/ReminderUseCases/
   - CreateReminderUC
   - CheckRemindersUC (定时检查)
   ↓
4. Infrastructure/Persistence/
   - FileReminderRepo (实现)
   - ReminderScheduler (定时器)
   ↓
5. WidgetServices/ (UI 适配)
   - CReminderViewModel
   ↓
6. QML 界面
```

每一步都是增量添加，不影响已有代码。

---

## 七、风险评估与缓解措施

| 风险 | 可能性 | 影响 | 缓解措施 |
|-----|--------|------|---------|
| 重构过程中引入 bug | 中 | 高 | 分阶段实施，每阶段都要跑通测试 |
| 团队学习成本 | 低 | 中 | 提供本文档 + 代码示例 |
| 性能下降（多一层抽象） | 低 | 低 | UseCase 内联简单，编译器会优化 |
| 过度设计 | 中 | 中 | 遵循 YAGNI 原则，只建需要的层 |
| CMake 构建配置复杂化 | 中 | 低 | 使用 CMake 的 target-based 依赖管理 |

---

## 八、参考资源

- [六边形架构（Ports & Adapters）- Alistair Cockburn](https://alistaircockburn.com/hexagonal-architecture.html)
- [Clean Architecture - Robert C. Martin](https://blog.cleancoder.com/uncle-bob/2012/08/13/the-clean-architecture.html)
- [DDD 快速入门 - Microsoft](https://learn.microsoft.com/en-us/dotnet/architecture/microservices/microservice-ddd-cqrs-patterns/)
- [SOLID 原则 - Wikipedia](https://en.wikipedia.org/wiki/SOLID)

---

## 九、版本历史

| 版本 | 日期 | 作者 | 说明 |
|-----|------|------|------|
| v1.0 | 2026-08-16 | nuo/AI | 初稿，基于当前 Services 代码分析 |

---

*本文档应随着重构进展持续更新，保持与代码同步。*
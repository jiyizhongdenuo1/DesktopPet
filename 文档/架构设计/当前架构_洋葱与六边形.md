# DesktopPet 当前架构图 - 洋葱模型 & 六边形端口适配器模型

## 一、洋葱架构视角

```
┌─────────────────────────────────────────────────────────────────────┐
│                         🌐 外部世界 (External)                       │
│                                                                     │
│   ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐     │
│   │  QML 视图 │    │  Qt 框架  │    │  文件系统  │    │  控制台    │     │
│   │ MainWidget│    │ QGuiApp  │    │  ~/SaveFile│    │  stdout   │     │
│   └─────┬─────┘    └─────┬─────┘    └─────┬─────┘    └─────┬─────┘     │
│         │               │               │               │           │
└─────────┼───────────────┼───────────────┼───────────────┼───────────┘
          │               │               │               │
          ▼               ▼               ▼               ▼
┌─────────────────────────────────────────────────────────────────────┐
│                     🖥️  表现层 (Presentation)                        │
│                                                                     │
│  ┌──────────────────────────────────────────────────────┐           │
│  │  CMainNoteListViewModel (QAbstractListModel)         │──────┐    │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────────────────┐ │      │    │
│  │  │ rowCount │ │ data()   │ │ PutArrNoteData() ⚠   │ │      │    │
│  │  │ roleNames│ │ setData  │ │ (跨线程回调风险)      │ │      │    │
│  │  └──────────┘ └──────────┘ └──────────────────────┘ │      │    │
│  └──────────────────────────────────────────────────────┘      │    │
│                                                                 │    │
│  ┌──────────────────────────────────────────────────────────┐  │    │
│  │  QML UI Layer (Widgets/)                                  │  │    │
│  │  MainWidget.qml ─┬─ ListOfNotes.qml                       │  │    │
│  │                  └─ PopInputNoteDia.qml                    │  │    │
│  └──────────────────────────────────────────────────────────┘  │    │
└─────────────────────────────────────────────────────────────────┼────┘
                                                                  │
                          ┌───────────────────────────────────────┘
                          │  contextProperty("noteModel", ...)
                          │  qml 绑定 / 数据驱动
                          ▼
┌─────────────────────────────────────────────────────────────────────┐
│                     🎯 应用层 (Application)                          │
│                     Use Cases / 编排 / 协调                          │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────┐     │
│  │  CAppBootstrapper (组合根 / Composition Root)              │     │
│  │  ┌─────────────────────────────────────────────────────┐  │     │
│  │  │  Initialize() 启动顺序:                              │  │     │
│  │  │  1. ParseCommandLine   5. InitializeQmlEngine        │  │     │
│  │  │  2. InitLogger         6. InitializeAppSystem        │  │     │
│  │  │  3. LoadConfiguration  7. Execute() → exec()         │  │     │
│  │  │  4. RegisterServices   8. Shutdown()                 │  │     │
│  │  └─────────────────────────────────────────────────────┘  │     │
│  └───────────────────────────────────────────────────────────┘     │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────┐     │
│  │  CServiceLocator (全局服务定位器 / 单例)                    │     │
│  │  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐       │     │
│  │  │ NoteCache ✓  │ │ NoteService ✓│ │ DataSaver  ✓ │       │     │
│  │  │ NoteCollect ✓│ │              │ │              │       │     │
│  │  └──────────────┘ └──────────────┘ └──────────────┘       │     │
│  └───────────────────────────────────────────────────────────┘     │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────┐     │
│  │  CNoteDataService (应用服务 / 数据转换与分发)               │     │
│  │  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐       │     │
│  │  │ AddNote()    │ │ LoadFromBuf │ │ Invalidate() │       │     │
│  │  │ UI→存储格式  │ │ 文件→缓存   │ │ 失效缓存     │       │     │
│  │  └──────────────┘ └──────────────┘ └──────────────┘       │     │
│  └───────────────────────────────────────────────────────────┘     │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────┐     │
│  │  CAppSystem (系统生命周期 / 线程调度)                       │     │
│  │  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐       │     │
│  │  │ IniAppFrame  │ │ CreateThread│ │ DoSecEvent   │       │     │
│  │  │ 初始化框架   │ │ 创建线程池  │ │ 秒级事件     │       │     │
│  │  └──────────────┘ └──────────────┘ └──────────────┘       │     │
│  └───────────────────────────────────────────────────────────┘     │
└──────────────────────────────────┬──────────────────────────────────┘
                                   │ 依赖 (通过接口/ServiceLocator)
                                   ▼
┌─────────────────────────────────────────────────────────────────────┐
│                     ⭐ 领域层 (Domain Core) ⭐                        │
│               实体 / 值对象 / 枚举 / 端口接口                         │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────┐     │
│  │  Services/Base/  (核心数据类型)                             │     │
│  │  ┌──────────────────┐ ┌──────────────────┐                 │     │
│  │  │ ST_NOTE_DATA     │ │ NOTE_MODEL_ITEM  │                 │     │
│  │  │ (存储结构体)      │ │ (UI模型项)       │                 │     │
│  │  │ m_s64NoteId      │ │ m_s64NoteId      │                 │     │
│  │  │ m_szContent[4096]│ │ m_strContent     │                 │     │
│  │  │ m_eNoteLevel     │ │ m_eNoteLevel     │                 │     │
│  │  │ m_s64WriteTime   │ │ m_s64WriteTime   │                 │     │
│  │  │ ...              │ │ ...              │                 │     │
│  │  └──────────────────┘ └──────────────────┘                 │     │
│  │                                                             │     │
│  │  ┌──────────────────┐ ┌──────────────────┐                 │     │
│  │  │ 枚举定义          │ │ 常量定义          │                 │     │
│  │  │ E_NOTE_EVENT_TYPE│ │ CONTENT_LENGTH_  │                 │     │
│  │  │ E_NOTE_WAKEUP_   │ │ MAX = 4096       │                 │     │
│  │  │ LEVEL            │ │ NOTE_BUFFER_ITEM │                 │     │
│  │  │ E_NOTE_TIME_SPAN │ │ _COUNT_MAX = 128 │                 │     │
│  │  └──────────────────┘ └──────────────────┘                 │     │
│  └───────────────────────────────────────────────────────────┘     │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────┐     │
│  │  🔌 端口接口 (Ports / 六边形端口)                            │     │
│  │  ┌──────────────────────────────────────────────────────┐  │     │
│  │  │  INoteDataBuffer (缓冲区端口)                         │  │     │
│  │  │  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐ │  │     │
│  │  │  │ AppendData() │ │ ReadBuffer() │ │ HasData()    │ │  │     │
│  │  │  │ Model→Bytes  │ │ Bytes→Buffer │ │ 检查未读数据 │ │  │     │
│  │  │  └──────────────┘ └──────────────┘ └──────────────┘ │  │     │
│  │  └──────────────────────────────────────────────────────┘  │     │
│  │                                                             │     │
│  │  ┌──────────────────────────────────────────────────────┐  │     │
│  │  │  CALLBACK_NOTEDATALOAD (回调端口)                     │  │     │
│  │  │  function<void(array<ST_NOTE_DATA>, count)>           │  │     │
│  │  └──────────────────────────────────────────────────────┘  │     │
│  │                                                             │     │
│  │  ┌──────────────────────────────────────────────────────┐  │     │
│  │  │  CThreadHandler (线程处理器抽象)                       │  │     │
│  │  │  ┌──────────────┐ ┌──────────────┐                   │  │     │
│  │  │  │ HandleTask() │ │ AttachThread│                   │  │     │
│  │  │  │ 处理任务     │ │ 绑定线程    │                   │  │     │
│  │  │  └──────────────┘ └──────────────┘                   │  │     │
│  │  └──────────────────────────────────────────────────────┘  │     │
│  └───────────────────────────────────────────────────────────┘     │
└──────────────────────────────────┬──────────────────────────────────┘
                                   │ 实现 (implements)
                                   ▼
┌─────────────────────────────────────────────────────────────────────┐
│                     🏗️ 基础设施层 (Infrastructure)                    │
│                   文件IO / 缓存 / 线程 / 日志                         │
│                                                                     │
│  ┌──────────────────────┐  ┌──────────────────────┐                 │
│  │ 📁 持久化 (Persistence)│  │ 💾 缓存 (Cache)       │                 │
│  │                      │  │                      │                 │
│  │ CDataRWMgr           │  │ CNoteDataCache       │                 │
│  │ ┌──────────────────┐ │  │ ┌──────────────────┐ │                 │
│  │ │ WriteToFile()    │ │  │ │ InvalidateCache()│ │                 │
│  │ │ ReadFromFile()   │ │  │ │ SaveNoteData()   │ │                 │
│  │ │ (纯字节IO)       │ │  │ │ GetNoteData()    │ │                 │
│  │ └──────────────────┘ │  │ │ (Copy-on-Write)  │ │                 │
│  │                      │  │ └──────────────────┘ │                 │
│  │ CDataSave            │  │                      │                 │
│  │ ┌──────────────────┐ │  │ CNoteDataCollect     │                 │
│  │ │ ReadFileData()   │ │  │ ┌──────────────────┐ │                 │
│  │ │ WriteFileData()  │ │  │ │ AppendData()     │ │                 │
│  │ │ (文件头+数据区)  │ │  │ │ ReadBuffer()     │ │                 │
│  │ └──────────────────┘ │  │ │ (实现INoteDataBuf │ │                 │
│  │                      │  │ │  fer接口)        │ │                 │
│  │ NOTE_FILE_HEADER     │  │ └──────────────────┘ │                 │
│  │ ┌──────────────────┐ │  │                      │                 │
│  │ │ m_s64InitID      │ │  │ DDataCache           │                 │
│  │ │ m_s64FileID      │ │  │ ┌──────────────────┐ │                 │
│  │ │ m_s64FileType    │ │  │ │ MAX_CACHE_SIZE   │ │                 │
│  │ └──────────────────┘ │  │ │ = 100            │ │                 │
│  └──────────────────────┘  │ └──────────────────┘ │                 │
│                             └──────────────────────┘                 │
│                                                                     │
│  ┌──────────────────────┐  ┌──────────────────────┐                 │
│  │ 🧵 线程 (Threading)   │  │ 📝 日志 (Logging)     │                 │
│  │                      │  │                      │                 │
│  │ CThread (QThread)    │  │ CLogThreadHandler    │                 │
│  │ ┌──────────────────┐ │  │ ┌──────────────────┐ │                 │
│  │ │ counting_semaphore│ │  │ │ LogInit()        │ │                 │
│  │ │ WakeUp()          │ │  │ │ AddTask()        │ │                 │
│  │ │ TryExit()         │ │  │ │ HandleTask()     │ │                 │
│  │ └──────────────────┘ │  │ │ LogWrite2File()  │ │                 │
│  │                      │  │ │ (fprintf stderr)  │ │                 │
│  │ CThreadFactory       │  │ └──────────────────┘ │                 │
│  │ ┌──────────────────┐ │  │                      │                 │
│  │ │ CreateThread()   │ │  │ CDynsDataSaveThread  │                 │
│  │ │ InitThreadHanders│ │  │ Handler (数据保存)   │                 │
│  │ └──────────────────┘ │  │                      │                 │
│  │                      │  │ CSystemThreadHandler │                 │
│  │ DThread (线程定义)   │  │ (系统事件)           │                 │
│  └──────────────────────┘  └──────────────────────┘                 │
│                                                                     │
│  ┌──────────────────────────────────────────────────────────┐      │
│  │ ⚙️ 业务配置 (Business Config)                              │      │
│  │ CCommonConfig / DConfig                                    │      │
│  └──────────────────────────────────────────────────────────┘      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 二、六边形端口-适配器视角

```
┌─────────────────────────────────────────────────────────────────────┐
│                         外部适配器 (Adapters)                         │
│                                                                     │
│   ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐     │
│   │  QML Adapter    │  │  File Adapter   │  │  Console Adapter│     │
│   │  (表现层适配器)  │  │  (持久化适配器)  │  │  (日志适配器)   │     │
│   │                 │  │                 │  │                 │     │
│   │ QQmlAppEngine   │  │ CDataRWMgr      │  │ LogMessageHandle│     │
│   │ + contextProperty│  │ + CDataSave     │  │ r + fprintf    │     │
│   │ + QML bindings  │  │ + fstream       │  │ (stderr)       │     │
│   └────────┬────────┘  └────────┬────────┘  └────────┬────────┘     │
│            │                    │                    │               │
│   用户输入 │                    │ 文件IO             │ 日志输出       │
│            ▼                    ▼                    ▼               │
│   ┌─────────────────────────────────────────────────────────────┐   │
│   │                    🔌 端口 (Ports)                           │   │
│   │                                                             │   │
│   │  ┌──────────────┐ ┌──────────────┐ ┌──────────────────────┐ │   │
│   │  │ INoteDataBuf │ │ CALLBACK_    │ │ CThreadHandler      │ │   │
│   │  │ fer          │ │ NOTEDATALOAD │ │ (抽象线程处理器)     │ │   │
│   │  └──────────────┘ └──────────────┘ └──────────────────────┘ │   │
│   └─────────────────────────────────────────────────────────────┘   │
│                             │                                       │
│                     ┌───────┴───────┐                               │
│                     ▼               ▼                               │
│   ┌─────────────────────┐ ┌─────────────────────┐                   │
│   │  CNoteDataService   │ │  CAppSystem         │                   │
│   │  (应用层核心)        │ │  (系统调度)          │                   │
│   │                     │ │                     │                   │
│   │  • AddNote()        │ │  • IniAppFrame()    │                   │
│   │  • LoadFromBuffer() │ │  • CreateThread()   │                   │
│   │  • InvalidateCache()│ │  • DoSecEvent()     │                   │
│   └─────────────────────┘ └─────────────────────┘                   │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 三、数据流 (Data Flow)

### 写入流程 (User Input → File)

```
  QML 用户输入
      │
      ▼
  CMainNoteListViewModel.setData()
      │
      ▼
  CNoteDataService.AddNote()          ← 应用层
      │
      ├──→ CNoteDataCache (缓存)      ← 基础设施层
      │    SaveNoteData()               (COW 模式)
      │
      └──→ CNoteDataCollect           ← 基础设施层
           AppendData()                 (待写队列)
               │
               ▼
           INoteDataBuffer 接口         ← 领域层端口
               │
               ▼
           CDynsDataSaveThreadHandler  ← 基础设施层
           HandleTask()
               │
               ▼
           CDataRWMgr.WriteToFile()    ← 基础设施层
               │
               ▼
           CDataSave.WriteFileData()   ← 基础设施层
               │
               ▼
           std::fstream ──→ 文件系统    ← 外部世界
```

### 启动读取流程 (Startup Read)

```
  CAppSystem::IniAppFrame()
      │
      ▼
  CThreadFactory::CreateThread()
      │
      ▼
  CDynsDataSaveThreadHandler::HandleTask()
      │
      ▼
  CDataRWMgr::ReadFromFile()           ← 基础设施层
      │
      ▼
  CNoteDataService::LoadFromBuffer()   ← 应用层
      │
      ▼
  CNoteDataCache::SaveNoteData()       ← 基础设施层
      │
      ▼
  CALLBACK_NOTEDATALOAD 回调            ← 领域层端口
      │
      ▼
  CMainNoteListViewModel::PutArrNoteData()  ← 表现层
      │
      ▼
  beginResetModel() / endResetModel()  ← 通知 QML 刷新
```

---

## 四、文件目录映射

| 目录 | 洋葱层 | 核心类 |
|------|--------|--------|
| `Services/Base/` | 领域层 | `ST_NOTE_DATA`, `NOTE_MODEL_ITEM`, `INoteDataBuffer`, 枚举, 常量 |
| `Services/Module/Adapter/DataManager/` | 应用层 | `CNoteDataService`, `CServiceLocator` |
| `Services/Module/Adapter/System/` | 应用层 | `CAppSystem` |
| `Services/Module/BusinessModule/` | 应用层 | `CNoteBusiness`, `CCommonConfig` |
| `Services/Module/Other/DataCache/` | 基础设施层 | `CNoteDataCache`, `CNoteDataCollect`, `DDataCache` |
| `Services/Module/Other/DataReadWriter/` | 基础设施层 | `CDataRWMgr`, `CDataSave` |
| `Services/Module/Other/TaskThread/` | 基础设施层 | `CLogThreadHandler`, `CDynsDataSaveThreadHandler` |
| `Services/Module/Other/Factory/` | 基础设施层 | `CThreadFactory` |
| `Services/WidgetServices/` | 表现层 | `CMainNoteListViewModel` |
| `Widgets/` | 表现层 | QML 文件 |
| `Bootstrapper/` | 组合根 | `CAppBootstrapper` |

---

## 五、当前架构问题标记

| 问题 | 严重程度 | 说明 |
|------|----------|------|
| **跨线程回调** | ⚠ 高 | `PutArrNoteData()` 在数据线程被调用，直接操作 `QAbstractItemModel` |
| **领域层薄弱** | ⚠ 中 | `CNoteBusiness` 是空壳，`ST_NOTE_DATA` / `NOTE_MODEL_ITEM` 是 C 风格结构体，缺值对象封装 |
| **ServiceLocator 反射** | ⚠ 中 | 全局单例，依赖方向不明确，缺端口接口隔离 |
| **领域/基础设施混杂** | ⚠ 低 | `DDataCache` 定义在 `Base/` 但实际是基础设施层概念 |
| **日志线程 QFile 跨线程** | ⚠ 高 | `QFile` 在主线程创建，日志线程使用，未定义行为 |
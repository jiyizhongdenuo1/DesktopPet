# Composition Root & Bootstrapper - 组合根详细设计

## 🧅 洋葱架构：组合根（Composition Root）

### 设计原则
- **单一入口**：整个应用只有一个地方创建对象
- **依赖组装**：集中管理所有依赖关系
- **生命周期**：控制对象的创建和销毁顺序
- **配置驱动**：根据配置选择具体实现

---

## 1. Composition Root 核心设计

### 1.1 完整的组合根结构

```cpp
// Bootstrapper/CompositionRoot.h
#pragma once
#include <memory>
#include <vector>

// 前向声明所有层
namespace Domain::Note { class INoteRepository; }
namespace Infrastructure::Persistence { class FileNoteRepository; }
namespace Infrastructure::Cache { class InMemoryNoteCache; }
namespace Application::UseCases {
    class AddNoteUseCase;
    class GetNotesUseCase;
    class DeleteNoteUseCase;
}
namespace Presentation::ViewModels { class MainNoteListViewModel; }

namespace Bootstrapper {

class CompositionRoot {
public:
    // 单例访问（可选，也可以不用单例）
    static CompositionRoot& instance();

    // 初始化和清理
    void initialize();
    void shutdown();

    // 获取各层组件（供 Presentation 层使用）
    template<typename T>
    std::shared_ptr<T> resolve();

private:
    CompositionRoot() = default;
    ~CompositionRoot() = default;

    // 禁止拷贝和赋值
    CompositionRoot(const CompositionRoot&) = delete;
    CompositionRoot& operator=(const CompositionRoot&) = delete;

    // 分层创建方法
    void createInfrastructureLayer();
    void createApplicationLayer();
    void createPresentationLayer();

    // 存储所有创建的对象
    struct Services {
        // Domain 层接口（由 Infrastructure 实现）
        std::shared_ptr<Domain::Note::INoteRepository> noteRepository;

        // Infrastructure 层
        std::shared_ptr<Infrastructure::Cache::InMemoryNoteCache> noteCache;

        // Application 层
        std::shared_ptr<Application::UseCases::AddNoteUseCase> addNoteUC;
        std::shared_ptr<Application::UseCases::GetNotesUseCase> getNotesUC;
        std::shared_ptr<Application::UseCases::DeleteNoteUseCase> deleteNoteUC;

        // Presentation 层
        std::unique_ptr<Presentation::ViewModels::MainNoteListViewModel> mainNoteListVM;
    } m_services;

    bool m_initialized = false;
};

// 便捷宏（可选）
#define GET_SERVICE(type) Bootstrapper::CompositionRoot::instance().resolve<type>()

} // namespace Bootstrapper
```

### 1.2 完整实现

```cpp
// Bootstrapper/CompositionRoot.cpp
#include "CompositionRoot.h"
#include "Domain/Note/INoteRepository.h"
#include "Infrastructure/Persistence/FileNoteRepository.h"
#include "Infrastructure/Persistence/NoteFileMapper.h"
#include "Infrastructure/Persistence/CDataRWMgr_Adapter.h"
#include "Infrastructure/Cache/InMemoryNoteCache.h"
#include "Application/UseCases/AddNoteUseCase.h"
#include "Application/UseCases/GetNotesUseCase.h"
#include "Application/UseCases/DeleteNoteUseCase.h"
#include "Presentation/ViewModels/MainNoteListViewModel.h"

using namespace Bootstrapper;

CompositionRoot& CompositionRoot::instance() {
    static CompositionRoot instance;
    return instance;
}

void CompositionRoot::initialize() {
    if (m_initialized) return;

    try {
        std::cout << "[Composition Root] 开始初始化..." << std::endl;

        // 步骤1：创建基础设施层（最底层）
        createInfrastructureLayer();

        // 步骤2：创建应用层（依赖基础设施层）
        createApplicationLayer();

        // 步骤3：创建表现层（依赖应用层）
        createPresentationLayer();

        m_initialized = true;
        std::cout << "[Composition Root] 初始化完成！" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[Composition Root] 初始化失败: " << e.what() << std::endl;
        shutdown();  // 清理已创建的资源
        throw;
    }
}

void CompositionRoot::shutdown() {
    if (!m_initialized) return;

    std::cout << "[Composition Root] 开始清理..." << std::endl;

    // 销毁顺序：与创建顺序相反
    // 3. 销毁表现层
    m_services.mainNoteListVM.reset();

    // 2. 销毁应用层
    m_services.addNoteUC.reset();
    m_services.getNotesUC.reset();
    m_services.deleteNoteUC.reset();

    // 1. 销毁基础设施层
    m_services.noteCache.reset();
    m_services.noteRepository.reset();

    m_initialized = false;
    std::cout << "[Composition Root] 清理完成！" << std::endl;
}

void CompositionRoot::createInfrastructureLayer() {
    std::cout << "  [1/3] 创建基础设施层..." << std::endl;

    // 1.1 文件管理器
    auto fileManager = std::make_shared<CDataRWMgr>("notes.dat");

    // 1.2 对象映射器
    auto mapper = std::make_shared<NoteFileMapper>();

    // 1.3 文件仓储（实现 INoteRepository 接口）
    auto fileRepo = std::make_shared<FileNoteRepository>(
        "notes.dat",
        fileManager,
        mapper
    );

    // 1.4 缓存组件
    auto cache = std::make_shared<InMemoryNoteCache>(500);  // 最大500条

    // 可选：使用装饰器模式添加缓存功能
    /*
    auto cachedRepo = std::make_shared<CachedNoteRepository>(
        fileRepo,
        cache
    );
    m_services.noteRepository = cachedRepo;
    */

    // 或者：直接使用文件仓储（缓存由 Use Case 层管理）
    m_services.noteRepository = fileRepo;
    m_services.noteCache = cache;

    std::cout << "      ✓ 文件仓储创建完成" << std::endl;
    std::cout << "      ✓ 内存缓存创建完成 (容量: 500)" << std::endl;
}

void CompositionRoot::createApplicationLayer() {
    std::cout << "  [2/3] 创建应用层..." << std::endl;

    // 2.1 创建用例（注入仓储依赖）
    m_services.addNoteUC = std::make_shared<Application::UseCases::AddNoteUseCase>(
        m_services.noteRepository
    );

    m_services.getNotesUC = std::make_shared<Application::UseCases::GetNotesUseCase>(
        m_services.noteRepository
    );

    m_services.deleteNoteUC = std::make_shared<Application::UseCases::DeleteNoteUseCase>(
        m_services.noteRepository
    );

    std::cout << "      ✓ AddNoteUseCase 创建完成" << std::endl;
    std::cout << "      ✓ GetNotesUseCase 创建完成" << std::endl;
    std::cout << "      ✓ DeleteNoteUseCase 创建完成" << std::endl;
}

void CompositionRoot::createPresentationLayer() {
    std::cout << "  [3/3] 创建表现层..." << std::endl;

    // 3.1 创建 ViewModel（注入 Use Cases）
    m_services.mainNoteListVM = std::make_unique<Presentation::ViewModels::MainNoteListViewModel>(
        m_services.getNotesUC,
        m_services.addNoteUC,
        m_services.deleteNoteUC
    );

    std::cout << "      ✓ MainNoteListViewModel 创建完成" << std::endl;
}

template<typename T>
std::shared_ptr<T> CompositionRoot::resolve() {
    // 类型安全的解析
    if constexpr (std::is_same_v<T, Domain::Note::INoteRepository>) {
        return m_services.noteRepository;
    } else if constexpr (std::is_same_v<T, Application::UseCases::AddNoteUseCase>) {
        return m_services.addNoteUC;
    } else if constexpr (std::is_same_v<T, Application::UseCases::GetNotesUseCase>) {
        return m_services.getNotesUC;
    } else if constexpr (std::is_same_v<T, Application::UseCases::DeleteNoteUseCase>) {
        return m_services.deleteNoteUC;
    } else if constexpr (std::is_same_v<T, Infrastructure::Cache::InMemoryNoteCache>) {
        return m_services.noteCache;
    } else {
        throw std::runtime_error("Unknown type requested from Composition Root");
    }
}
```

---

## 2. 与 Qt/QML 集成

### 2.1 改造后的 CAppBootstrapper

```cpp
// Bootstrapper/CAppBootstrapper.h (重构后)
#pragma once
#include <memory>

class QQmlApplicationEngine;
class QApplication;

namespace Bootstrapper {

class CAppBootstrapper {
public:
    static CAppBootstrapper& Instance();

    int Run(int argc, char* argv[]);

private:
    CAppBootstrapper() = default;
    ~CAppBootstrapper() = default;

    // 禁止拷贝
    CAppBootstrapper(const CAppBootstrapper&) = delete;

    // 初始化步骤
    void ParseCommandLine(int argc, char* argv[]);
    void InitLogger();
    void LoadConfiguration();
    void InitializeQtApplication();
    void InitializeQmlEngine();

    // 新增：初始化组合根
    void InitializeCompositionRoot();

    // 执行主循环
    int Execute();
    void Shutdown();

    // 成员变量
    std::unique_ptr<QApplication> m_pQtApp;
    std::unique_ptr<QQmlApplicationEngine> m_pEngine;
};

#define g_CAppBootstrapper CAppBootstrapper::Instance()

} // namespace Bootstrapper
```

### 2.2 实现细节

```cpp
// Bootstrapper/CAppBootstrapper.cpp (重构后)
#include "CAppBootstrapper.h"
#include "CompositionRoot.h"
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

using namespace Bootstrapper;

CAppBootstrapper& CAppBootstrapper::Instance() {
    static CAppBootstrapper instance;
    return instance;
}

int CAppBootstrapper::Run(int argc, char* argv[]) {
    try {
        ParseCommandLine(argc, argv);
        InitLogger();
        LoadConfiguration();
        InitializeQtApplication();
        InitializeCompositionRoot();  // ⭐ 新增：初始化组合根
        InitializeQmlEngine();
        return Execute();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return -1;
    }
}

void CAppBootstrapper::InitializeCompositionRoot() {
    std::cout << "初始化组合根..." << std::endl;

    // 初始化组合根（创建所有服务）
    CompositionRoot::instance().initialize();

    // 将 ViewModel 注册到 QML 引擎
    auto& root = CompositionRoot::instance();
    auto viewModel = root.resolve<Presentation::ViewModels::MainNoteListViewModel>();

    m_pEngine->rootContext()->setContextProperty(
        "noteModel",
        viewModel.get()
    );

    std::cout << "组合根初始化完成！" << std::endl;
}

int CAppBootstrapper::Execute() {
    std::cout << "启动应用程序..." << std::endl;
    return m_pQtApp->exec();
}

void CAppBootstrapper::Shutdown() {
    std::cout << "关闭应用程序..." << std::endl;

    // 清理 QML 引擎
    m_pEngine.reset();

    // 清理组合根（按正确顺序销毁所有对象）
    CompositionRoot::instance().shutdown();

    // 清理 Qt 应用
    m_pQtApp.reset();
}
```

---

## 3. 配置管理（可扩展性）

### 3.1 AppConfig - 应用配置

```cpp
// Bootstrapper/AppConfig.h
#pragma once
#include <string>
#include <optional>

namespace Bootstrapper {

struct PersistenceConfig {
    enum class StorageType { FILE_SYSTEM, SQLITE_DATABASE, MEMORY };

    StorageType storageType = StorageType::FILE_SYSTEM;
    std::string filePath = "notes.dat";
    std::string databasePath = "notes.db";
    size_t maxCacheSize = 500;
};

struct UIConfig {
    bool enableAnimations = true;
    QString theme = "light";
    int windowWidth = 800;
    int windowHeight = 600;
};

struct LoggingConfig {
    enum class LogLevel { DEBUG, INFO, WARNING, ERROR };
    LogLevel consoleLevel = LogLevel::INFO;
    LogLevel fileLevel = LogLevel::DEBUG;
    std::string logFilePath = "desktoppet.log";
};

struct AppConfig {
    PersistenceConfig persistence;
    UIClassConfig ui;
    LoggingConfig logging;

    // 从文件加载配置
    static AppConfig loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;

    // 命令行参数覆盖
    void applyCommandLineOverrides(int argc, char* argv[]);
};

} // namespace Bootstrapper
```

### 3.2 使用配置的 Composition Root

```cpp
void CompositionRoot::initialize(const AppConfig& config) {
    // 根据配置选择实现
    switch (config.persistence.storageType) {
        case PersistenceConfig::StorageType::FILE_SYSTEM:
            m_services.noteRepository = createFileRepository(config);
            break;
        case PersistenceConfig::StorageType::SQLITE_DATABASE:
            m_services.noteRepository = createDatabaseRepository(config);
            break;
        case PersistenceConfig::StorageType::MEMORY:
            m_services.noteRepository = createInMemoryRepository();
            break;
    }

    m_services.noteCache = std::make_shared<InMemoryNoteCache>(
        config.persistence.maxCacheSize
    );
    // ... 其他初始化 ...
}
```

---

## 4. 依赖图与对象生命周期

### 4.1 完整依赖图

```
┌─────────────────────────────────────────────────────────────┐
│                   Composition Root                          │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              创建顺序（从下到上）                    │   │
│  │                                                     │   │
│  │  Layer 4: Presentation                               │   │
│  │  ┌───────────────────────────────────────────┐     │   │
│  │  │ MainNoteListViewModel                     │     │   │
│  │  │   ↑ 注入                                   │     │   │
│  │  │   ├─ GetNotesUseCase                       │     │   │
│  │  │   ├─ AddNoteUseCase                        │     │   │
│  │  │   └─ DeleteNoteUseCase                      │     │   │
│  │  └───────────────────────────────────────────┘     │   │
│  │           ↕ 注入                                    │   │
│  │  Layer 3: Application                              │   │
│  │  ┌───────────────────────────────────────────┐     │   │
│  │  │ AddNoteUseCase / GetNotesUseCase / ...    │     │   │
│  │  │   ↑ 注入                                   │     │   │
│  │  │   └─ INoteRepository (接口)                │     │   │
│  │  └───────────────────────────────────────────┘     │   │
│  │           ↕ 实现                                   │   │
│  │  Layer 2: Infrastructure                           │   │
│  │  ┌───────────────────────────────────────────┐     │   │
│  │  │ FileNoteRepository                         │     │   │
│  │  │   ↑ 包含                                   │     │   │
│  │  │   ├─ NoteFileMapper                        │     │   │
│  │  │   ├─ FileManagerAdapter                    │     │   │
│  │  │   └─ InMemoryNoteCache                     │     │   │
│  │  └───────────────────────────────────────────┘     │   │
│  │                                                     │   │
│  │  Layer 1: External Dependencies                    │   │
│  │  ┌───────────────────────────────────────────┐     │   │
│  │  │ • File System (OS)                         │     │   │
│  │  │ • SQLite / Database Library               │     │   │
│  │  │ • System Clock                             │     │   │
│  │  │ • Qt Framework                             │     │   │
│  │  └───────────────────────────────────────────┘     │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
│  销毁顺序（从上到下）：                                     │
│  Presentation → Application → Infrastructure → External    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 对象生命周期时间线

```
Time →

T=0ms    main()
         │
         ▼
T=10ms   CAppBootstrapper::Run()
         │
         ├── ParseCommandLine()
         ├── InitLogger()
         ├── LoadConfiguration()
         │
         ▼
T=50ms   InitializeCompositionRoot()  ⭐ 核心
         │
         ├── [1/3] createInfrastructureLayer()
         │   ├── new CDataRWMgr()
         │   ├── new NoteFileMapper()
         │   ├── new FileNoteRepository(rwMgr, mapper)
         │   └── new InMemoryNoteCache(500)
         │
         ├── [2/3] createApplicationLayer()
         │   ├── new AddNoteUseCase(repository)
         │   ├── new GetNotesUseCase(repository)
         │   └── new DeleteNoteUseCase(repository)
         │
         ├── [3/3] createPresentationLayer()
         │   └── new MainNoteListViewModel(getUC, addUC, delUC)
         │
         ▼
T=100ms  InitializeQmlEngine()
         │
         ├── setContextProperty("noteModel", viewModel)
         ├── load(QUrl("qrc:/MainWidget.qml"))
         │
         ▼
T=150ms  Execute() → app.exec()
         │
         ════════════════════════════
         应用运行中...
         ════════════════════════════
         │
         用户交互：
         • 点击"添加笔记"
           → ViewModel.addNote()
             → AddNoteUseCase.execute()
               → FileNoteRepository.save()
                 → 写入 notes.dat ✅
         │
         ▼
用户关闭窗口或调用 quit()
         │
         ▼
T=????   Shutdown()
         │
         ├── reset(mainNoteListVM)       ← 销毁 ViewModel
         ├── reset(addNoteUC)            ← 销毁 Use Cases
         ├── reset(getNotesUC)
         ├── reset(deleteNoteUC)
         ├── reset(noteCache)            ← 销毁 Cache
         ├── reset(noteRepository)       ← 销毁 Repository
         │
         ├── engine.reset()              ← 销毁 QML Engine
         └── qtApp.reset()              ← 销毁 QApplication
```

---

## 5. 高级特性

### 5.1 条件化注册（基于配置）

```cpp
void CompositionRoot::initialize(const AppConfig& config) {
    // 日志系统
    if (config.logging.fileLevel != LoggingConfig::LogLevel::DISABLED) {
        m_services.logger = make_shared<FileLogger>(config.logging.logFilePath);
    } else {
        m_services.logger = make_shared<NullLogger>();  // 空对象模式
    }

    // 时间服务（测试时可替换为固定时间）
    #ifdef TESTING
    m_services.clock = make_shared<TestClockAdapter>(fixedTestTime);
    #else
    m_services.clock = make_shared<SystemClockAdapter>();
    #endif

    // 持久化方式
    switch (config.persistence.storageType) {
        case FILE_SYSTEM:
            registerFilePersistence(config);
            break;
        case DATABASE:
            registerDatabasePersistence(config);
            break;
    }
}
```

### 5.2 懒加载（Lazy Initialization）

```cpp
template<typename T>
std::shared_ptr<T> CompositionRoot::resolve() {
    // 如果还没创建，则延迟创建
    if constexpr (std::is_same_v<T, SomeExpensiveService>) {
        if (!m_services.expensiveService) {
            std::cout << "[Lazy] Creating ExpensiveService on first access..." << std::endl;
            m_services.expensiveService = createExpensiveService();
        }
        return m_services.expensiveService;
    }
    // 其他类型正常返回...
}
```

### 5.3 作用域管理（Scope）

```cpp
enum class ServiceScope {
    SINGLETON,    // 全局唯一实例（默认）
    TRANSIENT,    // 每次请求新实例
    SCOPED       // 在特定作用域内唯一
};

template<typename T, ServiceScope Scope = ServiceScope::SINGLETON>
std::shared_ptr<T> CompositionRoot::resolve() {
    if constexpr (Scope == ServiceScope::TRANSIENT) {
        // 每次都创建新实例
        return make_shared<T>(/* dependencies */);
    } else {
        // 返回缓存的实例
        return getCachedInstance<T>();
    }
}
```

---

## 6. 测试支持

### 6.1 测试替身（Test Doubles）工厂

```cpp
// TestCode/TestCompositionRoot.h
#pragma once
#include "Bootstrapper/CompositionRoot.h"
#include <gmock/gmock.h>

using ::testing::NiceMock;
using ::testing::Mock;

class TestCompositionRoot {
public:
    static TestCompositionRoot& instance() {
        static TestCompositionRoot inst;
        return inst;
    }

    void initializeForTesting() {
        // 用 Mock 替换真实实现
        root.m_services.noteRepository =
            make_shared<NiceMock<MockNoteRepository>>();

        root.m_services.noteCache =
            make_shared<InMemoryNoteCache>(50);  // 测试时用小容量

        root.m_services.addNoteUC =
            make_shared<AddNoteUseCase>(root.m_services.noteRepository);

        // ... 其他 mock 设置 ...
    }

    MockNoteRepository& getMockRepository() {
        return static_cast<MockNoteRepository&>(*root.m_services.noteRepository);
    }

private:
    Bootstrapper::CompositionRoot root;
};

// 便捷宏
#define TEST_ROOT TestCompositionRoot::instance()
```

### 6.2 测试示例

```cpp
// TestCode/TestFullIntegration.cpp
#include <gtest/gtest.h>
#include "TestCompositionRoot.h"

TEST(IntegrationTest, FullWorkflow_AddAndRetrieve) {
    // Arrange
    TEST_ROOT.initializeForTesting();
    auto& mockRepo = TEST_ROOT.getMockRepository();

    EXPECT_CALL(mockRepo, save(_))
        .Times(1);

    EXPECT_CALL(mockRepo, findAll())
        .WillOnce(Return(std::vector<Note>{
            Note(NoteId(1), NoteContent::create("Test"))
        }));

    // Act
    auto vm = TEST_ROOT.root.resolve<MainNoteListViewModel>();
    vm->addNote("Test Note");
    auto response = vm->getAllNotes();

    // Assert
    ASSERT_EQ(response.size(), 1);
    EXPECT_EQ(response[0].getContent().getText(), "Test");
}
```

---

## 7. 与当前代码的迁移计划

| 当前组件 | 目标位置 | 迁移动作 | 优先级 |
|---------|----------|----------|--------|
| CAppBootstrapper | → CompositionRoot | 重构为纯组合根 | P0 (最高) |
| RegisterServices() | → CompositionRoot::initialize() | 移入分层创建逻辑 | P0 |
| ServiceRegistry 结构体 | → CompositionRoot::Services | 保持但重命名 | P1 |
| 直接 new 对象 | → 通过 CompositionRoot 创建 | 统一管理 | P1 |

### 迁移步骤

#### Phase 1: 创建 CompositionRoot（不破坏现有代码）
```cpp
// 保留旧的 Bootstrapper，新增 CompositionRoot
class CAppBootstrapper {
    void Run(...) {
        // ... 现有逻辑 ...

        // 新增：同时初始化新的组合根
        CompositionRoot::instance().initialize();

        // ... 继续现有逻辑 ...
    }
};
```

#### Phase 2: 逐步迁移
1. 先迁移一个简单的服务（如 Logger）
2. 验证无误后迁移核心服务（Repository）
3. 最后迁移 UI 相关（ViewModel）

#### Phase 3: 清理旧代码
1. 标记旧方法为 @Deprecated
2. 更新所有调用点
3. 最终删除旧实现

---

## ⚠️ 常见陷阱

### 陷阱1：循环依赖

❌ 错误：
```cpp
void createA() { m_services.b = createB(); }  // A 需要 B
void createB() { m_services.a = createA(); }  // B 需要 A → 循环！
```

✅ 解决方案：
```cpp
// 方案1：重新设计，打破循环
// 方案2：使用接口 + 延迟绑定
void createA() { /* 不在这里创建 B */ }
void createB() { /* 不在这里创建 A */ }
void wireDependencies() {
    m_services.a->setB(m_services.b);  // 都创建后再连接
}
```

### 陷阱2：忘记注册到 QML

❌ 错误：
```cpp
auto vm = createViewModel();
// 忘记: engine->rootContext()->setContextProperty("noteModel", vm.get());
// 结果：QML 中无法访问 noteModel
```

✅ 正确：
```cpp
auto vm = createViewModel();
engine->rootContext()->setContextProperty("noteModel", vm.get());
```

### 陷阱3：线程安全问题

❌ 错误：
```cpp
// 多个线程同时调用 instance()
static CompositionRoot instance;  // 不是线程安全的！
```

✅ 正确：
```cpp
// 使用 Meyers' Singleton (C++11保证线程安全)
static CompositionRoot& instance() {
    static CompositionRoot inst;  // ✅ C++11 保证 magic statics 线程安全
    return inst;
}
```

---

## 🎯 下一步行动

1. ✅ 创建 `Bootstrapper/CompositionRoot.h/cpp`
2. ✅ 实现分层创建逻辑（Infrastructure → Application → Presentation）
3. ✅ 改造 `CAppBootstrapper` 使用 `CompositionRoot`
4. ✅ 编写集成测试验证完整流程
5. ✅ 添加配置支持和条件化注册
6. 🎉 **完成洋葱架构的重构！**

---

## 📊 重构检查清单

在宣布重构完成前，确认以下所有项：

### 架构完整性
- [ ] 所有层都已创建并正确定义
- [ ] 依赖方向严格指向圆心（外→内）
- [ ] Domain 层零外部依赖
- [ ] Infrastructure 层实现了 Domain 定义的接口
- [ ] Application 层只编排，无业务规则
- [ ] Presentation 层只展示，委托给 Application

### 组合根正确性
- [ ] 只有一个 Composition Root
- [ ] 所有对象都在这里创建
- [ ] 创建顺序正确（底层→上层）
- [ ] 销毁顺序正确（上层→底层）
- [ ] 无循环依赖
- [ ] 线程安全（如果需要并发）

### 功能完整性
- [ ] 可以编译通过
- [ ] 可以正常运行
- [ ] 所有原有功能保持不变
- [ ] 新架构可以通过单元测试
- [ ] 集成测试覆盖主要流程

### 代码质量
- [ ] 无内存泄漏（使用智能指针）
- [ ] 无未处理的异常
- [ ] 日志输出清晰
- [ ] 代码注释完整
- [ ] 符合项目编码规范

---

**记住**：Composition Root 是"指挥官"，它**统一调度**整个应用的创建和销毁！🎖️
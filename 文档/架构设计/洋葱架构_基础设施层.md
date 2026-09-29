# Infrastructure Layer Design - 基础设施层详细设计

## 🧅 洋葱架构：基础设施层（Infrastructure Layer）

### 设计原则
- **实现细节**：实现 Domain 层定义的接口
- **可替换性**：可以随时更换实现（文件↔数据库↔云存储）
- **框架依赖**：可以依赖 Qt、文件系统、网络库等
- **不被依赖**：Domain/Application/Presentation 都不直接依赖它

---

## 1. 持久化（Persistence）⭐ 核心！

### 1.1 FileNoteRepository - 文件仓储实现

```cpp
// Infrastructure/Persistence/FileNoteRepository.h
#pragma once
#include "Domain/Note/INoteRepository.h"
#include <string>
#include <memory>

namespace Infrastructure::Persistence {

class NoteFileMapper;       // 对象映射器
class CDataRWMgr;        // 文件管理器

class FileNoteRepository : public Domain::Note::INoteRepository {
public:
    // 构造函数注入依赖
    explicit FileNoteRepository(
        const std::string& filePath,
        std::shared_ptr<CDataRWMgr> fileManager,
        std::shared_ptr<NoteFileMapper> mapper
    );

    // 实现 INoteRepository 接口
    void save(const Domain::Note::Note& note) override;
    void remove(const Domain::Note::NoteId& id) override;

    std::optional<Domain::Note::Note> findById(
        const Domain::Note::NoteId& id
    ) const override;

    std::vector<Domain::Note::Note> findAll() const override;
    std::vector<Domain::Note::Note> findCompleted() const override;
    std::vector<Domain::Note::Note> findActive() const override;

    size_t count() const override;
    bool exists(const Domain::Note::NoteId& id) const override;

private:
    std::string m_filePath;
    std::shared_ptr<CDataRWMgr> m_fileManager;
    std::shared_ptr<NoteFileMapper> m_mapper;

    // 内部辅助方法
    std::vector<char> readRawData() const;
    void writeRawData(const std::vector<char>& data);
};

} // namespace Infrastructure::Persistence
```

**实现示例**：

```cpp
// Infrastructure/Persistence/FileNoteRepository.cpp
#include "FileNoteRepository.h"
#include "NoteFileMapper.h"
#include "CDataRWMgr.h"
#include "Domain/Common/NotFoundException.h"

using namespace Infrastructure::Persistence;

void FileNoteRepository::save(const Domain::Note::Note& note) {
    // 步骤1：读取现有数据
    auto rawData = readRawData();

    // 步骤2：转换为领域对象列表
    auto existingNotes = m_mapper->toDomainList(rawData);

    // 步骤3：查找并更新或追加
    auto it = std::find_if(existingNotes.begin(), existingNotes.end(),
        [&note](const auto& n) { return n.getId() == note.getId(); });

    if (it != existingNotes.end()) {
        *it = note;  // 更新已存在的笔记
    } else {
        existingNotes.push_back(note);  // 添加新笔记
    }

    // 步骤4：转换回原始数据格式并写入
    auto newData = m_mapper->fromDomainList(existingNotes);
    writeRawData(newData);
}

std::optional<Domain::Note::Note> FileNoteRepository::findById(
    const Domain::Note::NoteId& id) const {

    auto allNotes = findAll();
    for (const auto& note : allNotes) {
        if (note.getId() == id) {
            return note;
        }
    }

    return std::nullopt;  // 未找到
}
```

**关键点**：
- ✅ **完全实现** `INoteRepository` 接口
- ✅ **委托**给 Mapper 和 FileManager
- ✅ **异常转换**：将 I/O 异常转为领域异常

---

### 1.2 NoteFileMapper - 对象映射器

```cpp
// Infrastructure/Persistence/NoteFileMapper.h
#pragma once
#include <vector>
#include <memory>

namespace Domain::Note { class Note; }
struct ST_NOTE_DATA;  // 现有的二进制结构体

namespace Infrastructure::Persistence {

class NoteFileMapper {
public:
    // 领域对象 → 二进制格式（用于写入文件）
    std::vector<ST_NOTE_DATA> fromDomainList(
        const std::vector<Domain::Note::Note>& notes
    ) const;

    ST_NOTE_DATA fromDomain(
        const Domain::Note::Note& note
    ) const;

    // 二进制格式 → 领域对象（用于从文件读取）
    std::vector<Domain::Note::Note> toDomainList(
        const std::vector<ST_NOTE_DATA>& rawData
    ) const;

    std::optional<Domain::Note::Note> toDomain(
        const ST_NOTE_DATA& rawData
    ) const;

private:
    // 字段映射逻辑
    Domain::Note::NoteId mapId(int64_t rawId) const;
    Domain::Note::NoteContent mapContent(const char* rawContent) const;
    // ... 其他字段映射
};

} // namespace Infrastructure::Persistence
```

**为什么需要 Mapper？**
- ✅ **解耦**：领域对象不知道文件格式
- ✅ **可测试**：可以单独测试映射逻辑
- ✅ **灵活**：改变文件格式只需修改 Mapper

**映射示例**：
```
Domain::Note (面向对象)
├── id: NoteId (值对象)
├── content: NoteContent (值对象，带验证)
├── completed: bool
├── deleted: bool
├── createTime: int64_t
└── modifyTime: int64_t
         ↕ 映射 (Mapper)
ST_NOTE_DATA (C风格结构体)
├── m_s64NoteId: INT64
├── m_szContent: char[CONTENT_LENGTH_MAX]
├── m_eNoteLevel: ENUM
├── m_s64WriteTime: INT64
├── ... 其他字段
```

---

### 1.3 CDataRWMgr - 文件管理器（复用现有代码）

```cpp
// Infrastructure/Persistence/CDataRWMgr_Adapter.h
#pragma once
// 复用现有的 CDataRWMgr，但包装为更清晰的接口

namespace Infrastructure::Persistence {

class FileManagerAdapter {
public:
    explicit FileManagerAdapter(std::shared_ptr<CDataRWMgr> rwMgr);

    void createFile(const std::string& path);
    size_t readFile(const std::string& path, char* buffer, size_t maxSize);
    void writeFile(const std::string& path, const char* data, size_t size, int offset = -1);
    void truncateFile(const std::string& path);

    bool fileExists(const std::string& path) const;
    size_t getFileSize(const std::string& path) const;

private:
    std::shared_ptr<CDataRWMgr> m_rwMgr;
};

} // namespace Infrastructure::Persistence
```

---

## 2. 缓存（Cache）

### 2.1 InMemoryNoteCache - 内存缓存实现

```cpp
// Infrastructure/Cache/InMemoryNoteCache.h
#pragma once
#include <memory>
#include <vector>
#include <shared_mutex>

namespace Domain::Note { class Note; }

namespace Infrastructure::Cache {

class InMemoryNoteCache {
public:
    explicit InMemoryNoteCache(size_t maxSize = 500);

    // 缓存操作
    void put(const std::vector<Domain::Note::Note>& notes);
    std::vector<Domain::Note::Note> get() const;
    void invalidate();
    bool isValid() const;

    // 统计信息
    size_t size() const;
    size_t maxCapacity() const;
    bool isFull() const;

private:
    mutable std::shared_mutex m_mutex;
    std::shared_ptr<std::vector<Domain::Note::Note>> m_cache;
    size_t m_maxSize;

    // 缓存策略
    void evictIfNeeded();
};

} // namespace Infrastructure::Cache
```

**与当前 CNoteDataCache 的关系**：

| 当前 CNoteDataCache | 新 InMemoryNoteCache |
|---------------------|----------------------|
| 存储 `vector<ST_NOTE_DATA>` | 存储 `vector<Note>` (领域对象) |
| 直接暴露内部指针 | 返回副本（Copy-on-Write） |
| 与 Service 耦合 | 独立的缓存组件 |
| 固定500条容量 | 可配置容量 |

**迁移策略**：
```cpp
// 可以保留旧的作为兼容层
class LegacyNoteCacheAdapter : public InMemoryNoteCache {
    // 适配旧接口...
};
```

---

### 2.2 Cache Decorator - 装饰器模式（可选高级用法）

```cpp
// Infrastructure/Cache/CachedNoteRepository.h
#pragma once
#include "Domain/Note/INoteRepository.h"
#include "Persistence/FileNoteRepository.h"
#include "InMemoryNoteCache.h"

namespace Infrastructure::Cache {

class CachedNoteRepository : public Domain::Note::INoteRepository {
public:
    CachedNoteRepository(
        std::unique_ptr<FileNoteRepository> realRepo,  // 被装饰的真实仓储
        std::shared_ptr<InMemoryNoteCache> cache        // 缓存组件
    );

    // 实现接口（自动加入缓存逻辑）
    std::vector<Domain::Note::Note> findAll() const override {
        if (m_cache->isValid()) {
            return m_cache->get();  // 命中缓存
        }

        auto notes = m_realRepo->findAll();  // 未命中，读取真实数据
        m_cache->put(notes);                 // 更新缓存
        return notes;
    }

    void save(const Domain::Note::Note& note) override {
        m_realRepo->save(note);      // 先保存到真实存储
        m_cache->invalidate();        // 使缓存失效
    }

    // ... 其他方法类似 ...

private:
    std::unique_ptr<FileNoteRepository> m_realRepo;
    std::shared_ptr<InMemoryNoteCache> m_cache;
};

} // namespace Infrastructure::Cache
```

**优势**：
- ✅ 对调用者**透明**（无需知道缓存存在）
- ✅ 符合**开闭原则**（可以叠加多个装饰器）
- ✅ **单一职责**（缓存逻辑集中在一处）

---

## 3. 外部服务适配器（External Services）

### 3.1 SystemClockAdapter - 时间服务

```cpp
// Infrastructure/External/SystemClockAdapter.h
#pragma once
#include <chrono>
#include <cstdint>

namespace Infrastructure::External {

class SystemClockAdapter {
public:
    virtual ~SystemClockAdapter() = default;

    virtual int64_t currentTimeMillis() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    virtual int64_t currentTimestamp() const {
        return static_cast<int64_t>(std::time(nullptr));
    }
};

class TestClockAdapter : public SystemClockAdapter {  // 用于测试
public:
    explicit TestClockAdapter(int64_t fixedTime) : m_fixedTime(fixedTime) {}

    int64_t currentTimeMillis() const override {
        return m_fixedTime;
    }

private:
    int64_t m_fixedTime;
};

} // namespace Infrastructure::External
```

**为什么需要？**
- ✅ **可测试**：在单元测试中控制时间
- ✅ **解耦**：不依赖系统时间函数
- ✅ **一致性**：整个应用使用同一个时间源

---

### 3.2 LoggerAdapter - 日志服务

```cpp
// Infrastructure/External/LoggerAdapter.h
#pragma once
#include <string>
#include <fstream>

namespace Infrastructure::External {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class ILogger {
public:
    virtual ~ILogger() = default;

    virtual void log(LogLevel level, const std::string& message) = 0;
    virtual void debug(const std::string& msg) { log(LogLevel::DEBUG, msg); }
    virtual void info(const std::string& msg) { log(LogLevel::INFO, msg); }
    virtual void warning(const std::string& msg) { log(LogLevel::WARNING, msg); }
    virtual void error(const std::string& msg) { log(LogLevel::ERROR, msg); }
};

class FileLogger : public ILogger {
public:
    explicit FileLogger(const std::string& filePath);
    void log(LogLevel level, const std::string& message) override;

private:
    std::ofstream m_logFile;
};

class ConsoleLogger : public ILogger {
public:
    void log(LogLevel level, const std::string& message) override;
};

class NullLogger : public ILogger {  // 空对象模式
public:
    void log(LogLevel level, const std::string& message) override {}
};

} // namespace Infrastructure::External
```

---

## 4. 数据库仓储（未来扩展）- 可选

```cpp
// Infrastructure/Persistence/Database/SqliteNoteRepository.h
#pragma once
#include "Domain/Note/INoteRepository.h"
#include <sqlite3.h>

namespace Infrastructure::Persistence::Database {

class SqliteNoteRepository : public Domain::Note::INoteRepository {
public:
    explicit SqliteNoteRepository(const std::string& dbPath);
    ~SqliteNoteRepository();

    // 实现 INoteRepository 接口
    void save(const Domain::Note::Note& note) override;
    void remove(const Domain::Note::NoteId& id) override;
    std::vector<Domain::Note::Note> findAll() const override;
    // ... 其他方法 ...

private:
    sqlite3* m_db;
    void initializeSchema();
    Domain::Note::Note mapRowToNote(sqlite3_stmt* stmt) const;
};

} // namespace Infrastructure::Persistence::Database
```

**切换示例**：
```cpp
// 在组合根中根据配置选择实现
std::shared_ptr<Domain::Note::INoteRepository> createRepository(Config config) {
    if (config.useDatabase) {
        return make_shared<SqliteNoteRepository>(config.dbPath);
    } else {
        return make_shared<FileNoteRepository>(
            config.filePath,
            createFileManager(),
            make_shared<NoteFileMapper>()
        );
    }
}
```

---

## 5. 基础设施层依赖关系图

```
Infrastructure Layer
├── Persistence/
│   ├── FileNoteRepository.h/cpp     ⭐ 核心实现
│   ├── NoteFileMapper.h/cpp          ⭐ 映射器
│   ├── CDataRWMgr_Adapter.h/cpp  (复用现有代码)
│   └── Database/                     (未来扩展)
│       └── SqliteNoteRepository.h/cpp
├── Cache/
│   ├── InMemoryNoteCache.h/cpp       ⭐ 缓存组件
│   └── CachedNoteRepository.h/cpp   ⭐ 装饰器
├── External/
│   ├── SystemClockAdapter.h/cpp      ⭐ 时间服务
│   └── LoggerAdapter.h/cpp           ⭐ 日志服务
└── Config/
    └── AppConfig.h/cpp              配置管理

依赖关系：
✅ 实现 Domain 层的接口（INoteRepository 等）
✅ 可以依赖 Qt、标准库、第三方库
❌ 不被 Domain/Application 层依赖
⚠️ 只被 Application 层通过接口间接使用
```

---

## 6. 单元测试示例

```cpp
// TestCode/Infrastructure/TestFileNoteRepository.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Infrastructure/Persistence/FileNoteRepository.h"
#include "Infrastructure/Persistence/NoteFileMapper.h"
#include "Domain/Note/Note.h"

using ::testing::Return;
using ::testing::_;

class MockNoteFileMapper : public Infrastructure::Persistence::NoteFileMapper {
public:
    MOCK_METHOD(std::vector<ST_NOTE_DATA>, fromDomainList, (...), (const, override));
    MOCK_METHOD(std::vector<Domain::Note::Note>, toDomainList, (...), (const, override));
};

TEST(FileNoteRepositoryTest, SaveAndRetrieve) {
    // Arrange
    auto mapper = std::make_shared<MockNoteFileMapper>();
    auto fileManager = /* 创建真实的或 mock 的 */;

    Infrastructure::Persistence::FileNoteRepository repo("test.dat", fileManager, mapper);

    Domain::Note::Note testNote(Domain::Note::NoteId(1),
                                Domain::Note::NoteContent::create("Test"));

    EXPECT_CALL(*mapper, fromDomainList(_))
        .WillOnce(Return(std::vector<ST_NOTE_DATA>{/*...*/}));

    // Act
    repo.save(testNote);
    auto retrieved = repo.findById(Domain::Note::NoteId(1));

    // Assert
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->getId().value(), 1);
}

TEST(InMemoryNoteCacheTest, PutAndGet) {
    Infrastructure::Cache::InMemoryNoteCache cache(10);

    std::vector<Domain::Note::Note> notes{
        Domain::Note::Note(Domain::Note::NoteId(1), Domain::Note::NoteContent::create("A")),
        Domain::Note::Note(Domain::Note::NoteId(2), Domain::Note::NoteContent::create("B"))
    };

    cache.put(notes);
    EXPECT_TRUE(cache.isValid());

    auto cached = cache.get();
    EXPECT_EQ(cached.size(), 2);
    EXPECT_EQ(cached[0].getContent().getText(), "A");
}

TEST(InMemoryNoteCacheTest, Invalidate) {
    Infrastructure::Cache::InMemoryNoteCache cache(10);

    cache.put(/* some notes */);
    cache.invalidate();

    EXPECT_FALSE(cache.isValid());
    EXPECT_EQ(cache.size(), 0);
}
```

**测试特点**：
- ✅ 测试**真实**的持久化逻辑（集成测试）
- ✅ Mock **外部依赖**（如文件系统）
- ✅ 验证**边界条件**（缓存满、文件不存在等）

---

## 7. 与当前代码的映射

| 当前组件 | 目标位置 | 迁移动作 |
|---------|----------|----------|
| CNoteDataRW | → FileNoteRepository + NoteFileMapper | 拆分为仓储+映射器 |
| CDataRWMgr | → FileManagerAdapter 或直接使用 | 包装或复用 |
| CNoteDataCache | → InMemoryNoteCache | 重写为泛型缓存 |
| CDataService 的缓存逻辑 | → CachedNoteRepository (装饰器) | 提取为独立组件 |
| CDataSave | → Application 层 UseCase | 业务编排上移 |

### 具体迁移步骤

#### Step 1: 创建 Mapper
```cpp
// 将 CNoteDataRW::FileData2NoteData 逻辑移入 Mapper
class NoteFileMapper {
    std::vector<Note> toDomainList(const vector<ST_NOTE_DATA>& raw) {
        // 原 FileData2NoteData 逻辑 + 转换为 Note 对象
    }
};
```

#### Step 2: 创建 Repository
```cpp
// 封装 CNoteDataRW 和 CDataRWMgr
class FileNoteRepository : public INoteRepository {
    void save(const Note& note) override {
        // 原 CNoteDataRW::SaveNoteData 逻辑
    }
};
```

#### Step 3: 适配缓存
```cpp
// 将 CNoteDataCache 改为存储 Note 对象
class InMemoryNoteCache {
    // 类似原 CNoteDataCache 但操作 Note 而非 ST_NOTE_DATA
};
```

---

## 🎯 下一步行动

1. ✅ 创建 `Infrastructure/` 目录结构
2. ✅ 实现 `NoteFileMapper`（字段映射）
3. ✅ 实现 `FileNoteRepository`（封装现有 RW 逻辑）
4. ✅ 实现 `InMemoryNoteCache`（改造现有 Cache）
5. ✅ 编写集成测试验证数据完整性
6. ⏭️ 然后进入 Presentation 层设计...

---

## ⚠️ 性能优化建议

### 优化1：批量操作
```cpp
void FileNoteRepository::saveAll(const std::vector<Note>& notes) {
    // 批量写入比逐条写入快 10-100 倍
    auto rawData = m_mapper->fromDomainList(notes);
    writeRawData(rawData.data(), rawData.size() * sizeof(ST_NOTE_DATA));
}
```

### 优化2：延迟加载
```cpp
class LazyLoadingNoteRepository : public INoteRepository {
    std::vector<Note> findAll() const override {
        if (!m_loaded) {
            m_cachedNotes = loadFromFile();  // 首次访问时加载
            m_loaded = true;
        }
        return m_cachedNotes;
    }
};
```

### 优化3：异步 I/O
```cpp
async future<void> FileNoteRepository::saveAsync(const Note& note) {
    return async(launch::async, [this, note]() {
        this->save(note);  // 在后台线程执行
    });
}
```

---

**记住**：基础设施层是"工人"，它**具体执行** Domain 层定义的"合同"！🔧
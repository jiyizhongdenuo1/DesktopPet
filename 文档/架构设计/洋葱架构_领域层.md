# Domain Layer Design - 领域层详细设计

## 🧅 洋葱架构核心：领域层

### 设计原则
- **零依赖**：不依赖任何外层（Infrastructure, Application, Presentation）
- **纯粹性**：只包含业务逻辑和业务规则
- **稳定性**：最不容易变化的层，是系统的核心价值

---

## 1. 实体（Entities）

### 1.1 Note - 笔记实体

```cpp
// Domain/Note/Note.h
#pragma once
#include <string>
#include <cstdint>
#include <memory>

#include "NoteId.h"
#include "NoteContent.h"

namespace Domain::Note {

class Note {
public:
    // 构造函数（工厂方法或直接构造）
    explicit Note(NoteId id, NoteContent content);

    // 核心业务行为（不是简单的 getter/setter！）
    void updateContent(const NoteContent& newContent);
    void markAsCompleted();
    void markAsDeleted();

    // 查询方法（返回值对象或基本类型）
    NoteId getId() const;
    NoteContent getContent() const;
    bool isCompleted() const;
    bool isDeleted() const;
    int64_t getCreateTime() const;
    int64_t getModifyTime() const;

    // 业务规则验证
    bool isValid() const;

private:
    NoteId m_id;
    NoteContent m_content;
    bool m_completed = false;
    bool m_deleted = false;
    int64_t m_createTime;
    int64_t m_modifyTime;

    // 私有构造强制使用工厂或明确意图
    Note();
};

} // namespace Domain::Note
```

**关键点**：
- ✅ 包含**业务行为**（updateContent, markAsCompleted）
- ✅ **自我验证**（isValid()）
- ✅ 使用**值对象**（NoteId, NoteContent）
- ❌ 不包含持久化逻辑
- ❌ 不包含缓存逻辑

---

### 1.2 NoteId - 值对象

```cpp
// Domain/Note/NoteId.h
#pragma once
#include <cstdint>
#include <string>

namespace Domain::Note {

class NoteId {
public:
    explicit NoteId(int64_t value);

    int64_t value() const;
    std::string toString() const;

    // 值对象语义：不可变，通过值比较
    bool operator==(const NoteId& other) const;
    bool operator!=(const NoteId& other) const;
    bool operator<(const NoteId& other) const;  // 用于排序

private:
    int64_t m_value;
};

} // namespace Domain::Note
```

**特点**：
- ✅ **不可变**（创建后不能修改）
- ✅ **自验证**（构造时检查有效性）
- ✅ **值语义**（比较的是值，不是引用）

---

### 1.3 NoteContent - 值对象

```cpp
// Domain/Note/NoteContent.h
#pragma once
#include <string>
#include "CommonDefine.h"  // CONTENT_LENGTH_MAX 等

namespace Domain::Note {

class NoteContent {
public:
    static NoteContent create(const std::string& text);
    static NoteContent createEmpty();

    std::string getText() const;
    size_t length() const;
    bool isEmpty() const;
    bool isValid() const;  // 检查长度限制、非法字符等

private:
    explicit NoteContent(const std::string& text);
    std::string m_text;

    // 业务常量
    static constexpr size_t MAX_LENGTH = NoteSpace::CONTENT_LENGTH_MAX;
};

} // namespace Domain::Note
```

**职责**：
- 封装内容的**业务规则**
- 长度限制验证
- 格式化（去除空格、特殊字符处理等）

---

## 2. 仓储接口（Repository Interface）⭐ 关键！

```cpp
// Domain/Note/INoteRepository.h
#pragma once
#include <vector>
#include <memory>
#include <optional>

namespace Domain::Note {

class Note;
using NotePtr = std::shared_ptr<Note>;

// 仓储接口（端口 Port）
// 由 Infrastructure 层实现
class INoteRepository {
public:
    virtual ~INoteRepository() = default;

    // CRUD 操作
    virtual void save(const Note& note) = 0;
    virtual void remove(const NoteId& id) = 0;

    virtual std::optional<Note> findById(const NoteId& id) const = 0;
    virtual std::vector<Note> findAll() const = 0;
    virtual std::vector<Note> findCompleted() const = 0;
    virtual std::vector<Note> findActive() const = 0;

    // 查询（返回值对象，避免泄露领域对象）
    virtual size_t count() const = 0;
    virtual bool exists(const NoteId& id) const = 0;

protected:
    INoteRepository() = default;  // 不允许直接实例化
};

} // namespace Domain::Note
```

**为什么这是核心？**
- ✅ **定义了领域层需要的持久化能力**
- ✅ **不关心如何实现**（文件？数据库？内存？）
- ✅ **由外层（Infrastructure）实现**
- ✅ **实现了依赖倒置原则（DIP）**

---

## 3. 领域服务（Domain Services）

### 3.1 NoteBusinessRules - 业务规则引擎

```cpp
// Domain/Note/NoteBusinessRules.h
#pragma once
#include <vector>
#include <string>

namespace Domain::Note {

class Note;
class NoteContent;

class NoteBusinessRules {
public:
    // 业务规则验证

    // 规则1：笔记内容不能为空
    static bool validateContent(const NoteContent& content,
                                std::string& errorMessage);

    // 规则2：笔记数量限制（如最多10000条）
    static bool validateNoteCount(size_t currentCount,
                                  std::string& errorMessage);

    // 规则3：笔记ID必须唯一
    static bool validateUniqueId(const std::vector<Note>& existingNotes,
                                 const Note& st_NoteData,
                                 std::string& errorMessage);

    // 规则4：时间戳有效性
    static bool validateTimestamps(int64_t createTime,
                                  int64_t modifyTime,
                                  std::string& errorMessage);

    // 业务计算
    static bool canAddMoreNotes(size_t currentCount, size_t maxAllowed);
    static bool isNoteTooOld(const Note& note, int64_t thresholdTimeMs);

private:
    NoteBusinessRules() = delete;  // 禁止实例化，纯静态工具类
};

} // namespace Domain::Note
```

**特点**：
- ✅ **无状态**（静态方法或纯函数）
- ✅ **可测试**（输入→输出，无副作用）
- ✅ **集中管理**（所有业务规则在一处）

---

## 4. 领域事件（Domain Events） - 可选

```cpp
// Domain/Note/NoteEvents.h
#pragma once
#include <functional>

namespace Domain::Note {

class NoteId;

// 领域事件基类
struct IDomainEvent {
    virtual ~IDomainEvent() = default;
};

struct NoteCreatedEvent : public IDomainEvent {
    NoteId noteId;
};

struct NoteUpdatedEvent : public IDomainEvent {
    NoteId noteId;
};

struct NoteDeletedEvent : public IDomainEvent {
    NoteId noteId;
};

// 事件处理器类型
template<typename EventType>
using EventHandler = std::function<void(const EventType&)>;

} // namespace Domain::Note
```

**用途**：
- 解耦领域逻辑与副作用（日志、通知、缓存失效等）
- 实现事件驱动架构（EDA）

---

## 5. 异常与错误处理

```cpp
// Domain/Common/DomainException.h
#pragma once
#include <stdexcept>
#include <string>

namespace Domain {

class DomainException : public std::runtime_error {
public:
    explicit DomainException(const std::string& message)
        : std::runtime_error(message) {}
};

class ValidationException : public DomainException {
public:
    explicit ValidationException(const std::string& message)
        : DomainException("Validation failed: " + message) {}
};

class NotFoundException : public DomainException {
public:
    explicit NotFoundException(const std::string& resource,
                              const std::string& id)
        : DomainException(resource + " not found with id: " + id) {}
};

class DuplicateException : public DomainException {
public:
    explicit DuplicateException(const std::string& resource,
                                const std::string& id)
        : DomainException(resource + " already exists with id: " + id) {}
};

} // namespace Domain
```

**原则**：
- ✅ 使用**特定异常类型**而非通用异常
- ✅ 包含**详细的错误上下文**
- ✅ 在**领域层定义**，各层复用

---

## 6. 依赖关系图

```
Domain Layer (零依赖)
├── Note/
│   ├── Note.h              (实体)
│   ├── NoteId.h            (值对象)
│   ├── NoteContent.h       (值对象)
│   ├── INoteRepository.h   (接口/端口) ⭐
│   └── NoteBusinessRules.h (领域服务)
├── Common/
│   └── DomainException.h   (异常)
└── Events/
    └── NoteEvents.h        (事件)

依赖关系：
• 无外部依赖 ✅
• 只依赖标准库 (std::string, std::vector 等)
• 可被 Application 和 Infrastructure 层依赖
```

---

## 7. 单元测试示例

```cpp
// TestCode/Domain/TestNoteEntity.cpp
#include <gtest/gtest.h>
#include "Domain/Note/Note.h"
#include "Domain/Note/NoteContent.h"
#include "Domain/Note/NoteBusinessRules.h"

using namespace Domain::Note;

TEST(NoteTest, CreateValidNote) {
    auto content = NoteContent::create("Hello World");
    NoteId id(1);
    Note note(id, content);

    EXPECT_TRUE(note.isValid());
    EXPECT_EQ(note.getId(), id);
    EXPECT_FALSE(note.isCompleted());
}

TEST(NoteTest, UpdateContent) {
    auto content1 = NoteContent::create("Original");
    Note note(NoteId(1), content1);

    auto content2 = NoteContent::create("Updated");
    note.updateContent(content2);

    EXPECT_EQ(note.getContent().getText(), "Updated");
}

TEST(NoteBusinessRulesTest, ValidateEmptyContent) {
    auto emptyContent = NoteContent::createEmpty();
    std::string error;

    EXPECT_FALSE(NoteBusinessRules::validateContent(emptyContent, error));
    EXPECT_FALSE(error.empty());
}

TEST(NoteBusinessRulesTest, ValidateNoteCount) {
    std::string error;
    EXPECT_TRUE(NoteBusinessRules::validateNoteCount(100, error));  // 允许
    EXPECT_FALSE(NoteBusinessRules::validateNoteCount(10001, error));  // 超限
}
```

**优势**：
- ✅ **无需 mock**（领域层无外部依赖）
- ✅ **快速执行**（纯逻辑，无 I/O）
- ✅ **高可信度**（测试核心业务规则）

---

## 📋 迁移清单

### 从当前代码迁移到领域层

| 当前组件 | 目标位置 | 迁移动作 |
|---------|----------|----------|
| `ST_NOTE_DATA` (datatype.h) | → `Note` 实体 + 值对象 | 拆分为 Note, NoteId, NoteContent |
| `CNoteBusiness` | → `NoteBusinessRules` | 提取为静态方法 |
| 业务验证逻辑 | → `NoteBusinessRules` 或实体的 `isValid()` | 集中到领域层 |
| 错误码/异常 | → `DomainException` 层次结构 | 统一异常体系 |

### 不应该迁移的内容（留在其他层）

| 组件 | 原因 | 正确位置 |
|------|------|----------|
| CNoteDataCache | 是缓存实现细节 | Infrastructure/Cache |
| CNoteDataRW | 是文件I/O实现 | Infrastructure/Persistence |
| CDataSave | 包含工作流编排 | Application/UseCases |
| CMainNoteListViewModel | UI相关 | Presentation |

---

## ⚠️ 常见陷阱与解决方案

### 陷阱1：贫血模型（Anemic Domain Model）

❌ 错误做法：
```cpp
class Note {
public:
    string content;  // 公有字段，无行为
    bool completed;
    // 只有 getter/setter，没有业务逻辑
};
```

✅ 正确做法：
```cpp
class Note {
public:
    void markAsCompleted() {  // 封装业务行为
        if (m_deleted) throw DomainException("Cannot complete deleted note");
        m_completed = true;
    }
private:
    bool m_completed;
};
```

### 陷阱2：泄漏基础设施依赖

❌ 错误做法：
```cpp
// Domain 层不应该知道数据库或文件
class Note {
    void saveToDatabase(...) {}  // ❌ 错误！
    void serializeToFile(...) {} // ❌ 错误！
};
```

✅ 正确做法：
```cpp
// Domain 只定义接口
class INoteRepository {
    virtual void save(const Note& note) = 0;
};

// Infrastructure 层实现
class FileNoteRepository : public INoteRepository {
    void save(const Note& note) override { /* 文件操作 */ }
};
```

### 陷阱3：滥用领域服务

❌ 错误做法：把所有逻辑都塞进领域服务
```cpp
class NoteService {  // 变成上帝类
    void createNote(...);
    void saveNote(...);
    void sendNotification(...);  // 这不是领域逻辑！
    void logToFile(...);         // 这也不是！
};
```

✅ 正确做法：
- **实体自身的行为** → 放在实体中
- **跨实体的业务规则** → 放在领域服务
- **应用流程编排** → 放在应用层用例
- **基础设施操作** → 放在基础设施层

---

## 🎯 下一步行动

1. ✅ 创建 `Domain/Note/` 目录结构
2. ✅ 实现 `Note`, `NoteId`, `NoteContent`
3. ✅ 定义 `INoteRepository` 接口
4. ✅ 实现 `NoteBusinessRules`
5. ✅ 编写单元测试验证领域逻辑
6. ⏭️ 然后进入 Application 层设计...

---

**记住**：领域层是洋葱的**心脏**，值得花时间精心设计！💚
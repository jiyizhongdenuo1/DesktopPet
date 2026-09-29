# Application Layer Design - 应用层详细设计

## 🧅 洋葱架构：应用层（Application Layer）

### 设计原则
- **编排**：协调领域对象完成业务流程
- **薄层**：不包含业务逻辑，只包含流程控制
- **用例驱动**：每个公开功能对应一个 Use Case
- **依赖倒置**：依赖 Domain 层接口，被 Presentation 层依赖

---

## 1. 用例（Use Cases）⭐ 核心！

### 1.1 AddNoteUseCase - 添加笔记用例

```cpp
// Application/UseCases/AddNoteUseCase.h
#pragma once
#include <memory>
#include <functional>

// 前向声明（避免头文件依赖）
namespace Domain::Note {
    class Note;
    class INoteRepository;
    class NoteContent;
}

namespace Application::UseCases {

class AddNoteUseCase {
public:
    // 请求/响应 DTO（数据传输对象）
    struct Request {
        std::string content;
        int64_t timestamp;  // 创建时间
    };

    struct Response {
        bool success;
        std::string message;  // 成功消息或错误信息
        int64_t noteId;       // 新创建笔记的ID
    };

    using Callback = std::function<void(const Response&)>;

    // 构造函数注入依赖
    explicit AddNoteUseCase(
        std::shared_ptr<Domain::Note::INoteRepository> repository
    );

    // 执行用例（同步版本）
    Response execute(const Request& request);

    // 执行用例（异步版本）
    void executeAsync(const Request& request, Callback callback);

private:
    std::shared_ptr<Domain::Note::INoteRepository> m_repository;

    // 内部业务流程编排
    Domain::Note::Note createNoteFromRequest(const Request& request);
    void validateRequest(const Request& request);
};

} // namespace Application::UseCases
```

**实现示例**：

```cpp
// Application/UseCases/AddNoteUseCase.cpp
#include "AddNoteUseCase.h"
#include "Domain/Note/Note.h"
#include "Domain/Note/NoteContent.h"
#include "Domain/Note/NoteBusinessRules.h"
#include "Domain/Common/DomainException.h"

using namespace Application::UseCases;

AddNoteUseCase::AddNoteUseCase(
    std::shared_ptr<Domain::Note::INoteRepository> repository)
    : m_repository(std::move(repository))
{
    if (!m_repository) {
        throw std::invalid_argument("AddNoteUseCase: repository cannot be null");
    }
}

void AddNoteUseCase::validateRequest(const Request& request) {
    // 验证内容非空
    auto content = Domain::Note::NoteContent::create(request.content);
    std::string error;
    if (!Domain::Note::NoteBusinessRules::validateContent(content, error)) {
        throw Domain::ValidationException(error);
    }

    // 验证笔记数量限制
    auto currentCount = m_repository->count();
    if (!Domain::Note::NoteBusinessRules::validateNoteCount(currentCount, error)) {
        throw Domain::ValidationException(error);
    }
}

Domain::Note::Note AddNoteUseCase::createNoteFromRequest(const Request& request) {
    auto content = Domain::Note::NoteContent::create(request.content);
    auto newId = Domain::Note::NoteId(generateUniqueId());  // ID生成策略
    return Domain::Note::Note(newId, content);
}

AddNoteUseCase::Response AddNoteUseCase::execute(const Request& request) {
    Response response{false, "", -1};

    try {
        // 步骤1：验证请求
        validateRequest(request);

        // 步骤2：创建领域对象
        auto note = createNoteFromRequest(request);

        // 步骤3：持久化（通过仓储）
        m_repository->save(note);

        // 步骤4：构建成功响应
        response.success = true;
        response.message = "Note created successfully";
        response.noteId = note.getId().value();

    } catch (const Domain::ValidationException& e) {
        response.message = e.what();
    } catch (const std::exception& e) {
        response.message = std::string("Internal error: ") + e.what();
    }

    return response;
}
```

**关键点**：
- ✅ **纯编排**：调用领域对象的方法
- ✅ **异常处理**：将领域异常转换为响应
- ✅ **事务边界**：一个用例 = 一个事务单元
- ❌ 不包含业务规则（在 Domain 层）

---

### 1.2 GetNotesUseCase - 获取笔记列表用例

```cpp
// Application/UseCases/GetNotesUseCase.h
#pragma once
#include <vector>
#include <memory>

namespace Domain::Note {
    class Note;
    class INoteRepository;
}

namespace Application::UseCases {

class GetNotesUseCase {
public:
    struct Request {
        enum Filter {
            ALL,
            ACTIVE_ONLY,
            COMPLETED_ONLY
        };
        Filter filter = ALL;
    };

    struct NoteDTO {  // Data Transfer Object
        int64_t id;
        std::string content;
        bool completed;
        bool deleted;
        int64_t createTime;
        int64_t modifyTime;
    };

    struct Response {
        bool success;
        std::string message;
        std::vector<NoteDTO> notes;
    };

    explicit GetNotesUseCase(
        std::shared_ptr<Domain::Note::INoteRepository> repository
    );

    Response execute(const Request& request = Request{});

private:
    std::shared_ptr<Domain::Note::INoteRepository> m_repository;

    NoteDTO convertToDTO(const Domain::Note::Note& note) const;
};

} // namespace Application::UseCases
```

**为什么需要 DTO？**
- ✅ **隔离层**：Presentation 层不知道 Domain 实体
- ✅ **灵活性**：可以按需裁剪数据（如隐藏敏感字段）
- ✅ **序列化友好**：容易转换为 JSON/QML 数据

---

### 1.3 DeleteNoteUseCase - 删除笔记用例

```cpp
// Application/UseCases/DeleteNoteUseCase.h
#pragma once
#include <memory>

namespace Domain::Note {
    class NoteId;
    class INoteRepository;
}

namespace Application::UseCases {

class DeleteNoteUseCase {
public:
    struct Request {
        int64_t noteId;
    };

    struct Response {
        bool success;
        std::string message;
    };

    explicit DeleteNoteUseCase(
        std::shared_ptr<Domain::Note::INoteRepository> repository
    );

    Response execute(const Request& request);

private:
    std::shared_ptr<Domain::Note::INoteRepository> m_repository;
};

} // namespace Application::UseCases
```

---

## 2. 应用服务（Application Services）

### 2.1 NoteApplicationService - 门面模式

```cpp
// Application/Services/NoteApplicationService.h
#pragma once
#include <memory>
#include "UseCases/AddNoteUseCase.h"
#include "UseCases/GetNotesUseCase.h"
#include "UseCases/DeleteNoteUseCase.h"

namespace Application::Services {

class NoteApplicationService {
public:
    explicit NoteApplicationService(
        std::shared_ptr<Domain::Note::INoteRepository> repository
    );

    // 提供统一的入口点（门面）
    AddNoteUseCase::Response addNote(const std::string& content, int64_t timestamp);
    GetNotesUseCase::Response getNotes(GetNotesUseCase::Request::Filter filter);
    DeleteNoteUseCase::Response deleteNote(int64_t noteId);

    // 直接访问底层用例（如果需要更细粒度控制）
    AddNoteUseCase& getAddNoteUseCase() { return *m_addNoteUseCase; }
    GetNotesUseCase& getGetNotesUseCase() { return *m_getNotesUseCase; }
    DeleteNoteUseCase& getDeleteNoteUseCase() { return *m_deleteNoteUseCase; }

private:
    std::unique_ptr<AddNoteUseCase> m_addNoteUseCase;
    std::unique_ptr<GetNotesUseCase> m_getNotesUseCase;
    std::unique_ptr<DeleteNoteUseCase> m_deleteNoteUseCase;
};

} // namespace Application::Services
```

**何时使用应用服务？**
- ✅ 需要**跨多个用例协调**时
- ✅ 需要**统一的事务管理**时
- ✅ 需要**权限检查、日志**等横切关注点时
- ⚠️ 如果只是简单包装，可能过度设计

---

## 3. 端口与适配器（Ports & Adapters）

### 3.1 输入端口（Input Ports）- 从表现层接收

```cpp
// Application/Ports/IInputPort.h
#pragma once
#include <functional>

namespace Application::Ports {

// 通用输入端口接口（可选）
template<typename RequestType, typename ResponseType>
class IInputPort {
public:
    virtual ~IInputPort() = default;
    virtual ResponseType execute(const RequestType& request) = 0;
};

// 笔记相关的输入端口（更具体）
class INoteInputPort {
public:
    virtual ~INoteInputPort() = default;

    virtual void addNote(std::string content, int64_t timestamp) = 0;
    virtual std::vector<NoteDTO> getAllNotes() = 0;
    virtual void deleteNote(int64_t id) = 0;
};

} // namespace Application::Ports
```

### 3.2 输出端口（Output Ports）- 向表现层输出

```cpp
// Application/Ports/IOutputPort.h
#pragma once
#include <vector>
#include <functional>

namespace Application::Ports {

// 观察者模式通知
template<typename DataType>
using DataChangedCallback = std::function<void(const std::vector<DataType>&)>;

class INoteOutputPort {
public:
    virtual ~INoteOutputPort() = default;

    // 通知数据变化（用于更新 UI）
    template<typename T>
    void onDataChanged(DataChangedCallback<T> callback) {
        m_callbacks.push_back(callback);
    }

protected:
    std::vector<std::function<void(const void*)>> m_callbacks;
};

} // namespace Application::Ports
```

---

## 4. DTO 定义

```cpp
// Application/Common/DTOs.h
#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace Application::DTOs {

struct NoteDTO {
    int64_t id;
    std::string content;
    std::string level;      // 笔记级别
    bool isCompleted;
    bool isDeleted;
    int64_t writeTime;      // 写入时间戳
    int64_t modifyTime;     // 修改时间戳
    int64_t remindTime;     // 提醒时间戳
    int32_t remindLevel;    // 提醒级别
    int32_t type;           // 笔记类型

    // 转换辅助方法
    static NoteDTO fromDomain(const Domain::Note::Note& note);
    Domain::Note::Note toDomain() const;
};

struct ErrorDTO {
    bool success;
    int code;
    std::string message;
    std::string details;
};

} // namespace Application::DTOs
```

---

## 5. 应用层依赖关系图

```
Application Layer
├── UseCases/
│   ├── AddNoteUseCase.h/cpp
│   ├── GetNotesUseCase.h/cpp
│   └── DeleteNoteUseCase.h/cpp
├── Services/
│   └── NoteApplicationService.h/cpp (可选)
├── Ports/
│   ├── IInputPort.h
│   └── IOutputPort.h
└── Common/
    └── DTOs.h

依赖关系：
✅ 依赖 Domain 层（实体、仓储接口、领域服务）
❌ 不依赖 Infrastructure 层
❌ 不依赖 Presentation 层
⚠️ 被 Presentation 层依赖
```

---

## 6. 单元测试示例

```cpp
// TestCode/Application/TestAddNoteUseCase.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Application/UseCases/AddNoteUseCase.h"
#include "Domain/Note/INoteRepository.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Throw;

// Mock 仓储（基础设施层的替身）
class MockNoteRepository : public Domain::Note::INoteRepository {
public:
    MOCK_METHOD(void, save, (const Domain::Note::Note&), (override));
    MOCK_METHOD(void, remove, (const Domain::Note::NoteId&), (override));
    MOCK_METHOD(std::optional<Domain::Note::Note>, findById,
                (const Domain::Note::NoteId&), (const, override));
    MOCK_METHOD(std::vector<Domain::Note::Note>, findAll, (), (const, override));
    MOCK_METHOD(size_t, count, (), (const, override));
    MOCK_METHOD(bool, exists, (const Domain::Note::NoteId&), (const, override));
};

TEST(AddNoteUseCaseTest, ExecuteSuccessfully) {
    // Arrange
    auto mockRepo = std::make_shared<MockNoteRepository>();
    EXPECT_CALL(*mockRepo, count()).WillOnce(Return(0));  // 允许添加
    EXPECT_CALL(*mockRepo, save(_)).Times(1);              // 应该保存一次

    Application::UseCases::AddNoteUseCase useCase(mockRepo);
    Application::UseCases::AddNoteUseCase::Request request{
        .content = "Test Note",
        .timestamp = 1625000000
    };

    // Act
    auto response = useCase.execute(request);

    // Assert
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.noteId, 1);  // 第一个笔记ID
    EXPECT_FALSE(response.message.empty());
}

TEST(AddNoteUseCaseTest, RejectEmptyContent) {
    auto mockRepo = std::make_shared<MockNoteRepository>();

    Application::UseCases::AddNoteUseCase useCase(mockRepo);
    Application::UseCases::AddNoteUseCase::Request request{
        .content = "",  // 空内容
        .timestamp = 1625000000
    };

    auto response = useCase.execute(request);

    EXPECT_FALSE(response.success);
    EXPECT_TRUE(response.message.find("Validation") != std::string::npos);
}
```

**测试优势**：
- ✅ 可以 mock 仓储（不需要真实文件）
- ✅ 快速执行（无 I/O 操作）
- ✅ 测试所有路径（成功、验证失败、异常等）
- ✅ 行为验证（确保调用了正确的方法和次数）

---

## 7. 与当前代码的映射

| 当前组件 | 目标位置 | 迁移动作 |
|---------|----------|----------|
| CMainNoteListViewModel::AddNote | → AddNoteUseCase | 提取业务流程 |
| CDataSave::SaveNoteData | → AddNoteUseCase 或 NoteApplicationService | 编排逻辑 |
| CNoteDataService::ReadNoteData | → GetNotesUseCase | 封装为用例 |
| CNoteDataService::SaveNoteData | → AddNoteUseCase | 封装为用例 |
| 业务流程控制逻辑 | → Application Service | 统一管理 |

### 迁移步骤建议

#### Phase 1: 创建用例骨架
1. 创建 `Application/` 目录结构
2. 实现 `AddNoteUseCase`, `GetNotesUseCase`, `DeleteNoteUseCase`
3. 编写单元测试

#### Phase 2: 接入 ViewModel
1. 修改 `CMainNoteListViewModel` 使用 Use Cases
2. 移除直接依赖 `CNoteDataCollect`
3. 通过构造函数或 Setter 注入 Use Cases

#### Phase 3: 清理旧代码
1. 标记 `CDataService` 为 @Deprecated
2. 逐步移除旧的业务逻辑
3. 保留兼容层一段时间（如果有必要）

---

## ⚠️ 常见陷阱

### 陷阱1：用例变成上帝类

❌ 错误：
```cpp
class NoteManagementUseCase {
    void addNote(...);
    void deleteNote(...);
    void updateNote(...);
    void searchNotes(...);
    void exportNotes(...);
    void importNotes(...);
    void archiveNotes(...);
    // ... 无限增长
};
```

✅ 正确：
```cpp
// 每个用例只做一件事
class AddNoteUseCase { /* 只负责添加 */ };
class DeleteNoteUseCase { /* 只负责删除 */ };
class SearchNotesUseCase { /* 只负责搜索 */ };
```

### 陷阱2：在用例中包含业务规则

❌ 错误：
```cpp
Response AddNoteUseCase::execute(Request req) {
    if (req.content.length() > 1000) {  // ❌ 业务规则泄漏！
        return {false, "Content too long"};
    }
    // ...
}
```

✅ 正确：
```cpp
Response AddNoteUseCase::execute(Request req) {
    validateRequest(req);  // 委托给领域层验证
    // ...
}
```

### 陷阱3：忽略错误处理

❌ 错误：
```cpp
Response execute(Request req) {
    auto note = createNote(req);  // 可能抛异常！
    m_repo->save(note);          // 可能失败！
    return {true, ""};           // 假设总是成功
}
```

✅ 正确：
```cpp
Response execute(Request req) {
    try {
        validateRequest(req);
        auto note = createNote(req);
        m_repo->save(note);
        return buildSuccessResponse(note);
    } catch (const ValidationException& e) {
        return buildErrorResponse(e.what());
    } catch (const PersistenceException& e) {
        return buildErrorResponse("Failed to save: " + string(e.what()));
    }
}
```

---

## 🎯 下一步行动

1. ✅ 创建 `Application/` 目录结构
2. ✅ 实现核心 Use Cases（Add, Get, Delete）
3. ✅ 定义 DTO 和 Port 接口
4. ✅ 编写完整的单元测试
5. ⏭️ 然后进入 Infrastructure 层实现...

---

**记住**：应用层是**指挥官**，不是**战士**！它只负责**编排**，把具体工作委托给领域对象。🎖️
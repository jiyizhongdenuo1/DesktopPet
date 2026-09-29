# Presentation Layer Design - 表现层详细设计

## 🧅 洋葱架构：表现层（Presentation Layer）

### 设计原则
- **UI 专注**：只负责展示和用户交互
- **薄层**：不包含业务逻辑（委托给 Application 层）
- **框架依赖**：可以依赖 Qt/QML、UI 库等
- **依赖注入**：通过构造函数接收 Use Cases

---

## 1. ViewModel 层 ⭐ 核心！

### 1.1 MainNoteListViewModel - 主列表 ViewModel

```cpp
// Presentation/ViewModels/MainNoteListViewModel.h
#pragma once
#include <QObject>
#include <QAbstractListModel>
#include <memory>

// 前向声明（避免头文件依赖）
namespace Application::UseCases {
    class GetNotesUseCase;
    class AddNoteUseCase;
    class DeleteNoteUseCase;
}
namespace Application::DTOs { struct NoteDTO; }

namespace Presentation::ViewModels {

class MainNoteListViewModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        ContentRole,
        CompletedRole,
        WriteTimeRole,
        // ... 其他角色
    };

    explicit MainNoteListViewModel(
        std::shared_ptr<Application::UseCases::GetNotesUseCase> getNotesUC,
        std::shared_ptr<Application::UseCases::AddNoteUseCase> addNoteUC,
        std::shared_ptr<Application::UseCases::DeleteNoteUseCase> deleteNoteUC,
        QObject* parent = nullptr
    );
    ~MainNoteListViewModel() override;

    // QAbstractListModel 接口实现
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    // 用户交互操作（由 QML 调用）
    Q_INVOKABLE void addNote(const QString& content);
    Q_INVOKABLE void deleteNote(int index);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void markAsCompleted(int index);

signals:
    // 状态通知（给 QML）
    void noteAddedSuccessfully(int id);
    void errorOccurred(const QString& message);
    void loadingChanged(bool isLoading);

private:
    struct PrivateData;
    std::unique_ptr<PrivateData> d_ptr;

    // 内部数据转换
    QVariantMap noteToVariantMap(const Application::DTOs::NoteDTO& dto) const;

    // UI 更新方法
    void updateModelData(const std::vector<Application::DTOs::NoteDTO>& notes);
};

} // namespace Presentation::ViewModels
```

**实现示例**：

```cpp
// Presentation/ViewModels/MainNoteListViewModel.cpp
#include "MainNoteListViewModel.h"
#include "Application/UseCases/GetNotesUseCase.h"
#include "Application/UseCases/AddNoteUseCase.h"
#include "Application/UseCases/DeleteNoteUseCase.h"
#include "Application/Common/DTOs.h"

using namespace Presentation::ViewModels;

struct MainNoteListViewModel::PrivateData {
    QList<QVariantMap> m_notes;  // UI 数据模型
    bool m_isLoading = false;

    // Use Cases (从外部注入)
    std::shared_ptr<Application::UseCases::GetNotesUseCase> getNotesUC;
    std::shared_ptr<Application::UseCases::AddNoteUseCase> addNoteUC;
    std::shared_ptr<Application::UseCases::DeleteNoteUseCase> deleteNoteUC;
};

MainNoteListViewModel::MainNoteListViewModel(
    std::shared_ptr<Application::UseCases::GetNotesUseCase> getNotesUC,
    std::shared_ptr<Application::UseCases::AddNoteUseCase> addNoteUC,
    std::shared_ptr<Application::UseCases::DeleteNoteUseCase> deleteNoteUC,
    QObject* parent)
    : QAbstractListModel(parent)
    , d_ptr(std::make_unique<PrivateData>())
{
    d_ptr->getNotesUC = getNotesUC;
    d_ptr->addNoteUC = addNoteUC;
    d_ptr->deleteNoteUC = deleteNoteUC;

    refresh();  // 初始化时加载数据
}

void MainNoteListViewModel::addNote(const QString& content) {
    if (!d_ptr->addNoteUC) return;

    emit loadingChanged(true);

    // 构建请求
    Application::UseCases::AddNoteUseCase::Request request{
        .content = content.toStdString(),
        .timestamp = QDateTime::currentMSecsSinceEpoch()
    };

    // 执行 Use Case
    auto response = d_ptr->addNoteUC->execute(request);

    if (response.success) {
        // 刷新列表以显示新笔记
        refresh();
        emit noteAddedSuccessfully(response.noteId);
    } else {
        emit errorOccurred(QString::fromStdString(response.message));
    }

    emit loadingChanged(false);
}

void MainNoteListViewModel::refresh() {
    if (!d_ptr->getNotesUC) return;

    emit loadingChanged(true);

    auto response = d_ptr->getNotesUC->execute();

    if (response.success) {
        beginResetModel();
        d_ptr->m_notes.clear();

        for (const auto& dto : response.notes) {
            d_ptr->m_notes.append(noteToVariantMap(dto));
        }

        endResetModel();
    } else {
        emit errorOccurred(QString::fromStdString(response.message));
    }

    emit loadingChange(false);
}

QVariant MainNoteListViewModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= d_ptr->m_notes.size()) {
        return QVariant();
    }

    const auto& note = d_ptr->m_notes[index.row()];

    switch (role) {
        case IdRole:          return note["id"];
        case ContentRole:     return note["content"];
        case CompletedRole:   return note["completed"];
        case WriteTimeRole:   return note["writeTime"];
        default:              return QVariant();
    }
}

QHash<int, QByteArray> MainNoteListViewModel::roleNames() const {
    return {
        {IdRole, "noteId"},
        {ContentRole, "content"},
        {CompletedRole, "isCompleted"},
        {WriteTimeRole, "writeTime"}
    };
}

QVariantMap MainNoteListViewModel::noteToVariantMap(
    const Application::DTOs::NoteDTO& dto) const {

    return {
        {"id", static_cast<qlonglong>(dto.id)},
        {"content", QString::fromStdString(dto.content)},
        {"completed", dto.isCompleted},
        {"writeTime", static_cast<qlonglong>(dto.writeTime)}
    };
}
```

**关键点**：
- ✅ **不包含业务逻辑**（全部委托给 Use Cases）
- ✅ **Qt 风格**（使用信号槽、Q_INVOKABLE）
- ✅ **数据转换**（DTO → QVariantMap 给 QML）
- ✅ **错误处理**（emit errorOccurred）

---

### 1.2 NoteDetailViewModel - 笔记详情 ViewModel（可选）

```cpp
// Presentation/ViewModels/NoteDetailViewModel.h
#pragma once
#include <QObject>
#include <memory>

namespace Application::UseCases { class GetNotesUseCase; class DeleteNoteUseCase; }
namespace Application::DTOs { struct NoteDTO; }

namespace Presentation::ViewModels {

class NoteDetailViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(int noteId READ noteId CONSTANT)
    Q_PROPERTY(QString content READ content NOTIFY contentChanged)
    Q_PROPERTY(bool isCompleted READ isCompleted NOTIFY isCompletedChanged)

public:
    explicit NoteDetailViewModel(
        std::shared_ptr<Application::UseCases::GetNotesUseCase> getNotesUC,
        std::shared_ptr<Application::UseCases::DeleteNoteUseCase> deleteNoteUC,
        QObject* parent = nullptr
    );

    int noteId() const;
    QString content() const;
    bool isCompleted() const;

public slots:
    Q_INVOKABLE void loadNote(int id);
    Q_INVOKABLE void saveContent(const QString& newContent);
    Q_INVOKABLE void toggleCompleted();

signals:
    void contentChanged();
    void isCompletedChanged();
    void errorOccurred(const QString& message);

private:
    Application::DTOs::NoteDTO m_currentNote;
    std::shared_ptr<Application::UseCases::GetNotesUseCase> m_getNotesUC;
    std::shared_ptr<Application::UseCases::DeleteNoteUseCase> m_deleteNoteUC;
};

} // namespace Presentation::ViewModels
```

---

## 2. QML 视图层

### 2.1 MainWidget.qml - 主界面

```qml
// Presentation/QML/MainWidget.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: "Desktop Pet - Notes"

    // 注入 ViewModel (在 C++ 中设置 contextProperty)
    property var noteModel: null  // 将被设置为 MainNoteListViewModel*

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        // 工具栏
        RowLayout {
            Layout.fillWidth: true
            TextField {
                id: inputField
                placeholderText: "输入新笔记..."
                Layout.fillWidth: true
            }
            Button {
                text: "添加"
                enabled: inputField.text.trim() !== ""
                onClicked: noteModel.addNote(inputField.text)
            }
            Button {
                text: "刷新"
                onClicked: noteModel.refresh()
            }
        }

        // 笔记列表
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: noteModel

            delegate: ItemDelegate {
                width: parent.width
                height: 80

                Column {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 5

                    Text {
                        text: model.content
                        font.pixelSize: 16
                        elide: Text.ElideRight
                        width: parent.width - 100
                    }

                    Text {
                        text: new Date(model.writeTime).toLocaleString()
                        font.pixelSize: 12
                        color: "#888888"
                    }
                }

                CheckBox {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    checked: model.isCompleted
                    onCheckedChanged: noteModel.markAsCompleted(index)
                }
            }
        }

        // 加载指示器
        BusyIndicator {
            running: noteModel ? noteModel.loading : false
            anchors.centerIn: parent
            visible: running
        }
    }

    // 错误提示弹窗
    Dialog {
        id: errorDialog
        title: "错误"
        standardButtons: Dialog.Ok
        modal: true

        Label {
            text: noteModel ? noteModel.errorMessage : ""
            wrapMode: Text.Wrap
        }

        Connections {
            target: noteModel
            function onErrorOccurred(message) {
                errorDialog.open();
            }
        }
    }
}
```

**关键点**：
- ✅ **纯 UI 逻辑**（布局、动画、样式）
- ✅ **数据绑定**（model.xxx 绑定到 ViewModel）
- ✅ **事件转发**（onClick → 调用 ViewModel 方法）
- ❌ 不包含业务规则

---

## 3. 表现层辅助组件

### 3.1 ViewModelFactory - ViewModel 工厂

```cpp
// Presentation/Factories/ViewModelFactory.h
#pragma once
#include <memory>

namespace Presentation::ViewModels {
    class MainNoteListViewModel;
    class NoteDetailViewModel;
}

namespace Application::UseCases {
    class AddNoteUseCase;
    class GetNotesUseCase;
    class DeleteNoteUseCase;
}

namespace Presentation::Factories {

class ViewModelFactory {
public:
    explicit ViewModelFactory(
        std::shared_ptr<Application::UseCases::AddNoteUseCase> addNoteUC,
        std::shared_ptr<Application::UseCases::GetNotesUseCase> getNotesUC,
        std::shared_ptr<Application::UseCases::DeleteNoteUseCase> deleteNoteUC
    );

    std::unique_ptr<Presentation::ViewModels::MainNoteListViewModel>
    createMainNoteListViewModel(QObject* parent = nullptr);

    std::unique_ptr<Presentation::ViewModels::NoteDetailViewModel>
    createNoteDetailViewModel(QObject* parent = nullptr);

private:
    std::shared_ptr<Application::UseCases::AddNoteUseCase> m_addNoteUC;
    std::shared_ptr<Application::UseCases::GetNotesUseCase> m_getNotesUC;
    std::shared_ptr<Application::UseCases::DeleteNoteUseCase> m_deleteNoteUC;
};

} // namespace Presentation::Factories
```

**用途**：
- ✅ **集中创建** ViewModel（统一注入 Use Cases）
- ✅ **简化组合根**（Bootstrapper 只需创建 Factory）
- ✅ **可测试**（可以替换为 Mock Factory）

---

### 3.2 QMLBridge - QML 与 C++ 的桥梁

```cpp
// Presentation/QMLBridge.h
#pragma once
#include <QObject>
#include <QQmlEngine>
#include <memory>

namespace Presentation {

class QMLBridge : public QObject {
    Q_OBJECT

public:
    explicit QMLBridge(QQmlEngine* engine, QObject* parent = nullptr);

    void registerTypes();
    void setContextProperties();

private:
    QQmlEngine* m_engine;
    std::unique_ptr<Presentation::Factories::ViewModelFactory> m_vmFactory;

    void registerQMLTypes();
    void createAndRegisterViewModels();
};

} // namespace Presentation
```

**使用示例**：

```cpp
// 在 Bootstrapper 中
auto bridge = make_unique<Presentation::QMLBridge>(engine.get());
bridge->registerTypes();
bridge->setContextProperties();
```

---

## 4. 表现层依赖关系图

```
Presentation Layer
├── ViewModels/
│   ├── MainNoteListViewModel.h/cpp  ⭐ 核心组件
│   └── NoteDetailViewModel.h/cpp    (可选)
├── QML/
│   ├── MainWidget.qml               ⭐ 主界面
│   ├── NoteDetail.qml               (可选)
│   └── Components/                  可复用UI组件
│       ├── NoteItemDelegate.qml
│       └── ErrorDialog.qml
├── Factories/
│   └── ViewModelFactory.h/cpp      ViewModel工厂
├── Bridges/
│   └── QMLBridge.h/cpp             QML桥接器
└── Converters/
    └── DTOConverters.h/cpp         DTO→QVariant 转换

依赖关系：
✅ 依赖 Application 层（Use Cases, DTOs）
✅ 可以依赖 Qt/QML 框架
❌ 不依赖 Domain 或 Infrastructure 层
⚠️ 被 Bootstrapper 创建和组装
```

---

## 5. 单元测试示例

```cpp
// TestCode/Presentation/TestMainNoteListViewModel.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Presentation/ViewModels/MainNoteListViewModel.h"

using ::testing::Return;
using ::testing::_;

class MockGetNotesUseCase : public Application::UseCases::GetNotesUseCase {
public:
    MOCK_METHOD(Response, execute, ((Request), (override)));
};

class MockAddNoteUseCase : public Application::UseCases::AddNoteUseCase {
public:
    MOCK_METHOD(Response, execute, ((Request), (override)));
};

TEST(MainNoteListViewModelTest, AddNoteSuccess) {
    // Arrange
    auto mockGetUC = make_shared<MockGetNotesUseCase>();
    auto mockAddUC = make_shared<MockAddNoteUseCase>();

    EXPECT_CALL(*mockAddUC, execute(_))
        .WillOnce(Return(Application::UseCases::AddNoteUseCase::Response{
            true, "Success", 1
        }));

    EXPECT_CALL(*mockGetUC, execute())
        .WillOnce(Return(Application::UseCases::GetNotesUseCase::Response{
            true, "", {/* some notes */}
        }));

    Presentation::ViewModels::MainNoteListViewModel vm(mockGetUC, mockAddUC, nullptr);

    // Act & Assert (验证信号是否发射)
    bool signalEmitted = false;
    QObject::connect(&vm, &Presentation::ViewModels::MainNoteListViewModel::noteAddedSuccessfully,
                     [&](int id) {
                         signalEmitted = true;
                         EXPECT_EQ(id, 1);
                     });

    vm.addNote("Test Note");

    EXPECT_TRUE(signalEmitted);
}

TEST(MainNoteListViewModelTest, HandleErrorFromUseCase) {
    auto mockGetUC = make_shared<MockGetNotesUseCase>();
    auto mockAddUC = make_shared<MockAddNoteUseCase>();

    EXPECT_CALL(*mockAddUC, execute(_))
        .WillOnce(Return(Application::UseCases::AddNoteUseCase::Response{
            false, "Validation failed: Content too long", -1
        }));

    Presentation::ViewModels::MainNoteListViewModel vm(mockGetUC, mockAddUC, nullptr);

    QString errorMessage;
    QObject::connect(&vm, &Presentation::ViewModels::MainNoteListViewModel::errorOccurred,
                     [&](const QString& msg) {
                         errorMessage = msg;
                     });

    vm.addNote("A very long content that exceeds the limit...");

    EXPECT_FALSE(errorMessage.isEmpty());
    EXPECT_TRUE(errorMessage.contains("Validation"));
}
```

**测试特点**：
- ✅ 可以 **Mock Use Cases**（不需要真实仓储或文件）
- ✅ 测试 **UI 行为**（信号发射、状态变化）
- ✅ 快速执行（无 GUI 渲染）
- ✅ 验证 **错误处理流程**

---

## 6. 与当前代码的映射

| 当前组件 | 目标位置 | 迁移动作 |
|---------|----------|----------|
| CMainNoteListViewModel | → MainNoteListViewModel | 重构为纯 UI 层 |
| 直接调用 CNoteDataCollect | → 通过 Use Case 接口 | 移除直接依赖 |
| st_NoteModelItem 结构体 | → DTO + QVariantMap | 数据格式标准化 |
| 业务逻辑（如验证） | → 上移至 Application/Domain | 清理 ViewModel |

### 具体迁移步骤

#### Step 1: 提取接口依赖
```cpp
// 当前代码（紧耦合）
CMainNoteListViewModel::Init() {
    auto ptr = g_CDataCollectFactory->GetCDataCollect(E_COLLECTION_TYPE_NOTE);
    m_pNoteDataCollect = dynamic_pointer_cast<CNoteDataCollect>(ptr);
}

// 新代码（松耦合）
MainNoteListViewModel::MainNoteListViewModel(
    shared_ptr<GetNotesUseCase> getUC,
    shared_ptr<AddNoteUseCase> addUC
) : m_getNotesUC(getUC), m_addNotesUC(addUC) {}
```

#### Step 2: 转换数据结构
```cpp
// 当前：内部数据结构
struct st_NoteModelItem {
    qint64 m_s64NoteId;
    QString m_strContent;
    // ...
};
QList<st_NoteModelItem> m_listNote;

// 新改：使用标准容器 + 角色
QList<QVariantMap> m_notes;  // 直接给 QML 用
```

#### Step 3: 简化业务调用
```cpp
// 当前：手动处理所有细节
void CMainNoteListViewModel::AddNote(...) {
    // 构建 st_NoteModelItem
    // 更新 m_listNote
    // 调用 m_pNoteDataCollect->AddNoteData(...)
    // 处理各种异常...
}

// 新改：委托给 Use Case
void MainNoteListViewModel::addNote(const QString& content) {
    auto response = m_addNoteUC->execute({content.toStdString(), ...});
    if (!response.success) {
        emit errorOccurred(QString::fromStdString(response.message));
        return;
    }
    refresh();  // 刷新列表
}
```

---

## 🎯 下一步行动

1. ✅ 创建 `Presentation/` 目录结构
2. ✅ 重构 `MainNoteListViewModel` 使用 Use Cases
3. ✅ 定义清晰的 QML 接口（属性、信号、槽）
4. ✅ 实现 `ViewModelFactory` 和 `QMLBridge`
5. ✅ 编写 UI 测试（Mock Use Cases）
6. ⏭️ 最后进入 Composition Root 组装...

---

## ⚠️ 常见陷阱与最佳实践

### 陷阱1：ViewModel 变胖（Fat ViewModel）

❌ 错误：
```cpp
class MainNoteListViewModel {
    void addNote(QString content) {
        // ❌ 在这里做业务验证！
        if (content.length() > 1000) throw ...;

        // ❌ 在这里做持久化！
        fileRepo.save(...);

        // ❌ 在这里做缓存失效！
        cache.invalidate(...);
    }
};
```

✅ 正确：
```cpp
class MainNoteListViewModel {
    void addNote(QString content) {
        // ✅ 只做一件事：委托给 Use Case
        auto response = m_addNoteUC->execute({content});
        handleResponse(response);
    }
};
```

### 陷阱2：QML 直接访问领域对象

❌ 错误：
```qml
ListView {
    model: noteRepository.findAll()  // ❌ 直接暴露领域对象！
    delegate: Text { text: model.content.getText() }  // ❌ 调用领域方法
}
```

✅ 正确：
```qml
ListView {
    model: viewModel.notes  // ✅ 通过 ViewModel 访问
    delegate: Text { text: model.content }  // ✅ 只读取简单属性
}
```

### 陷阱3：忘记线程安全

❌ 错误：
```cpp
// ViewModel 在主线程，但 Use Case 可能在后台线程执行
void refresh() {
    auto notes = m_getNotesUC->execute();  // 可能阻塞UI线程！
    updateUI(notes);
}
```

✅ 正确：
```cpp
void refresh() {
    emit loadingChanged(true);

    QtConcurrent::run([this]() {
        auto response = m_getNotesUC->execute();
        QMetaObject::invokeMethod(this, [this, response]() {
            updateUI(response.notes);  // 回到主线程更新UI
            emit loadingChanged(false);
        });
    });
}
```

---

## 📱 UI/UX 改进建议

### 建议1：加载状态管理
```cpp
// ViewModel 中
Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

bool loading() const { return m_loading; }

void refresh() {
    setLoading(true);
    asyncExecute([this](auto result) {
        processResult(result);
        setLoading(false);
    });
}
```

### 建议2：撤销/重做支持
```cpp
// 使用 Command Pattern
class UndoableAddNoteCommand {
    void execute() { /* 调用 AddNoteUseCase */ }
    void undo() { /* 调用 DeleteNoteUseCase */ }
};
```

### 建议3：离线模式支持
```cpp
// 检测网络状态
if (offlineManager.isOffline()) {
    localCache.queueForSync(note);
    showOfflineIndicator(true);
}
```

---

**记住**：表现层是"门面"，它应该**美观且简洁**，把复杂的工作交给后面的层！🎨
# DesktopPet 洋葱架构（Onion Architecture）完整重构方案

## 📋 文档索引

本文件夹包含完整的洋葱架构设计文档：

1. **[OnionArchitecture_Domain_Layer.md](./OnionArchitecture_Domain_Layer.md)** - 领域层详细设计
   - 实体（Entities）：Note, NoteId, NoteContent
   - 仓储接口（Repository Interface）：INoteRepository ⭐
   - 领域服务（Domain Services）：NoteBusinessRules
   - 领域事件（Domain Events）

2. **[OnionArchitecture_Application_Layer.md](./OnionArchitecture_Application_Layer.md)** - 应用层详细设计
   - 用例（Use Cases）：AddNote, GetNotes, DeleteNote ⭐
   - 应用服务（Application Services）：NoteApplicationService
   - DTO（Data Transfer Objects）
   - 端口与适配器（Ports & Adapters）

3. **[OnionArchitecture_Infrastructure_Layer.md](./OnionArchitecture_Infrastructure_Layer.md)** - 基础设施层详细设计
   - 持久化实现：FileNoteRepository, NoteFileMapper ⭐
   - 缓存实现：InMemoryNoteCache, CachedNoteRepository
   - 外部服务适配器：SystemClockAdapter, LoggerAdapter

4. **[OnionArchitecture_Presentation_Layer.md](./OnionArchitecture_Presentation_Layer.md)** - 表现层详细设计
   - ViewModel：MainNoteListViewModel, NoteDetailViewModel ⭐
   - QML 视图：MainWidget.qml
   - ViewModel 工厂和 QML 桥接器

5. **[OnionArchitecture_Composition_Root.md](./OnionArchitecture_Composition_Root.md)** - 组合根详细设计
   - CompositionRoot 核心设计 ⭐
   - 与 Qt/QML 集成方案
   - 配置管理和测试支持

---

## 🧅 洋葱架构核心理念

### 架构图

```
                    ┌─────────────────────────────────┐
                    │         表现层 (Presentation)     │
                    │        QML / ViewModel           │ ← 最外层
                    └──────────────┬──────────────────┘
                                   │ 依赖
                    ┌──────────────▼──────────────────┐
                    │       应用层 (Application)       │
                    │    用例 / 服务协调 / 工作流      │
                    └──────────────┬──────────────────┘
                                   │ 依赖
              ┌────────────────────▼────────────────────┐
              │            领域层 (Domain) ⭐             │
              │    实体 / 值对象 / 领域服务 / 仓储接口   │ ← 最核心
              └────────────────────┬────────────────────┘
                                   │ 实现
              ┌────────────────────▼────────────────────┐
              │      基础设施层 (Infrastructure)         │
              │   文件读写 / 缓存 / 数据库 / 外部API    │
              └─────────────────────────────────────────┘
```

### 三大核心原则

| 原则 | 说明 | 本项目应用 |
|------|------|-----------|
| **依赖倒置** | 依赖方向指向圆心 | 外层依赖内层的接口 |
| **单一职责** | 每个类只有一个变化原因 | Use Case 只做一件事 |
| **开闭原则** | 对扩展开放，对修改关闭 | 通过接口扩展，不改代码 |

---

## 📂 新的目录结构（目标架构）

```
DesktopPet/
│
├── ArchitectureDesign/                  ⭐ 新增：架构设计文档
│   ├── OnionArchitecture_Overview.md          （本文档）
│   ├── OnionArchitecture_Domain_Layer.md
│   ├── OnionArchitecture_Application_Layer.md
│   ├── OnionArchitecture_Infrastructure_Layer.md
│   ├── OnionArchitecture_Presentation_Layer.md
│   └── OnionArchitecture_Composition_Root.md
│
├── Domain/                        ⭐ 新增：领域层（最核心）
│   ├── Note/
│   │   ├── Note.h                 笔记实体
│   │   ├── NoteId.h               值对象：笔记ID
│   │   ├── NoteContent.h          值对象：笔记内容
│   │   ├── INoteRepository.h      仓储接口（端口）⭐⭐⭐
│   │   └── NoteBusinessRules.h    领域服务
│   ├── Common/
│   │   └── DomainException.h      异常定义
│   └── CMakeLists.txt
│
├── Application/                   ⭐ 新增：应用层
│   ├── UseCases/
│   │   ├── AddNoteUseCase.h/cpp   添加笔记用例
│   │   ├── GetNotesUseCase.h/cpp  获取列表用例
│   │   └── DeleteNoteUseCase.h/cpp 删除笔记用例
│   ├── DTOs/
│   │   └── NoteDTO.h             数据传输对象
│   └── CMakeLists.txt
│
├── Infrastructure/                ⭐ 新增：基础设施层
│   ├── Persistence/
│   │   ├── FileNoteRepository.h/cpp    文件仓储实现
│   │   ├── NoteFileMapper.h/cpp        对象映射器
│   │   └── CDataRWMgr_Adapter.h/cpp
│   ├── Cache/
│   │   ├── InMemoryNoteCache.h/cpp     内存缓存
│   │   └── CachedNoteRepository.h/cpp  缓存装饰器
│   └── CMakeLists.txt
│
├── Presentation/                  ⭐ 新增：表现层
│   ├── ViewModels/
│   │   └── MainNoteListViewModel.h/cpp
│   ├── QML/
│   │   └── MainWidget.qml
│   └── CMakeLists.txt
│
├── Bootstrapper/                  ✏️ 修改：组合根
│   ├── CompositionRoot.h/cpp      ⭐ 新增：组合根
│   ├── CAppBootstrapper.h/cpp     修改：使用 CompositionRoot
│   └── CMakeLists.txt
│
├── Services/                      ✏️ 保留但重新组织
│   ├── Module/
│   │   ├── Adapter/DataManager/
│   │   │   ├── CServiceLocator.h/cpp        可选：服务定位器
│   │   │   ├── CNoteDataService.h/cpp       迁移到 Application 层
│   │   │   ├── CNoteDataCache.h/cpp         迁移到 Infrastructure 层
│   │   │   └── ... 其他文件逐步迁移
│   │   └── Other/DataReadWriter/
│   │       ├── CNoteDataRW.h/cpp            迁移到 Infrastructure 层
│   │       └── CDataSave.h/cpp             迁移到 Application 层
│   └── WidgetServices/MainWidget/
│       └── CMainNoteListViewModel.h/cpp    迁移到 Presentation 层
│
├── TestCode/                      ✏️ 扩展：测试代码
│   ├── Domain/                    领域层单元测试
│   ├── Application/               应用层单元测试
│   ├── Infrastructure/            基础设施层集成测试
│   ├── Presentation/              表现层 UI 测试
│   └── Integration/               集成测试
│
└── RequirementDoc/                ✏️ 更新：需求文档
    └── 编码规范与项目规则.md       添加洋葱架构规范
```

---

## 🔄 当前 vs 目标映射关系

### 组件迁移对照表

| 当前组件 | 当前位置 | 目标位置 | 迁移类型 | 优先级 |
|---------|----------|----------|----------|--------|
| **ST_NOTE_DATA** | datatype.h | Domain/Note/Note + 值对象 | 拆分+重写 | P0 |
| **CNoteBusiness** | BusinessModule/CNoteBusiness | Domain/Note/NoteBusinessRules | 重构 | P0 |
| **INoteRepository** (新) | 无 | Domain/Note/INoteRepository.h | **新增** | P0 ⭐ |
| **CNoteDataService** | DataManager/CNoteDataService | Application/UseCases/*UseCase | 拆分 | P1 |
| **CNoteDataCache** | DataManager/CNoteDataCache | Infrastructure/Cache/InMemoryNoteCache | 重构 | P1 |
| **CNoteDataRW** | DataReadWriter/CNoteDataRW | Infrastructure/Persistence/FileNoteRepository | 拆分 | P1 |
| **CDataRWMgr** | DataReadWriter/CDataRWMgr | Infrastructure/Persistence/FileManagerAdapter | 包装 | P2 |
| **CDataSave** | DataReadWriter/CDataSave | Application/UseCases 或 Service | 重构 | P1 |
| **CMainNoteListViewModel** | WidgetServices/MainWidget | Presentation/ViewModels/MainNoteListViewModel | 重构 | P1 |
| **CAppBootstrapper** | Bootstrapper/ | Bootstrapper/CompositionRoot | 重构 | P0 |
| **CServiceLocator** | DataManager/CServiceLocator | 移除或保留为适配器 | 决策中 | P3 |

### 数据结构转换

```
当前：ST_NOTE_DATA (C风格结构体)
├── m_s64NoteId: INT64
├── m_szContent: char[CONTENT_LENGTH_MAX]
├── m_eNoteLevel: ENUM
├── m_s64WriteTime: INT64
├── m_s64ModifyTime: INT64
├── m_s64RemindTime: INT64
├── m_s32RemindLevel: INT32
└── m_s32Type: INT32

         ↓ 映射 (Mapper)

目标：Domain::Note (面向对象实体)
├── id: NoteId (值对象)
├── content: NoteContent (值对象)
├── completed: bool
├── deleted: bool
├── createTime: int64_t
└── modifyTime: int64_t
```

---

## 🎯 分阶段实施计划

### Phase 0: 准备工作（预计 1-2 天）

#### 任务清单
- [ ] 阅读并理解所有架构设计文档
- [ ] 创建新的目录结构骨架
- [ ] 设置 CMakeLists.txt（新目录的编译配置）
- [ ] 备份现有代码（Git分支或复制）
- [ ] 确认开发环境和工具链

#### 交付物
- [x] ✅ ArchitectureDesign/ 文件夹及5份设计文档
- [ ] 新目录结构的空文件框架
- [ ] 可编译的最小项目骨架

---

### Phase 1: 领域层实现（预计 3-5 天）⭐ 最重要！

#### 任务清单
- [ ] 实现 `Domain/Note/Note.h` - Note 实体类
- [ ] 实现 `Domain/Note/NoteId.h` - NoteId 值对象
- [ ] 实现 `Domain/Note/NoteContent.h` - NoteContent 值对象
- [ ] 定义 `Domain/Note/INoteRepository.h` - 仓储接口 ⭐⭐⭐
- [ ] 实现 `Domain/Note/NoteBusinessRules.h` - 业务规则
- [ ] 定义 `Domain/Common/DomainException.h` - 异常体系
- [ ] 编写领域层单元测试（TestCode/Domain/）
- [ ] 运行测试确保通过

#### 关键决策点
- [ ] Note 实体是否需要继承 QObject？（建议：不需要，纯 C++ 类）
- [ ] 是否使用 Qt 的元对象系统？（建议：只在 Presentation 层使用）
- [ ] ID生成策略？自增？UUID？时间戳？

#### 交付物
- [ ] 完整的 Domain 层代码
- [ ] 通过的单元测试
- [ ] API 文档（Doxygen 或注释）

---

### Phase 2: 基础设施层实现（预计 3-5 天）

#### 任务清单
- [ ] 实现 `Infrastructure/Persistence/NoteFileMapper.h/cpp`
  - [ ] `toDomain()` - 二进制 → 领域对象
  - [ ] `fromDomain()` - 领域对象 → 二进制
- [ ] 实现 `Infrastructure/Persistence/FileNoteRepository.h/cpp`
  - [ ] 实现 INoteRepository 所有方法
  - [ ] 复用现有的 CNoteDataRW 和 CDataRWMgr 逻辑
- [ ] 实现 `Infrastructure/Cache/InMemoryNoteCache.h/cpp`
  - [ ] 改造现有 CNoteDataCache 为泛型版本
  - [ ] 支持 Copy-on-Write
- [ ] 可选：实现 `CachedNoteRepository` 装饰器
- [ ] 编写基础设施层测试（TestCode/Infrastructure/）
- [ ] 集成测试：验证数据持久化正确性

#### 关键挑战
- [ ] 如何无缝对接现有的二进制文件格式？
- [ ] 如何处理向后兼容性（读取旧格式的文件）？
- [ ] 性能优化：批量操作、异步 I/O？

#### 交付物
- [ ] 完整的 Infrastructure 层代码
- [ ] 通过的单元测试和集成测试
- [ ] 性能基准测试报告

---

### Phase 3: 应用层实现（预计 2-3 天）

#### 任务清单
- [ ] 定义 `Application/DTOs/NoteDTO.h`
- [ ] 实现 `Application/UseCases/AddNoteUseCase.h/cpp`
- [ ] 实现 `Application/UseCases/GetNotesUseCase.h/cpp`
- [ ] 实现 `Application/UseCases/DeleteNoteUseCase.h/cpp`
- [ ] 可选：实现 `NoteApplicationService` 门面
- [ ] 编写应用层测试（TestCode/Application/）
  - [ ] Mock 仓储进行单元测试
  - [ ] 测试各种场景（成功、失败、边界条件）

#### 关键点
- [ ] Use Case 的异常处理策略？
- [ ] 是否需要异步支持（QtConcurrent）？
- [ ] 事务边界如何定义？

#### 交付物
- [ ] 完整的 Application 层代码
- [ ] 通过的单元测试
- [ ] 使用示例和文档

---

### Phase 4: 表现层改造（预计 2-3 天）

#### 任务清单
- [ ] 重构 `Presentation/ViewModels/MainNoteListViewModel`
  - [ ] 移除对旧系统的依赖
  - [ ] 注入 Use Cases
  - [ ] 适配 QML 接口
- [ ] 更新 QML 视图（如需要）
- [ ] 实现 `Presentation/Factories/ViewModelFactory`
- [ ] 改造 `Bootstrapper/QMLBridge`
- [ ] 编写表现层测试（TestCode/Presentation/）
  - [ ] Mock Use Cases 测试 ViewModel 行为

#### 注意事项
- [ ] 保持向后兼容（QML 属性名不变）
- [ ] 渐进式迁移（可以并行运行新旧系统）
- [ ] UI 响应性能（避免主线程阻塞）

#### 交付物
- [ ] 重构后的表现层代码
- [ ] 通过的 UI 测试
- [ ] 用户手册更新（如有必要）

---

### Phase 5: 组装与集成（预计 2-3 天）

#### 任务清单
- [ ] 实现 `Bootstrapper/CompositionRoot.h/cpp`
  - [ ] 分层创建逻辑
  - [ ] 生命周期管理
  - [ ] 错误处理
- [ ] 改造 `CAppBootstrapper` 使用 CompositionRoot
- [ ] 配置管理（可选）
- [ ] 全量集成测试（TestCode/Integration/）
  - [ ] 端到端流程测试
  - [ ] 性能测试
  - [ ] 内存泄漏检测
- [ ] 清理旧代码
  - [ ] 标记 @Deprecated
  - [ ] 更新注释
  - [ ] 最终删除（确认无问题后）

#### 交付物
- [ ] 完整可运行的系统
- [ ] 通过的所有测试
- [ ] 性能报告
- [ ] 迁移完成报告

---

### Phase 6: 文档与收尾（预计 1-2 天）

#### 任务清单
- [ ] 更新 `RequirementDoc/编码规范与项目规则.md`
  - [ ] 添加洋葱架构规范
  - [ ] 更新目录结构说明
  - [ ] 更新命名规范
- [ ] 编写开发者指南
  - [ ] 如何添加新的 Use Case？
  - [ ] 如何添加新的 Repository 实现？
  - [ ] 如何编写测试？
- [ ] 代码审查
- [ ] 最终 Git 提交和打标签

#### 交付物
- [ ] 完整的项目文档
- [ ] 开发者指南
- [ ] 发布说明

---

## ⚠️ 风险与缓解措施

### 高风险项

| 风险 | 影响 | 概率 | 缓解措施 |
|------|------|------|----------|
| **数据格式不兼容** | 无法读取旧文件 | 中 | Mapper 兼容旧格式；渐进式迁移 |
| **性能下降** | 吞吐量降低 | 低 | 性能基准测试；优化热点路径 |
| **学习曲线陡峭** | 团队适应慢 | 中 | 详细文档；培训；结对编程 |
| **过度工程** | 复杂度超出需求 | 中 | YAGNI原则；按需实现 |

### 回滚策略

如果某个阶段出现问题：
1. **Git 分支策略**：每个 Phase 一个分支，可随时回退
2. **兼容层**：保留旧代码作为后备
3. **特性开关**：运行时切换新旧实现
4. **灰度发布**：先在部分功能上验证

---

## 📊 成功标准

### 功能完整性
- [ ] 所有原有功能正常工作
- [ ] 无回归缺陷
- [ ] 数据兼容性 100%

### 架构质量
- [ ] 依赖方向严格符合洋葱架构
- [ ] 无循环依赖
- [ ] 代码覆盖率 > 80%
- [ ] 技术债务显著减少

### 可维护性
- [ ] 新功能开发效率提升 20%+
- [ ] Bug 修复时间缩短 30%+
- [ ] 新团队成员上手时间 < 3天

### 性能指标
- [ ] 启动时间不超过原来 110%
- [ ] 内存占用增长 < 15%
- [ ] 操作响应时间 < 100ms (P99)

---

## 💡 最佳实践提示

### 开发时遵循的原则

1. **小步快跑**
   - 每次只改一个小的功能点
   - 频繁提交（每天至少一次）
   - 持续集成（每次提交都跑测试）

2. **测试先行**
   - 先写测试，再写实现
   - 保持测试绿灯
   - 重构前确保测试覆盖

3. **持续沟通**
   - 每日站会同步进度
   - 及时记录决策和原因
   - Code Review 必不可少

4. **文档同步**
   - 代码变更后立即更新文档
   - 保持文档与代码一致
   - 使用示例优于长篇描述

---

## ❓ 常见问题 FAQ

### Q1: 为什么不直接用 Spring/DI 框架？
**A**: 
- ✅ C++ 项目，Qt 已提供部分 DI 能力
- ✅ 手动 DI 更透明，易于调试
- ✅ 避免引入外部依赖
- ⚠️ 如果项目规模扩大，可以考虑 Boost.DI 或 fruit

### Q2: 这个架构是否过度设计？
**A**: 
- 对于当前规模可能略显复杂
- 但考虑到：
  - 学习价值高（掌握企业级架构模式）
  - 未来扩展性好（容易添加新功能）
  - 可测试性强（质量保障）
- 建议：**这是学习的好机会，值得投入！**

### Q3: 迁移需要多长时间？
**A**: 
- 按照上述计划：约 **2-3 周**（全职）
- 如果兼职：**1-2 个月**
- 可以根据实际情况调整优先级
- 建议先做 Phase 1-3（核心），Phase 4-6 可以延后

### Q4: 如何处理遗留代码？
**A**: 
- 不要一次性重写！
- 采用"绞杀者图模式"（Strangler Fig Pattern）
- 新功能用新架构，旧功能逐步替换
- 保持两套系统并行一段时间

---

## 🎉 总结

### 这套方案的优点
✅ **架构清晰**：职责分明，易于理解  
✅ **高度解耦**：各层独立演进  
✅ **易于测试**：Mock 友好，测试覆盖率高  
✅ **可扩展性强**：轻松添加新功能  
✅ **符合最佳实践**：业界公认的成熟模式  

### 开始行动的建议
1. **从 Phase 1（领域层）开始** - 这是基础
2. **边做边学** - 理论结合实践
3. **保持耐心** - 重构是个过程，不是一蹴而就
4. **及时记录** - 记录决策和遇到的问题

### 最后的话
> **"好的架构不是一开始就设计完美的，而是随着理解的深入不断演进的。"**  
> —— Robert C. Martin (Uncle Bob)

祝您重构顺利！如有任何问题，随时讨论。💪🚀

---

**文档版本**: v1.0  
**创建日期**: 2026-07-30  
**作者**: AI Assistant (GLM-5V-Turbo)  
**适用项目**: DesktopPet  
**许可协议**: 内部使用
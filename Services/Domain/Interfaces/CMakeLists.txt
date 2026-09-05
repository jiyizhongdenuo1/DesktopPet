# =============================================================================
# Interfaces Library
# =============================================================================
# 抽象接口定义（按入站/出站端口分类）

add_library(Interfaces STATIC
        # 入站端口 (Inbound Ports) - 驱动接口
        Inbound/CThreadHandler.h

        # 出站端口 (Outbound Ports) - 被驱动接口
#        IConfigPort.h
        Outbound/INoteDataBuffer.h

        # 基础类型/工具类（待迁移）
        CDataCollectBase.h
        
        # 旧版基类（待清理）
        IConfigBase.h
)

# 关键：使用 INTERFACE 传播给所有依赖者，让它们能直接 #include "xxx.h"
target_include_directories(Interfaces
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/Inbound   # 入站端口目录 → 可直接 #include "CThreadHandler.h"
        ${CMAKE_CURRENT_SOURCE_DIR}/Outbound  # 出站端口目录 → 可直接 #include "INoteDataBuffer.h"
        ${CMAKE_CURRENT_SOURCE_DIR}           # 根目录 → 可直接 #include "IConfigBase.h" 等
)

target_link_libraries(Interfaces
        PUBLIC
        CoreTypes
        NoteDomain
)
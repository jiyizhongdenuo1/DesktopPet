#!/bin/bash

# ============================================================
# @file: copy_useful_files.sh
# @brief: 拷贝项目源码到上一层目录（仅核心源码）
# @author: nuo
# @date: 2026/8/2
# @update: 
#   - 2026/8/18 - 适配新目录结构 + 排除编译产物
#   - 2026/9/10 - 排除测试代码、文档、IDE配置等非必要文件
#   - 2026/9/27 - 改为包含式拷贝 + 新增文档文件夹
# ============================================================

set -e

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DEST_DIR="${PROJECT_DIR}/../DesktopPet_Source"

echo "========================================"
echo "📦 开始拷贝 DesktopPet 源码"
echo "📍 源目录: ${PROJECT_DIR}"
echo "🎯 目标目录: ${DEST_DIR}"
echo "========================================"

# 清理并创建目标目录
rm -rf "${DEST_DIR}"
mkdir -p "${DEST_DIR}"

# 定义需要拷贝的目录（包含式）
COPY_DIRS=(
    'Bootstrapper'
    'Services'
    'Widgets'
    'config'
    'resource'
    'ExternalCode'
    '文档'
)

# Services 内部需要排除的子目录（测试代码）
SERVICE_EXCLUDES=(
    '--exclude=TestCode/'
)

# 定义需要拷贝的根目录文件
COPY_FILES=(
    'main.cpp'
    'CMakeLists.txt'
    'Autoqmldir'
)

# 1. 拷贝目录
idx=1
total=$((${#COPY_DIRS[@]} + ${#COPY_FILES[@]}))
for dir_name in "${COPY_DIRS[@]}"; do
    src="${PROJECT_DIR}/${dir_name}"
    if [ ! -d "${src}" ]; then
        echo "⚠️  [${idx}/${total}] 跳过不存在的目录: ${dir_name}/"
        idx=$((idx + 1))
        continue
    fi

    echo "📂 [${idx}/${total}] 拷贝 ${dir_name}/..."
    if [ "${dir_name}" = "Services" ]; then
        rsync -a "${SERVICE_EXCLUDES[@]}" "${src}" "${DEST_DIR}/" || true
    else
        rsync -a "${src}" "${DEST_DIR}/" || true
    fi
    idx=$((idx + 1))
done

# 2. 拷贝根目录文件
for file_name in "${COPY_FILES[@]}"; do
    src="${PROJECT_DIR}/${file_name}"
    if [ ! -f "${src}" ]; then
        echo "⚠️  [${idx}/${total}] 跳过不存在的文件: ${file_name}"
        idx=$((idx + 1))
        continue
    fi

    echo "📄 [${idx}/${total}] 拷贝 ${file_name}..."
    cp "${src}" "${DEST_DIR}/" 2>/dev/null || true
    idx=$((idx + 1))
done

# 输出结果
echo ""
echo "========================================"
echo "✅ 拷贝完成！"
echo "📁 目标位置: ${DEST_DIR}"
echo ""
echo "📊 复制统计："
echo "─────────────────────────────"
du -sh "${DEST_DIR}" 2>/dev/null || true
echo ""
echo "📁 各目录大小："
du -sh "${DEST_DIR}"/* 2>/dev/null | sort -hr | head -10
echo "─────────────────────────────"
echo ""
echo "✨ 已包含："
echo "  ✓ Bootstrapper/ (应用启动器)"
echo "  ✓ Services/ (核心服务层，不含 TestCode)"
echo "  ✓ Widgets/ (QML界面)"
echo "  ✓ config/ (配置文件)"
echo "  ✓ resource/ (资源文件)"
echo "  ✓ ExternalCode/ (外部代码)"
echo "  ✓ 文档/ (项目文档)"
echo "  ✓ 根目录构建文件"
echo ""
echo "❌ 已排除："
echo "  ✗ TestCode/ (测试代码)"
echo "  ✗ Scripts/ (工具脚本)"
echo "  ✗ bin/, build/ (编译产物)"
echo "  ✗ .qtcreator/, .git/ (IDE和版本控制)"
echo "  ✗ data/, RecordNoteData/ (运行时数据)"
echo "  ✗ win/, vulkan_win_headers/ (平台特定)"
echo "========================================"
#!/bin/bash

# ============================================================
# @file: copy_useful_files.sh
# @brief: 拷贝项目源码到上一层目录（仅核心源码）
# @author: nuo
# @date: 2026/8/2
# @update: 
#   - 2026/8/18 - 适配新目录结构 + 排除编译产物
#   - 2026/9/10 - 排除测试代码、文档、IDE配置等非必要文件
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

# 定义通用排除项（编译产物、版本控制、IDE配置、日志等）
COMMON_EXCLUDES=(
    'bin/'
    'build/'
    '.git/'
    '.gitignore'
    '.qtcreator/'
    '*.log'
    '*.o'
    '*.a'
    '*.so'
    '*.dylib'
    'CMakeFiles/'
    'CMakeCache.txt'
    'cmake_install.cmake'
    'Makefile'
)

# 构建 rsync 排除参数
EXCLUDE_ARGS=""
for exclude in "${COMMON_EXCLUDES[@]}"; do
    EXCLUDE_ARGS="${EXCLUDE_ARGS} --exclude='${exclude}'"
done

# 1. 复制核心代码目录
echo ""
echo "📂 [1/5] 复制 Bootstrapper..."
eval rsync -a ${EXCLUDE_ARGS} "${PROJECT_DIR}/Bootstrapper" "${DEST_DIR}/" || true

echo "📂 [2/5] 复制 Services（排除 TestCode）..."
eval rsync -a ${EXCLUDE_ARGS} --exclude='TestCode/' "${PROJECT_DIR}/Services" "${DEST_DIR}/" || true

echo "📂 [3/5] 复制 Widgets..."
eval rsync -a ${EXCLUDE_ARGS} "${PROJECT_DIR}/Widgets" "${DEST_DIR}/" || true

# 2. 复制配置文件
echo "📂 [4/5] 复制 config..."
cp -r "${PROJECT_DIR}/config" "${DEST_DIR}/" || true

# 3. 复制根目录必要文件
echo "📂 [5/5] 复制根目录文件..."
cp "${PROJECT_DIR}/main.cpp" "${DEST_DIR}/" 2>/dev/null || true
cp "${PROJECT_DIR}/CMakeLists.txt" "${DEST_DIR}/" 2>/dev/null || true
cp "${PROJECT_DIR}/Autoqmldir" "${DEST_DIR}/" 2>/dev/null || true

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
echo "  ✓ Services/ (核心服务层)"
echo "  ✓ Widgets/ (QML界面)"
echo "  ✓ config/ (配置文件)"
echo "  ✓ 根目录构建文件"
echo ""
echo "❌ 已排除："
echo "  ✗ TestCode/ (测试代码和GoogleTest)"
echo "  ✗ 文档/ (文档资料)"
echo "  ✗ Scripts/ (工具脚本)"
echo "  ✗ .qtcreator/ (IDE配置)"
echo "  ✗ bin/, build/ (编译产物)"
echo "  ✗ *.log, *.o, *.a (中间文件)"
echo "========================================"
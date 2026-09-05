#!/bin/bash

# ============================================================
# @file: copy_useful_files.sh
# @brief: 拷贝项目源码到上一层目录（仅源码，不含编译产物和测试）
# @author: nuo
# @date: 2026/8/2
# @update: 2026/8/18 - 适配新目录结构 + 排除编译产物
# ============================================================

set -e

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DEST_DIR="${PROJECT_DIR}/../DesktopPet_Source"

echo "开始拷贝..."

rm -rf "${DEST_DIR}"
mkdir -p "${DEST_DIR}"

# 1. 复制核心代码目录（排除 bin、build、.git 等编译产物）
echo "复制 Bootstrapper..."
rsync -a --exclude='bin/' --exclude='build/' --exclude='.git/' \
    "${PROJECT_DIR}/Bootstrapper" "${DEST_DIR}/" || true

echo "复制 Services..."
rsync -a --exclude='bin/' --exclude='build/' --exclude='.git/' \
    "${PROJECT_DIR}/Services" "${DEST_DIR}/" || true

echo "复制 Widgets..."
rsync -a --exclude='bin/' --exclude='build/' --exclude='.git/' \
    "${PROJECT_DIR}/Widgets" "${DEST_DIR}/" || true

echo "复制 config..."
cp -r "${PROJECT_DIR}/config" "${DEST_DIR}/" || true

echo "复制 文档..."
cp -r "${PROJECT_DIR}/文档" "${DEST_DIR}/" || true

echo "复制 Scripts..."
cp -r "${PROJECT_DIR}/Scripts" "${DEST_DIR}/" || true

# 2. 复制根目录文件
cp "${PROJECT_DIR}/main.cpp" "${DEST_DIR}/" || true
cp "${PROJECT_DIR}/CMakeLists.txt" "${DEST_DIR}/" || true
cp "${PROJECT_DIR}/.gitignore" "${DEST_DIR}/" || true
cp "${PROJECT_DIR}/Autoqmldir" "${DEST_DIR}/" || true

echo "✓ 完成！文件在: ${DEST_DIR}"
echo ""
echo "📊 复制统计："
du -sh "${DEST_DIR}" 2>/dev/null || true
du -sh "${DEST_DIR}"/* 2>/dev/null | sort -hr || true
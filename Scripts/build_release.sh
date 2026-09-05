#!/bin/bash

# ============================================================
# @file: build_release.sh
# @brief: 编译 Release 版本并打包
# @author: nuo
# @date: 2026/8/13
# ============================================================

set -e

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build_release"
OUTPUT_DIR="${PROJECT_DIR}/bin"
DIST_DIR="${PROJECT_DIR}/dist"
APP_NAME="DesktopPet"

echo "================================================"
echo "  ${APP_NAME} Release 构建脚本"
echo "================================================"
echo "项目目录: ${PROJECT_DIR}"
echo "构建目录: ${BUILD_DIR}"
echo ""

# 1. 清理旧的构建目录
echo "[1/5] 清理旧的构建目录..."
rm -rf "${BUILD_DIR}"
rm -rf "${OUTPUT_DIR}"
rm -rf "${DIST_DIR}"

# 2. CMake 配置 (Release)
echo "[2/5] CMake 配置 (Release)..."
cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${OUTPUT_DIR}"

# 3. 编译
echo "[3/5] 编译中..."
cmake --build "${BUILD_DIR}" --config Release -j$(nproc)

# 4. 安装
echo "[4/5] 安装到输出目录..."
cmake --install "${BUILD_DIR}" --config Release

# 5. 复制依赖并打包
echo "[5/5] 打包发布版本..."
mkdir -p "${DIST_DIR}"

# 复制可执行文件
cp "${OUTPUT_DIR}/bin/${APP_NAME}" "${DIST_DIR}/"

# 复制 config 文件夹
if [ -d "${PROJECT_DIR}/config" ]; then
    cp -r "${PROJECT_DIR}/config" "${DIST_DIR}/"
fi

# 复制 SaveFile 目录（如果存在）
if [ -d "${PROJECT_DIR}/bin/SaveFile" ]; then
    cp -r "${PROJECT_DIR}/bin/SaveFile" "${DIST_DIR}/"
fi

echo ""
echo "================================================"
echo "  构建完成！"
echo "================================================"
echo "发布目录: ${DIST_DIR}"
echo "可执行文件: ${DIST_DIR}/${APP_NAME}"
echo ""
ls -la "${DIST_DIR}"
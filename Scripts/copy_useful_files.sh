#!/bin/bash

# ============================================================
# @file: copy_useful_files.sh
# @brief: 拷贝项目源码到上一层目录
# @author: nuo
# @date: 2026/8/2
# ============================================================

set -e

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DEST_DIR="${PROJECT_DIR}/../DesktopPet_Source"

echo "开始拷贝..."

rm -rf "${DEST_DIR}"
mkdir -p "${DEST_DIR}"

cp -r "${PROJECT_DIR}/Bootstrapper" "${DEST_DIR}/"
cp -r "${PROJECT_DIR}/Services" "${DEST_DIR}/"
cp -r "${PROJECT_DIR}/Widgets" "${DEST_DIR}/"
cp -r "${PROJECT_DIR}/config" "${DEST_DIR}/"
cp -r "${PROJECT_DIR}/RequirementDoc" "${DEST_DIR}/"
cp -r "${PROJECT_DIR}/ArchitectureDesign" "${DEST_DIR}/"
cp -r "${PROJECT_DIR}/Scripts" "${DEST_DIR}/"

cp "${PROJECT_DIR}/main.cpp" "${DEST_DIR}/"
cp "${PROJECT_DIR}/CMakeLists.txt" "${DEST_DIR}/"
cp "${PROJECT_DIR}/.gitignore" "${DEST_DIR}/"

if [ -d "${PROJECT_DIR}/TestCode" ]; then
    mkdir -p "${DEST_DIR}/TestCode"
    cp -r "${PROJECT_DIR}/TestCode/GoogleTest" "${DEST_DIR}/TestCode/" 2>/dev/null || true
    cp -r "${PROJECT_DIR}/TestCode/TestDataMgr" "${DEST_DIR}/TestCode/" 2>/dev/null || true
    cp "${PROJECT_DIR}/TestCode/CMakeLists.txt" "${DEST_DIR}/TestCode/" 2>/dev/null || true
fi

rm -rf "${DEST_DIR}"/Services/*/bin "${DEST_DIR}"/Services/*/*/bin
rm -rf "${DEST_DIR}"/Services/*/build "${DEST_DIR}"/Services/*/*/build
rm -rf "${DEST_DIR}"/*/googletest-main

echo "✓ 完成！文件在: ${DEST_DIR}"

ls -la "${DEST_DIR}"
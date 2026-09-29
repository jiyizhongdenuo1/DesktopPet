#!/bin/bash

# ============================================================
# @file: build_release_win.sh
# @brief: 交叉编译 Windows 版本（Linux → Windows）
# @author: nuo
# @date: 2026/8/13
# ============================================================

set -e

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
WIN_DIR="${PROJECT_DIR}/win"
BUILD_DIR="${WIN_DIR}/build"
DIST_DIR="${WIN_DIR}/dist"
QT_DIR="${HOME}/Qt"
QT_VERSION="6.8.3"
APP_NAME="DesktopPet"

echo "================================================"
echo "  ${APP_NAME} Windows 交叉编译脚本"
echo "================================================"
echo "项目目录: ${PROJECT_DIR}"
echo ""

# 1. 检查 mingw-w64
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "[错误] 未安装 mingw-w64，请手动执行: sudo apt install -y mingw-w64"
    exit 1
else
    echo "[1/5] mingw-w64 已安装"
fi

# 2. 检查 Qt Windows 预编译包
if [ ! -d "${QT_DIR}/${QT_VERSION}/mingw_64" ]; then
    echo "[错误] Qt Windows 预编译包不存在: ${QT_DIR}/${QT_VERSION}/mingw_64"
    echo "        请手动执行: aqt install-qt windows desktop ${QT_VERSION} win64_mingw -O ${QT_DIR}"
    exit 1
fi
echo "[2/5] Qt Windows 预编译包已存在"

# 3. 生成 CMake toolchain 文件
TOOLCHAIN_FILE="${PROJECT_DIR}/toolchain_mingw.cmake"
echo "[3/5] 生成 CMake toolchain..."
cat > "${TOOLCHAIN_FILE}" << 'TOOLCHAIN_EOF'
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER  x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# 阻断宿主系统 include 路径泄漏到交叉编译
set(CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES "")
set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "")
TOOLCHAIN_EOF

# 4. 隔离 Vulkan 头文件（避免 /usr/include 泄漏）
VULKAN_DIR="${PROJECT_DIR}/vulkan_win_headers"
if [ -d /usr/include/vulkan ]; then
    rm -rf "${VULKAN_DIR}"
    mkdir -p "${VULKAN_DIR}/vulkan"
    cp /usr/include/vulkan/*.h "${VULKAN_DIR}/vulkan/"
    echo "[4/5] Vulkan 头文件已隔离到 ${VULKAN_DIR}"
else
    echo "[4/5] 未找到 Vulkan 头文件，跳过"
fi

# 5. CMake 配置 + 编译
echo "[5/5] CMake 配置 (Release) 并编译..."
rm -rf "${BUILD_DIR}"
cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
    -DCMAKE_PREFIX_PATH="${QT_DIR}/${QT_VERSION}/mingw_64" \
    -DQT_HOST_PATH="${QT_DIR}/${QT_VERSION}/gcc_64" \
    -DVulkan_INCLUDE_DIR="${VULKAN_DIR}"

cmake --build "${BUILD_DIR}" -j$(nproc)

# 8. 打包
echo ""
echo "打包中..."
rm -rf "${DIST_DIR}"
mkdir -p "${DIST_DIR}"

# 复制可执行文件
EXE_PATH="${BUILD_DIR}/bin/${APP_NAME}.exe"
if [ -f "${EXE_PATH}" ]; then
    cp "${EXE_PATH}" "${DIST_DIR}/"
fi

# 复制 config
if [ -d "${PROJECT_DIR}/config" ]; then
    cp -r "${PROJECT_DIR}/config" "${DIST_DIR}/"
fi

# 复制项目 DLL
echo "复制项目 DLL..."
cp "${BUILD_DIR}/bin"/*.dll "${DIST_DIR}/" 2>/dev/null || true

QT_DLL_DIR="${QT_DIR}/${QT_VERSION}/mingw_64/bin"

# 复制 MinGW 运行时 DLL（与编译工具链同源，ABI 一致）
cp /usr/lib/gcc/x86_64-w64-mingw32/13-win32/libgcc_s_seh-1.dll "${DIST_DIR}/"
cp /usr/lib/gcc/x86_64-w64-mingw32/13-win32/libstdc++-6.dll "${DIST_DIR}/"
cp /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll "${DIST_DIR}/"

# 复制 Qt 平台插件
QT_PLUGIN_DIR="${QT_DIR}/${QT_VERSION}/mingw_64/plugins"
mkdir -p "${DIST_DIR}/platforms"
cp "${QT_PLUGIN_DIR}/platforms/qwindows.dll" "${DIST_DIR}/platforms/"

# 复制 QML 模块
QT_QML_DIR="${QT_DIR}/${QT_VERSION}/mingw_64/qml"
mkdir -p "${DIST_DIR}/qml"
cp -r "${QT_QML_DIR}/QtQuick" "${DIST_DIR}/qml/"
# 删掉不需要的 Controls 子模块（节省 ~8MB）
rm -rf "${DIST_DIR}/qml/QtQuick/Controls/designer" "${DIST_DIR}/qml/QtQuick/Controls/FluentWinUI3"
rm -rf "${DIST_DIR}/qml/QtQuick/Dialogs" "${DIST_DIR}/qml/QtQuick/Effects" "${DIST_DIR}/qml/QtQuick/LocalStorage"
rm -rf "${DIST_DIR}/qml/QtQuick/Particles" "${DIST_DIR}/qml/QtQuick/Shapes" "${DIST_DIR}/qml/QtQuick/tooling" "${DIST_DIR}/qml/QtQuick/VectorImage"
cp -r "${QT_QML_DIR}/QtQml" "${QT_QML_DIR}/QtCore" "${QT_QML_DIR}/QtNetwork" "${QT_QML_DIR}/Qt" "${DIST_DIR}/qml/"
cp "${QT_QML_DIR}/builtins.qmltypes" "${QT_QML_DIR}/jsroot.qmltypes" "${DIST_DIR}/qml/" 2>/dev/null || true

# 复制所有 Qt6 DLL（避免遗漏依赖）
cp "${QT_DLL_DIR}/Qt6"*.dll "${DIST_DIR}/"

# 复制到共享文件夹
SHARE_DIR="/mnt/hgfs/ShareFile"
if [ -d "${SHARE_DIR}" ]; then
    rm -rf "${SHARE_DIR}/DesktopPet"
    cp -r "${DIST_DIR}" "${SHARE_DIR}/DesktopPet"
    echo "已复制到共享文件夹: ${SHARE_DIR}/DesktopPet"
else
    echo "共享文件夹不存在: ${SHARE_DIR}"
fi

echo ""
echo "================================================"
echo "  构建完成！"
echo "================================================"
echo "发布目录: ${DIST_DIR}"
echo ""
ls -la "${DIST_DIR}"
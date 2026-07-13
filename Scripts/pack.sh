#!/bin/bash

# 配置文件夹复制脚本
# 参数1: 源目录路径（项目根目录）
# 参数2: 目标目录路径（输出目录）

SOURCE_DIR="${1:-$(pwd)/../}"
DEST_DIR="${2:-$(pwd)/../bin/}"

# 确保目标目录存在
mkdir -p "$DEST_DIR"

# 检查源目录是否存在 config 文件夹
if [ -d "$SOURCE_DIR/config" ]; then
    # 检查目标目录是否已有 config
    if [ -d "$DEST_DIR/config" ]; then
        # 目标目录已存在，删除旧的
        rm -rf "$DEST_DIR/config"
    fi
    # 复制整个 config 文件夹
    cp -r "$SOURCE_DIR/config" "$DEST_DIR/"
    echo "[INFO] Copied config folder from $SOURCE_DIR/config to $DEST_DIR/config"
else
    echo "[WARNING] Source config folder not found at $SOURCE_DIR/config"
    echo "[INFO] Creating empty config folder in $DEST_DIR"
    mkdir -p "$DEST_DIR/config"
    echo "[INFO] Created empty config directory in $DEST_DIR/config"
fi


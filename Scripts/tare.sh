cat > ~/patch_jcef.sh << 'EOF'
#!/bin/bash
PLUGIN_DIR="$HOME/.local/share/JetBrains/CLion2026.2/marscode/lib"

JAR_FILE=""
for jar in "$PLUGIN_DIR"/*.jar; do
  if unzip -l "$jar" 2>/dev/null | grep -q "META-INF/plugin.xml"; then
    JAR_FILE="$jar"
    break
  fi
done

if [ -z "$JAR_FILE" ]; then
  echo "未找到包含 plugin.xml 的 JAR，请确认插件路径"
  exit 1
fi

echo "目标 JAR: $JAR_FILE"

unzip -o -q "$JAR_FILE" META-INF/plugin.xml -d /tmp/jcef_patch

if grep -q "com.intellij.modules.jcef" /tmp/jcef_patch/META-INF/plugin.xml; then
  echo "已经包含 JCEF 依赖，无需重复打补丁"
  exit 0
fi

sed -i '/<depends>com.intellij.modules.lang<\/depends>/a\    <depends>com.intellij.modules.jcef</depends>' /tmp/jcef_patch/META-INF/plugin.xml

cd /tmp/jcef_patch
zip -u "$JAR_FILE" META-INF/plugin.xml
echo "补丁完成，请重启 CLion"
EOF
chmod +x ~/patch_jcef.sh

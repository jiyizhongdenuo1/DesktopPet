#!/bin/bash
# ============================================================================
# DesktopPet 一键式测试脚本
# ============================================================================
#
# 功能：自动完成 CMake 配置 → 编译 → 执行测试 的完整流程
#
# 使用方法：
#   chmod +x run_tests.sh
#   ./run_tests.sh [选项]
#
# 选项：
#   -h, --help       显示帮助信息
#   -c, --clean      清理构建目录后重新构建
#   -v, --verbose    详细输出模式（显示编译命令）
#   -l, --legacy     启用传统 Qt Test 套件
#   --coverage       启用代码覆盖率分析
#   --filter=PATTERN 仅运行匹配的测试（支持 GTest 通配符）
#   --list           列出所有可用测试（不执行）
#   --no-color       禁用彩色输出
#
# 示例：
#   ./run_tests.sh                    # 标准运行
#   ./run_tests.sh -c                 # 清理后重新运行
#   ./run_tests.sh --filter="*Cache*" # 只运行缓存相关测试
#   ./run_tests.sh --coverage         # 启用覆盖率分析
#
# ============================================================================

set -e  # 遇到错误立即退出

# ============================================================================
# 颜色定义（可通过 --no-color 禁用）
# ============================================================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

USE_COLOR=true

# ============================================================================
# 配置变量
# ============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}/.."
BUILD_DIR="${SCRIPT_DIR}/build-test"
TEST_FILTER=""
CLEAN_BUILD=false
VERBOSE=false
LEGACY_TESTS=false
COVERAGE=false
LIST_ONLY=false
CMAKE_ARGS=""
CTEST_ARGS="--output-on-failure"

# ============================================================================
# 辅助函数
# ============================================================================

print_header() {
    if [ "$USE_COLOR" = true ]; then
        echo -e "\n${CYAN}====================================${NC}"
        echo -e "${CYAN}🧪 $1${NC}"
        echo -e "${CYAN}====================================${NC}\n"
    else
        echo ""
        echo "===================================="
        echo "🧪 $1"
        echo "===================================="
        echo ""
    fi
}

print_success() {
    if [ "$USE_COLOR" = true ]; then
        echo -e "  ${GREEN}✅ $1${NC}"
    else
        echo "  ✅ $1"
    fi
}

print_error() {
    if [ "$USE_COLOR" = true ]; then
        echo -e "  ${RED}❌ $1${NC}" >&2
    else
        echo "  ❌ $1" >&2
    fi
}

print_warning() {
    if [ "$USE_COLOR" = true ]; then
        echo -e "  ${YELLOW}⚠️  $1${NC}"
    else
        echo "  ⚠️  $1"
    fi
}

print_info() {
    if [ "$USE_COLOR" = true ]; then
        echo -e "  ${BLUE}ℹ️  $1${NC}"
    else
        echo "  ℹ️  $1"
    fi
}

show_help() {
    cat << EOF
${BOLD}DesktopPet 一键式测试脚本${NC}

${BOLD}用法:${NC}
    $0 [选项]

${BOLD}选项:${NC}
    -h, --help       显示此帮助信息
    -c, --clean      清理构建目录后重新构建
    -v, --verbose    详细输出模式（显示完整编译命令）
    -l, --legacy     启用传统 Qt Test (TestDataMgr) 套件
    --coverage       启用代码覆盖率分析（需要 GCC/Clang）
    --filter=PAT     仅运行名称匹配 PATTERN 的测试（GTest 通配符）
    --list           列出所有可用的测试（不实际执行）
    --no-color       禁用彩色输出

${BOLD}示例:${NC}
    $0                        标准配置、构建并运行所有测试
    $0 -c                     清理旧的构建，重新开始
    $0 --filter="*Cache*"     只运行 NoteDataCache 相关的测试
    $0 --filter="NoteData*::Update*"  更精确的过滤
    $0 --coverage             启用代码覆盖率分析
    $0 --list                 查看有哪些测试可以运行
    $0 -v                     显示详细的编译过程

${BOLD}环境变量:${NC}
    TEST_FILTER              同 --filter 参数
    DESKTOPPET_SOURCE_DIR    指定主项目源码路径（默认自动检测）
    CMAKE_BUILD_TYPE         构建类型（Debug/Release/RelWithDebInfo）

${BOLD}输出目录:${NC}
    构建产物: ${BUILD_DIR}/bin/
    测试结果: 构建过程中直接输出到终端

EOF
}

# ============================================================================
# 参数解析
# ============================================================================

parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--clean)
                CLEAN_BUILD=true
                shift
                ;;
            -v|--verbose)
                VERBOSE=true
                CMAKE_ARGS="${CMAKE_ARGS} --verbose"
                shift
                ;;
            -l|--legacy)
                LEGACY_TESTS=true
                shift
                ;;
            --coverage)
                COVERAGE=true
                shift
                ;;
            --filter=*)
                TEST_FILTER="${1#*=}"
                shift
                ;;
            --filter)
                shift
                if [[ -n "$1" ]]; then
                    TEST_FILTER="$1"
                    shift
                else
                    print_error "--filter 需要一个参数值"
                    exit 1
                fi
                ;;
            --list)
                LIST_ONLY=true
                shift
                ;;
            --no-color)
                USE_COLOR=false
                shift
                ;;
            *)
                print_error "未知参数: $1"
                show_help
                exit 1
                ;;
        esac
    done

    # 支持通过环境变量设置过滤器
    if [[ -z "$TEST_FILTER" && -n "$TEST_FILTER_ENV" ]]; then
        TEST_FILTER="$TEST_FILTER_ENV"
    fi
}

# ============================================================================
# 前置检查
# ============================================================================

check_prerequisites() {
    print_info "检查前置条件..."
    
    # 检查 CMake
    if ! command -v cmake &> /dev/null; then
        print_error "未找到 CMake！请先安装 CMake ≥ 3.28.3"
        exit 1
    fi
    print_success "CMake: $(cmake --version | head -n1)"
    
    # 检查 C++ 编译器
    if command -v g++ &> /dev/null; then
        COMPILER_VERSION=$(g++ --version | head -n1)
        print_success "编译器: ${COMPILER_VERSION}"
    elif command -v clang++ &> /dev/null; then
        COMPILER_VERSION=$(clang++ --version | head -n1)
        print_success "编译器: ${COMPILER_VERSION}"
    else
        print_warning "未找到 C++ 编译器！可能需要安装 g++ 或 clang++"
    fi
    
    # 检查主项目源码
    SOURCE_DIR="${DESKTOPPET_SOURCE_DIR:-$PROJECT_ROOT}"
    if [[ ! -f "${SOURCE_DIR}/CMakeLists.txt" ]]; then
        print_error "无法找到主项目 CMakeLists.txt"
        print_info "尝试路径: ${SOURCE_DIR}"
        print_info "请使用 DESKTOPPET_SOURCE_DIR 环境变量指定正确路径"
        exit 1
    fi
    print_success "主项目源码: ${SOURCE_DIR}"
    
    echo ""
}

# ============================================================================
# 主流程函数
# ============================================================================

clean_build_dir() {
    if [[ -d "$BUILD_DIR" ]]; then
        print_info "清理旧构建目录: ${BUILD_DIR}"
        rm -rf "${BUILD_DIR}"
        print_success "已清理"
    fi
}

configure_project() {
    print_header "步骤 1/3: 配置项目 (CMake)"
    
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    
    local CMAKE_OPTIONS=""
    
    # 基础选项
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DDESKTOPPET_SOURCE_DIR=${SOURCE_DIR}"
    
    # 构建类型
    if [[ -z "$CMAKE_BUILD_TYPE" ]]; then
        CMAKE_OPTIONS="${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Debug"
    fi
    
    # 可选功能
    if [[ "$LEGACY_TESTS" = true ]]; then
        CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBUILD_LEGACY_TESTS=ON"
        print_info "已启用: 传统 Qt Test 套件"
    fi
    
    if [[ "$COVERAGE" = true ]]; then
        CMAKE_OPTIONS="${CMAKE_OPTIONS} -DENABLE_COVERAGE=ON"
        print_info "已启用: 代码覆盖率分析"
    fi
    
    # 执行 CMake 配置
    if [[ "$VERBOSE" = true ]]; then
        print_info "执行命令: cmake ${SCRIPT_DIR} ${CMAKE_OPTIONS} ${CMAKE_ARGS}"
    fi
    
    if cmake "${SCRIPT_DIR}" ${CMAKE_OPTIONS} ${CMAKE_ARGS}; then
        print_success "配置成功"
    else
        print_error "配置失败！请检查上方错误信息"
        exit 1
    fi
    
    echo ""
}

build_project() {
    print_header "步骤 2/3: 编译项目"
    
    cd "${BUILD_DIR}"
    
    local BUILD_CMD="cmake --build . -j$(nproc)"
    
    if [[ "$VERBOSE" = true ]]; then
        BUILD_CMD="${BUILD_CMD} --verbose"
    fi
    
    if [[ "$VERBOSE" = true ]]; then
        print_info "执行命令: ${BUILD_CMD}"
    fi
    
    # 计时开始
    local START_TIME=$(date +%s)
    
    if eval ${BUILD_CMD}; then
        local END_TIME=$(date +%s)
        local DURATION=$((END_TIME - START_TIME))
        print_success "编译成功 (耗时: ${DURATION} 秒)"
    else
        print_error "编译失败！请检查上方错误信息"
        
        # 尝试提供常见问题建议
        print_warning "常见原因："
        print_warning "  1. Qt6 未找到 → 设置 CMAKE_PREFIX_PATH 或 PATH"
        print_warning "  2. 缺少依赖库 → 安装必要的开发包"
        print_warning "  3. 编译器版本过低 → 需要 C++20 支持"
        
        exit 1
    fi
    
    echo ""
}

run_tests() {
    cd "${BUILD_DIR}"
    
    if [[ "$LIST_ONLY" = true ]]; then
        print_header "列出所有可用测试"
        
        ctest -N
        
        print_info "使用 --filter='PATTERN' 选择特定测试运行"
        return
    fi
    
    print_header "步骤 3/3: 执行测试"
    
    local TEST_CMD="ctest"
    
    # 添加基础参数
    TEST_CMD="${TEST_CMD} ${CTEST_ARGS}"
    
    # 过滤器
    if [[ -n "$TEST_FILTER" ]]; then
        TEST_CMD="${TEST_CMD} -R '${TEST_FILTER}'"
        print_info "测试过滤器: ${TEST_FILTER}"
    fi
    
    # 详细输出
    if [[ "$VERBOSE" = true ]]; then
        TEST_CMD="${TEST_CMD} -V"
    fi
    
    if [[ "$VERBOSE" = true ]]; then
        print_info "执行命令: ${TEST_CMD}"
    fi
    
    # 计时开始
    local START_TIME=$(date +%s)
    
    # 执行测试
    if eval ${TEST_CMD}; then
        local END_TIME=$(date +%s)
        local DURATION=$((END_TIME - START_TIME))
        
        print_header "✨ 测试全部通过！"
        print_success "总耗时: ${DURATION} 秒"
        
        if [[ "$COVERAGE" = true ]]; then
            print_info "覆盖率报告已生成在: ${BUILD_DIR}/"
            print_info "使用以下命令查看详细报告："
            print_info "  gcovr -r ${SOURCE_DIR}/Services --html -o coverage.html"
        fi
        
        exit 0
    else
        local END_TIME=$(date +%s)
        local DURATION=$((END_TIME - START_TIME))
        
        print_error "存在失败的测试！"
        print_error "总耗时: ${DURATION} 秒"
        print_info "请查看上方的详细失败信息"
        
        exit 1
    fi
}

# ============================================================================
# 主入口
# ============================================================================

main() {
    # 解析命令行参数
    parse_args "$@"
    
    # 显示标题
    print_header "DesktopPet 单元测试套件"
    
    # 前置条件检查
    check_prerequisites
    
    # 可选：清理构建目录
    if [[ "$CLEAN_BUILD" = true ]]; then
        clean_build_dir
    fi
    
    # 步骤 1: CMake 配置
    configure_project
    
    # 步骤 2: 编译
    build_project
    
    # 步骤 3: 运行测试
    run_tests
}

# 执行主函数
main "$@"
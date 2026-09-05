/*
 * @file: CConfigManager.h
 * @brief: 配置管理应用服务
 * @detail: 
 *   负责配置的编排、翻译、协调
 *   持有系统配置(CCommonConfig)和用户配置(CUserConfig)的实例
 *   对外提供语义化的业务接口，内部路由到对应的配置实例
 *
 * @author: nuo
 * @date: 2026/9/1
 * @update: 2026/9/2 - 重构为直接包含配置实例
 */

#pragma once

#include <string>
#include <memory>
#include <vector>

#include "IConfigBase.h"
#include "SCategoryInfo.h"

class IConfigUpdater;
class IConfigEditor;

class CConfigManager
{
public:
    /** ***********************************************************
     * @brief       构造函数
     * @param[in]   pCommonConfig  系统配置实例（IConfigUpdater接口）
     * @param[in]   pUserConfig    用户配置实例（IConfigEditor接口）
     * @note        通过基类接口注入配置实例，遵循依赖倒置原则
     * @throw       std::invalid_argument  配置实例为空
     *************************************************************/
    explicit CConfigManager(
        std::shared_ptr<IConfigUpdater> pCommonConfig,
        std::shared_ptr<IConfigEditor> pUserConfig
    );
    
    /** ***********************************************************
     * @brief       析构函数
     *************************************************************/
    ~CConfigManager();
    
    /** ***********************************************************
     * @brief       获取配置管理器单例实例
     * @return      std::shared_ptr<CConfigManager>  配置管理器的共享指针
     * @note        线程安全的单例模式，全局唯一实例
     * @warning     必须先调用 Initialize() 初始化
     * @see         g_ConfigManager 宏可直接使用此单例
     *************************************************************/
    static std::shared_ptr<CConfigManager> GetInstance();
    
    /** ***********************************************************
     * @brief       初始化配置管理器单例（必须在首次使用前调用）
     * @param[in]   pCommonConfig  系统配置实例（IConfigUpdater接口）
     * @param[in]   pUserConfig    用户配置实例（IConfigEditor接口）
     * @note        通常在程序启动时调用一次
     *************************************************************/
    static void Initialize(
        std::shared_ptr<IConfigUpdater> pCommonConfig,
        std::shared_ptr<IConfigEditor> pUserConfig
    );

    // ========== 系统配置接口（语义化）==========
    
    /** ***********************************************************
     * @brief       获取语言设置（已翻译）
     * @return      std::string  如 "简体中文"、"English"
     * @note        从SYSTEM节读取language键，并翻译为可读文本
     *************************************************************/
    std::string GetLanguage();
    
    /** ***********************************************************
     * @brief       获取主题
     * @return      std::string  "light" / "dark"
     * @note        从UI节读取theme键，默认返回"light"
     *************************************************************/
    std::string GetTheme();
    
    /** ***********************************************************
     * @brief       设置主题
     * @param[in]   strTheme  主题名称（"light" 或 "dark"）
     * @throw       std::invalid_argument  无效的主题值
     * @note        设置后会通知主题变更监听者
     *************************************************************/
    void SetTheme(const std::string &strTheme);
    
    /** ***********************************************************
     * @brief       判断是否为调试模式
     * @return      true=调试模式，false=非调试模式
     * @note        综合判断日志级别和环境变量
     *************************************************************/
    bool IsDebugMode();
    
    /** ***********************************************************
     * @brief       获取应用版本号
     * @return      std::string  版本号字符串，如 "1.0.0"
     * @note        默认返回 "1.0.0"
     *************************************************************/
    std::string GetAppVersion();
    
    /** ***********************************************************
     * @brief       获取最大重试次数
     * @return      int  最大重试次数，默认为3
     *************************************************************/
    int GetMaxRetryCount();
    
    /** ***********************************************************
     * @brief       获取超时时间（秒）
     * @return      int  超时秒数，默认为30
     *************************************************************/
    int GetTimeoutSeconds();
    
    /** ***********************************************************
     * @brief       获取数据文件存储路径
     * @return      std::string  数据路径，如 "/home/user/data/"
     * @note        从 DATA 配置节读取 dataPath 键
     *************************************************************/
    std::string GetDataPath();

    // ========== 用户配置接口 ==========
    
    /** ***********************************************************
     * @brief       获取窗口位置
     * @param[out]  nX  输出X坐标
     * @param[out]  nY  输出Y坐标
     * @note        从UICONTENT节读取窗口位置配置
     *************************************************************/
    void GetWindowPosition(int& nX, int& nY);
    
    /** ***********************************************************
     * @brief       设置窗口位置
     * @param[in]   nX  X坐标（不能为负数）
     * @param[in]   nY  Y坐标（不能为负数）
     * @throw       std::out_of_range  坐标值为负数
     *************************************************************/
    void SetWindowPosition(int nX, int nY);
    
    /** ***********************************************************
     * @brief       获取背景图片路径
     * @return      std::string  背景图片路径
     * @note        自动补全相对路径前缀
     *************************************************************/
    std::string GetBackgroundPath();
    
    /** ***********************************************************
     * @brief       获取所有分类信息列表
     * @return      std::vector<SCategoryInfo>  分类信息向量
     *************************************************************/
    std::vector<SCategoryInfo> GetCategories();
    
    /** ***********************************************************
     * @brief       新增配置分类
     * @param[in]   info  分类信息结构体
     * @return      true=成功，false=同名分类已存在
     *************************************************************/
    bool AddCategory(const SCategoryInfo& info);
    
    /** ***********************************************************
     * @brief       删除配置分类
     * @param[in]   strName  要删除的分类名称
     * @return      true=成功，false=分类不存在
     *************************************************************/
    bool DeleteCategory(const std::string& strName);
    
    /** ***********************************************************
     * @brief       获取快捷键配置
     * @param[in]   strAction  操作名称（如 "saveNote"、"newNote"）
     * @return      std::string  快捷键字符串（如 "Ctrl+Enter"）
     *************************************************************/
    std::string GetShortcut(const std::string& strAction);
    
    /** ***********************************************************
     * @brief       设置快捷键配置
     * @param[in]   strAction  操作名称
     * @param[in]   strKey     快捷键字符串
     *************************************************************/
    void SetShortcut(const std::string& strAction, const std::string& strKey);

    // ========== 偏好设置 ==========
    
    /** ***********************************************************
     * @brief       判断是否启用自动保存
     * @return      true=已启用，false=未启用
     *************************************************************/
    bool IsAutoSaveEnabled();
    
    /** ***********************************************************
     * @brief       获取自动保存间隔（秒）
     * @return      int  保存间隔秒数
     *************************************************************/
    int GetAutoSaveInterval();

private:
    // ========== 内部辅助方法 ==========
    
    /** ***********************************************************
     * @brief       翻译语言代码为可读文本
     * @param[in]   strRawCode  原始语言代码（如 "zh-CN"、"en-US"）
     * @return      std::string  翻译后的文本（如 "简体中文"、"English"）
     *************************************************************/
    std::string TranslateLanguageCode(const std::string& strRawCode);
    
    /** ***********************************************************
     * @brief       验证主题值的有效性
     * @param[in]   strTheme  主题名称
     * @throw       std::invalid_argument  无效的主题值
     *************************************************************/
    void ValidateTheme(const std::string& strTheme);
    
    /** ***********************************************************
     * @brief       通知主题变更
     * @param[in]   strTheme  新主题名称
     * @note        可扩展为观察者模式，当前为空实现
     *************************************************************/
    void NotifyThemeChanged(const std::string& strTheme);

private:
    std::shared_ptr<IConfigUpdater> m_pCommonConfig;  ///< 系统配置接口（只读+更新）
    std::shared_ptr<IConfigEditor> m_pUserConfig;      ///< 用户配置接口（完整编辑）
    
    static std::shared_ptr<CConfigManager> s_pInstance;  ///< 单例实例
};

/** ***********************************************************
 * @brief       配置管理器全局访问宏
 * @usage       g_ConfigManager->GetTheme();
 * @see         CConfigManager::GetInstance()
 *************************************************************/
#define g_ConfigManager CConfigManager::GetInstance()
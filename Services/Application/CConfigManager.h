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
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>

#include "datatype.h"
#include "IConfigBase.h"
#include "DBackFun.h"

class IConfigUpdater;
class IConfigEditor;
struct SCategoryInfo;

class CConfigManager
{
public:
    enum E_BCFUN_TYPE
    {
        E_BCFUN_TYPE_THEME = 0,       ///< 主题变更回调
        E_BCFUN_TYPE_CATEGORY,         ///< 分类变更回调（新增/删除）
        E_BCFUN_TYPE_WINDOW_POS,       ///< 窗口位置变更回调
        E_BCFUN_TYPE_SHORTCUT,         ///< 快捷键变更回调
        E_BCFUN_TYPE_UICONFIG,         ///< UI配置变更回调
        E_BCFUN_TYPE_MAX               ///< 枚举最大值（用于边界检查）
    };
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
    std::string GetLanguage() const;

    /** ***********************************************************
     * @brief       获取主题
     * @return      std::string  "light" / "dark"
     * @note        从UI节读取theme键，默认返回"light"
     *************************************************************/
    std::string GetTheme() const;
    
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
    bool IsDebugMode() const;

    /** ***********************************************************
     * @brief       获取应用版本号
     * @return      std::string  版本号字符串，如 "1.0.0"
     * @note        默认返回 "1.0.0"
     *************************************************************/
    std::string GetAppVersion() const;

    /** ***********************************************************
     * @brief       获取最大重试次数
     * @return      int  最大重试次数，默认为3
     *************************************************************/
    int GetMaxRetryCount() const;

    /** ***********************************************************
     * @brief       获取超时时间（秒）
     * @return      int  超时秒数，默认为30
     *************************************************************/
    int GetTimeoutSeconds() const;

    /** ***********************************************************
     * @brief       获取数据文件存储路径
     * @return      std::string  数据路径，如 "/home/user/data/"
     * @note        从 DATA 配置节读取 dataPath 键
     *************************************************************/
    std::string GetDataPath() const;

    // ========== 用户配置接口 ==========
    
    /** ***********************************************************
     * @brief       获取窗口位置
     * @param[out]  nX  输出X坐标
     * @param[out]  nY  输出Y坐标
     * @note        从UICONTENT节读取窗口位置配置
     *************************************************************/
    void GetWindowPosition(int& nX, int& nY) const;
    
    /** ***********************************************************
     * @brief       设置窗口位置
     * @param[in]   nX  X坐标（不能为负数）
     * @param[in]   nY  Y坐标（不能为负数）
     * @throw       std::out_of_range  坐标值为负数
     *************************************************************/
    void SetWindowPosition(int nX, int nY);

    /** ***********************************************************
     * @brief       获取窗口尺寸
     * @param[out]  nWidth   输出宽度
     * @param[out]  nHeight  输出高度
     * @note        从UICONTENT节读取窗口尺寸配置
     *************************************************************/
    void GetWindowSize(int& nWidth, int& nHeight) const;

    /** ***********************************************************
     * @brief       判断窗口是否最大化
     * @return      true=最大化，false=正常
     *************************************************************/
    bool GetWindowIsMaximized() const;
    
    /** ***********************************************************
     * @brief       获取背景图片路径
     * @return      std::string  背景图片路径
     * @note        自动补全相对路径前缀
     *************************************************************/
    std::string GetBackgroundPath() const;

    /** ***********************************************************
     * @brief       获取所有分类信息列表
     * @return      std::vector<SCategoryInfo>  分类信息向量
     *************************************************************/
    std::vector<SCategoryInfo> GetCategories() const;
    
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
    std::string GetShortcut(const std::string& strAction) const;
    
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
    bool IsAutoSaveEnabled() const;

    /** ***********************************************************
     * @brief       获取自动保存间隔（秒）
     * @return      int  保存间隔秒数
     *************************************************************/
    int GetAutoSaveInterval() const;

    // ========== 统一配置设置接口 ==========

    /** ***********************************************************
     * @brief       设置系统配置值（通用接口）
     * @param[in]   strSection  配置节名称
     * @param[in]   strKey      配置键名
     * @param[in]   strValue    配置值
     * @return      true=设置成功，false=设置失败
     * @note        内部自动判断是否需要触发回调通知
     *************************************************************/
    bool SetSystemConfig(const std::string& strSection,
                         const std::string& strKey,
                         const std::string& strValue);

    /** ***********************************************************
     * @brief       设置用户配置值（通用接口）
     * @param[in]   strSection  配置节名称
     * @param[in]   strKey      配置键名
     * @param[in]   strValue    配置值
     * @return      true=设置成功，false=设置失败
     * @note        内部自动判断是否需要触发回调通知
     *************************************************************/
    bool SetUserConfig(const std::string& strSection,
                       const std::string& strKey,
                       const std::string& strValue);

    // ========== 回调注册接口 ==========
    
    /** ***********************************************************
     * @brief       注册配置变更回调函数
     * @param[in]   eType  回调类型（E_BCFUN_TYPE 枚举值）
     * @param[in]   func   回调函数
     * @return      true=注册成功，false=类型无效或回调已存在
     * @note        同一类型只能注册一个回调，重复注册会覆盖
     * @see         E_BCFUN_TYPE
     *************************************************************/
    bool RegisterCallback(E_BCFUN_TYPE eType, Func_ConfigChange func);
    
    /** ***********************************************************
     * @brief       注销配置变更回调函数
     * @param[in]   eType  要注销的回调类型
     * @return      true=注销成功，false=类型无效或未注册
     *************************************************************/
    bool UnregisterCallback(E_BCFUN_TYPE eType);

private:
    // ========== 私有辅助方法 ==========
    
    /** ***********************************************************
     * @brief       验证主题值的有效性
     * @param[in]   strTheme  主题名称
     * @return      true=有效，false=无效
     *************************************************************/
    bool IsValidTheme(const std::string& strTheme) const;
    
    /** ***********************************************************
     * @brief       判断配置项是否需要触发回调
     * @param[in]   strSection  配置节
     * @param[in]   strKey      配置键
     * @return      回调类型，E_BCFUN_TYPE_MAX 表示不需要
     *************************************************************/
    E_BCFUN_TYPE ShouldNotifyCallback(const std::string& strSection,
                                      const std::string& strKey = "") const;
    
    /** ***********************************************************
     * @brief       翻译语言代码为可读文本
     *************************************************************/
    std::string TranslateLanguageCode(const std::string& strRawCode) const;
    
    /** ***********************************************************
     * @brief       通知配置变更
     * @param[in]   eType  回调类型
     * @param[in]   param1  参数1
     * @param[in]   param2  参数2
     *************************************************************/
    void NotifyConfigChanged(E_BCFUN_TYPE eType, PARAM param1 = 0, PARAM param2 = 0);

private:
    std::shared_ptr<IConfigUpdater> m_pCommonConfig;  ///< 系统配置接口
    std::shared_ptr<IConfigEditor> m_pUserConfig;      ///< 用户配置接口
    std::unordered_map<E_BCFUN_TYPE, Func_ConfigChange> m_puomapFunc;  ///< 回调函数表
    static std::shared_ptr<CConfigManager> s_pInstance;  ///< 单例实例
};

/** ***********************************************************
 * @brief       配置管理器全局访问宏
 * @usage       g_ConfigManager->GetTheme();
 * @see         CConfigManager::GetInstance()
 *************************************************************/
#define g_ConfigManager CConfigManager::GetInstance()
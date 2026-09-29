/*
 * @file: CCommonConfig.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/14
 * @Detail:
 */

#pragma once

#include <string>
#include <memory>

#include "IConfigBase.h"

class CCommonConfigPrivate;

class CCommonConfig : public IConfigUpdater
{
public:
    /** ***********************************************************
     * @brief       构造函数
     * @note        初始化配置管理器的私有实现类
     *************************************************************/
    explicit CCommonConfig();

    /** ***********************************************************
     * @brief       析构函数
     * @note        释放配置管理器资源
     *************************************************************/
    ~CCommonConfig();

    /** ***********************************************************
     * @brief       更新系统配置项
     * @param[in]   strConfigKey    配置键名
     * @param[in]   strConfigValue  配置值
     * @return      void
     * @note        线程安全，修改后自动保存到配置文件
     *************************************************************/
    void UpdateSystemConfig(const std::string &strConfigKey, std::string &strConfigValue);

    /** ***********************************************************
     * @brief       更新业务配置项
     * @param[in]   strConfigKey    配置键名
     * @param[in]   strConfigValue  配置值
     * @return      void
     * @note        线程安全，修改后自动保存到配置文件
     *************************************************************/
    void UpdateBusinessConfig(const std::string &strConfigKey, std::string &strConfigValue);

    /** ***********************************************************
     * @brief       更新日志配置项
     * @param[in]   strConfigKey    配置键名
     * @param[in]   strConfigValue  配置值
     * @return      void
     * @note        线程安全，修改后自动保存到配置文件
     *************************************************************/
    void UpdateLogConfig(const std::string &strConfigKey, std::string &strConfigValue);

    /** ***********************************************************
     * @brief       更新数据配置项
     * @param[in]   strConfigKey    配置键名
     * @param[in]   strConfigValue  配置值
     * @return      void
     * @note        线程安全，修改后自动保存到配置文件
     *************************************************************/
    void UpdateDataConfig(const std::string &strConfigKey, std::string &strConfigValue);

    /** ***********************************************************
     * @brief       更新默认配置项
     * @param[in]   strConfigKey    配置键名
     * @param[in]   strConfigValue  配置值
     * @return      void
     * @note        线程安全，修改后自动保存到配置文件
     *************************************************************/
    void UpdateDefaultConfig(const std::string &strConfigKey, std::string &strConfigValue);

    /** ***********************************************************
     * @brief       更新UI配置项
     * @param[in]   strConfigKey    配置键名
     * @param[in]   strConfigValue  配置值
     * @return      void
     * @note        线程安全，修改后自动保存到配置文件
     *************************************************************/
    void UpdateUIConfig(const std::string &strConfigKey, std::string &strConfigValue);

    /** ***********************************************************
     * @brief       获取系统配置项
     * @param[in]   strConfigKey    配置键名
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     * @note        线程安全，键不存在时返回空字符串
     *************************************************************/
    void GetSystemConfig(const std::string &strConfigKey, std::string &strConfigValue) const;

    /** ***********************************************************
     * @brief       获取业务配置项
     * @param[in]   strConfigKey    配置键名
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     * @note        线程安全，键不存在时返回空字符串
     *************************************************************/
    void GetBusinessConfig(const std::string &strConfigKey, std::string &strConfigValue) const;

    /** ***********************************************************
     * @brief       获取日志配置项
     * @param[in]   strConfigKey    配置键名
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     * @note        线程安全，键不存在时返回空字符串
     *************************************************************/
    void GetLogConfig(const std::string &strConfigKey, std::string &strConfigValue) const;

    /** ***********************************************************
     * @brief       获取数据配置项
     * @param[in]   strConfigKey    配置键名
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     * @note        线程安全，键不存在时返回空字符串
     *************************************************************/
    void GetDataConfig(const std::string &strConfigKey, std::string &strConfigValue) const;

    /** ***********************************************************
     * @brief       获取默认配置项
     * @param[in]   strConfigKey    配置键名
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     * @note        线程安全，键不存在时返回空字符串
     *************************************************************/
    void GetDefaultConfig(const std::string &strConfigKey, std::string &strConfigValue) const;

    /** ***********************************************************
     * @brief       获取UI配置项
     * @param[in]   strConfigKey    配置键名
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     * @note        线程安全，键不存在时返回空字符串
     *************************************************************/
    void GetUIConfig(const std::string &strConfigKey, std::string &strConfigValue) const;

    // ========== 实现 IConfigReader 接口 ==========
    
    /** ***********************************************************
     * @brief       获取指定配置节和键的值
     * @param[in]   strSection  配置节名称（如 "System"、"Business" 等）
     * @param[in]   strKey      配置键名
     * @param[out]  strValue    输出配置值
     * @return      void
     * @note        线程安全，键不存在时返回空字符串
     * @see         IConfigReader::GetValue
     *************************************************************/
    void GetValue(
        const std::string &strSection,
        const std::string &strKey,
        std::string &strValue
    ) const override;
    
    /** ***********************************************************
     * @brief       检查指定配置节中是否存在某个键
     * @param[in]   strSection  配置节名称
     * @param[in]   strKey      配置键名
     * @return      bool        true-键存在，false-键不存在
     * @note        线程安全
     * @see         IConfigReader::HasKey
     *************************************************************/
    bool HasKey(
        const std::string &strSection,
        const std::string &strKey
    ) const override;

    // ========== 实现 IConfigUpdater 接口 ==========
    
    /** ***********************************************************
     * @brief       设置指定配置节和键的值
     * @param[in]   strSection  配置节名称（如 "System"、"Business" 等）
     * @param[in]   strKey      配置键名
     * @param[in]   strValue    要设置的配置值
     * @return      void
     * @note        线程安全，仅修改内存中的配置，不自动保存到文件
     * @see         IConfigUpdater::SetValue
     *************************************************************/
    void SetValue(
        const std::string &strSection,
        const std::string &strKey,
        const std::string &strValue
    ) override;
    
    /** ***********************************************************
     * @brief       将当前内存中的配置写入到配置文件
     * @return      bool    true-写入成功，false-写入失败
     * @note        线程安全，会覆盖原配置文件内容
     * @see         IConfigUpdater::Write2File
     *************************************************************/
    bool Write2File() override;
private:
    /** ***********************************************************
     * @brief       初始化配置管理器
     * @return      void
     * @note        加载配置文件，初始化各配置模块
     *************************************************************/
    void InitConfig();
 
private:
    std::unique_ptr<CCommonConfigPrivate> d_ptr;
};
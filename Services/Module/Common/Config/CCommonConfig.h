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


class CCommonConfigPrivate;
class CCommonConfig
{
public:
    explicit CCommonConfig();

    ~CCommonConfig();
    /** ***********************************************************
     * @brief       初始化配置管理器
     * @return      void
     * @note        加载配置文件，初始化各配置模块
     *************************************************************/
    void InitConfig();

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
    void GetSystemConfig(const std::string &strConfigKey, std::string &strConfigValue);

    /** ***********************************************************
     * @brief       获取业务配置项
     * @param[in]   strConfigKey    配置键名
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     * @note        线程安全，键不存在时返回空字符串
     *************************************************************/
    void GetBusinessConfig(const std::string &strConfigKey, std::string &strConfigValue);

    /** ***********************************************************
     * @brief       获取日志配置项
     * @param[in]   strConfigKey    配置键名
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     * @note        线程安全，键不存在时返回空字符串
     *************************************************************/
    void GetLogConfig(const std::string &strConfigKey, std::string &strConfigValue);

    /** ***********************************************************
     * @brief       获取数据配置项
     * @param[in]   strConfigKey    配置键名
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     * @note        线程安全，键不存在时返回空字符串
     *************************************************************/
    void GetDataConfig(const std::string &strConfigKey, std::string &strConfigValue);

    /** ***********************************************************
     * @brief       获取默认配置项
     * @param[in]   strConfigKey    配置键名
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     * @note        线程安全，键不存在时返回空字符串
     *************************************************************/
    void GetDefaultConfig(const std::string &strConfigKey, std::string &strConfigValue);

    /** ***********************************************************
     * @brief       获取UI配置项
     * @param[in]   strConfigKey    配置键名
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     * @note        线程安全，键不存在时返回空字符串
     *************************************************************/
    void GetUIConfig(const std::string &strConfigKey, std::string &strConfigValue);
private:
    std::unique_ptr<CCommonConfigPrivate> d_ptr;
};
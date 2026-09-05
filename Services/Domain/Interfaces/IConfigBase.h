/*
 * @file: IConfigBase.h
 * @brief: 配置管理接口（出站端口）
 * @author: nuo
 * @date: 2026/8/30
 * @Detail: 
 *   采用分层设计，遵循接口隔离原则(ISP)：
 *   - IConfigReader: 基础只读操作（所有配置类必须实现）
 *   - IConfigUpdater: 更新操作（用于可修改配置值的场景，如CommonConfig）
 *   - IConfigEditor: 完整编辑操作（用于需要增删配置的场景，如UserConfig）
 * 
 * 架构位置:
 *   - 层级: Domain Layer (领域层)
 *   - 类型: Outbound Port (出站端口)
 *   - 实现: Infrastructure Layer (基础设施层)
 */

#pragma once

#include <string>
#include <vector>

// 前向声明
struct SCategoryInfo;

/**
 * @class IConfigReader
 * @brief 配置读取接口（基础出站端口）
 * @detail 定义配置的只读操作，所有配置管理器都必须实现
 */
class IConfigReader
{
public:
    virtual ~IConfigReader() = default;

    /** ***********************************************************
     * @brief       读取配置值
     * @param[in]   strSection  配置分组（如 "UI"、"SYSTEM"）
     * @param[in]   strKey      配置键名
     * @param[out]  strValue    输出配置值
     * @return      void
     * @note        键不存在时返回空字符串
     *************************************************************/
    virtual void GetValue(
        const std::string &strSection,
        const std::string &strKey,
        std::string &strValue
    ) const = 0;

    /** ***********************************************************
     * @brief       检查配置项是否存在
     * @param[in]   strSection  配置分组
     * @param[in]   strKey      配置键名
     * @return      bool  true=存在, false=不存在
     *************************************************************/
    virtual bool HasKey(
        const std::string &strSection,
        const std::string &strKey
    ) const = 0;
};

/**
 * @class IConfigUpdater
 * @brief 配置更新接口（可选扩展）
 * @detail 在读取基础上增加更新能力，用于需要修改配置值的场景
 */
class IConfigUpdater : public IConfigReader
{
public:
    ~IConfigUpdater() override = default;

    /** ***********************************************************
     * @brief       设置/更新配置值
     * @param[in]   strSection  配置分组
     * @param[in]   strKey      配置键名
     * @param[in]   strValue    要设置的值
     * @return      void
     * @note        键存在则更新，不存在则创建（取决于子类实现）
     * @note        通常会自动调用Write2File持久化
     *************************************************************/
    virtual void SetValue(
        const std::string &strSection,
        const std::string &strKey,
        const std::string &strValue
    ) = 0;

    /** ***********************************************************
     * @brief       将内存数据持久化到文件
     * @return      bool  true=成功, false=失败
     *************************************************************/
    virtual bool Write2File() = 0;
};

/**
 * @class IConfigEditor
 * @brief 配置编辑接口（完整编辑能力）
 * @detail 在更新基础上增加增删操作，用于需要完整编辑配置的场景
 */
class IConfigEditor : public IConfigUpdater
{
public:
    ~IConfigEditor() override = default;

    /** ***********************************************************
     * @brief       新增配置项（仅当键不存在时创建）
     * @param[in]   strSection  配置分组
     * @param[in]   strKey      配置键名
     * @param[in]   strValue    要添加的值
     * @return      bool  true=新增成功, false=键已存在
     *************************************************************/
    virtual bool AddValue(
        const std::string &strSection,
        const std::string &strKey,
        const std::string &strValue
    ) = 0;

    /** ***********************************************************
     * @brief       删除配置项
     * @param[in]   strSection  配置分组
     * @param[in]   strKey      配置键名
     * @return      bool  true=删除成功, false=键不存在
     *************************************************************/
    virtual bool DeleteValue(
        const std::string &strSection,
        const std::string &strKey
    ) = 0;

    // ========== 分类管理接口 ==========

    /** ***********************************************************
     * @brief       获取所有分类信息列表
     * @param[out]  vecCategories   输出分类信息向量
     * @return      void
     * @note        返回所有已定义的分类信息（名称、类型、图标等）
     *************************************************************/
    virtual void GetCategories(std::vector<struct SCategoryInfo> &vecCategories) = 0;
    
    /** ***********************************************************
     * @brief       新增一个配置分类
     * @param[in]   categoryInfo    分类信息结构体
     * @return      bool            true=新增成功，false=同名分类已存在或参数无效
     *************************************************************/
    virtual bool AddCategory(const struct SCategoryInfo &categoryInfo) = 0;
    
    /** ***********************************************************
     * @brief       删除指定名称的配置分类
     * @param[in]   strName         要删除的分类名称
     * @return      bool            true=删除成功，false=分类不存在
     *************************************************************/
    virtual bool DeleteCategory(const std::string &strName) = 0;

};
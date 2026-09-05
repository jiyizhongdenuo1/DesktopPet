/*
 * @file: CUserConfig.h
 * @brief: 用户配置管理接口
 * @author: nuo
 * @date: 2026/8/27
 * @Detail:
 */

#pragma once

#include  <memory>
#include <vector>
#include "DConfig.h"
#include "SCategoryInfo.h"
#include "IConfigBase.h"

class CUserConfigPrivate;

class CUserConfig : public IConfigEditor
{
public:
    /** ***********************************************************
     * @brief       构造函数
     * @note        初始化用户配置管理器的私有实现类（Pimpl模式）
     *************************************************************/
    explicit CUserConfig();

    /** ***********************************************************
     * @brief       析构函数
     * @note        释放用户配置管理器资源，自动保存未持久化的修改
     *************************************************************/
    ~CUserConfig();

    // ========== 实现 IConfigReader 接口 ==========
    
    /** ***********************************************************
     * @brief       读取配置值
     * @param[in]   strSection  配置分组（如 "UICONTENT"）
     * @param[in]   strKey      配置键名
     * @param[out]  strValue    输出配置值
     * @return      void
     * @note        键不存在时返回空字符串
     *************************************************************/
    void GetValue(const std::string &strSection,
                 const std::string &strKey,
                 std::string &strValue) const override;
    
    /** ***********************************************************
     * @brief       检查配置项是否存在
     * @param[in]   strSection  配置分组
     * @param[in]   strKey      配置键名
     * @return      bool  true=存在, false=不存在
     *************************************************************/
    bool HasKey(const std::string &strSection,
               const std::string &strKey) const override;

    // ========== 实现 IConfigUpdater 接口 ==========
    
    /** ***********************************************************
     * @brief       设置/更新配置值
     * @param[in]   strSection  配置分组
     * @param[in]   strKey      配置键名
     * @param[in]   strValue    要设置的值
     * @return      void
     * @note        键存在则更新，不存在则创建（取决于子类实现）
     * @note        通常会自动调用Write2File持久化
     *************************************************************/
    void SetValue(const std::string &strSection,
                 const std::string &strKey,
                 const std::string &strValue) override;
    
    /** ***********************************************************
     * @brief       将内存数据持久化到文件
     * @return      bool  true=成功, false=失败
     *************************************************************/
    bool Write2File() override;

    // ========== 实现 IConfigEditor 接口 ==========
    
    /** ***********************************************************
     * @brief       新增配置项（仅当键不存在时创建）
     * @param[in]   strSection  配置分组
     * @param[in]   strKey      配置键名
     * @param[in]   strValue    要添加的值
     * @return      bool  true=新增成功, false=键已存在
     *************************************************************/
    bool AddValue(const std::string &strSection,
                 const std::string &strKey,
                 const std::string &strValue) override;
    
    /** ***********************************************************
     * @brief       删除配置项
     * @param[in]   strSection  配置分组
     * @param[in]   strKey      配置键名
     * @return      bool  true=删除成功, false=键不存在
     *************************************************************/
    bool DeleteValue(const std::string &strSection,
                    const std::string &strKey) override;
    
    // ========== 分类相关接口方法 ==========
    
    /** ***********************************************************
     * @brief       获取所有分类信息列表
     * @param[out]  vecCategories   输出分类信息向量，包含所有已定义的分类
     * @return      void
     * @note        线程安全，返回的分类信息包含名称、类型、图标等属性
     * @see         IConfigEditor::GetCategories
     *************************************************************/
    void GetCategories(std::vector<SCategoryInfo> &vecCategories) override;
    
    /** ***********************************************************
     * @brief       新增一个配置分类
     * @param[in]   categoryInfo    分类信息结构体（名称、类型、图标路径等）
     * @return      bool            true=新增成功，false=同名分类已存在或参数无效
     * @note        新增后自动持久化到配置文件
     * @see         IConfigEditor::AddCategory
     *************************************************************/
    bool AddCategory(const SCategoryInfo &categoryInfo) override;
    
    /** ***********************************************************
     * @brief       删除指定名称的配置分类
     * @param[in]   strName         要删除的分类名称
     * @return      bool            true=删除成功，false=分类不存在
     * @note        删除后自动持久化到配置文件，关联的配置项也会被清理
     * @see         IConfigEditor::DeleteCategory
     *************************************************************/
    bool DeleteCategory(const std::string &strName) override;

    // ========== 便利方法（保留原有接口）==========
    
    /** ***********************************************************
     * @brief       获取UI内容配置项（分类列表）
     * @param[out]  vecCategoryInfo  输出分类信息向量
     * @return      void
     * @note        返回所有已定义的分类信息
     *************************************************************/
    void GetUIContentConfig(std::vector<SCategoryInfo> &vecCategoryInfo) const;

    /** ***********************************************************
     * @brief       获取UICONTENT下的通用配置项
     * @param[in]   strConfigKey    配置键名
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     * @note        用于获取UICONTENT Section下除数组外的普通配置项
     *************************************************************/
    void GetUserConfig(const std::string &strConfigKey, 
                      std::string &strConfigValue) const;

    /** ***********************************************************
     * @brief       获取窗口配置项
     * @param[in]   strConfigKey    配置键名（positionX/positionY/width/height/isMaximized/backgroundPath）
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     *************************************************************/
    void GetWindowConfig(const std::string &strConfigKey, 
                        std::string &strConfigValue) const;

    /** ***********************************************************
     * @brief       获取快捷键配置项
     * @param[in]   strConfigKey    配置键名（saveNote/newNote/deleteNote/searchNotes/toggleCategory）
     * @param[out]  strConfigValue  输出配置值（如 "Ctrl+Enter"）
     * @return      void
     *************************************************************/
    void GetShortcutConfig(const std::string &strConfigKey, 
                          std::string &strConfigValue) const;

    /** ***********************************************************
     * @brief       获取偏好设置配置项
     * @param[in]   strConfigKey    配置键名（language/theme/defaultCategoryType/autoSaveEnabled/autoSaveIntervalSeconds）
     * @param[out]  strConfigValue  输出配置值
     * @return      void
     *************************************************************/
    void GetPreferencesConfig(const std::string &strConfigKey, 
                             std::string &strConfigValue) const;

private:
    /** ***********************************************************
     * @brief       初始化配置管理器
     * @return      void
     * @note        加载配置文件，初始化各配置模块
     *************************************************************/
    void InitConfig();

private:
    std::unique_ptr<CUserConfigPrivate> d_ptr;
};
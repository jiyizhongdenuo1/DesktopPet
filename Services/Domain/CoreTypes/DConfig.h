/*
 * @file: DConfig.h
 * @brief: 配置常量定义
 * @author: nuo
 * @date: 2026/6/21
 * @Detail:
 */

#pragma once

#include <cstdint>

namespace NConfigSection
{
    constexpr char const DEFAULT_SECTION[] = "DEFAULT";
    constexpr char const SYSTEM_SECTION[] = "SYSTEM";
    constexpr char const LOG_SECTION[] = "LOG";
    constexpr char const DATA_SECTION[] = "DATA";
    constexpr char const UI_SECTION[] = "UI";
    constexpr char const DATA_SAVE_FILE_SECTION[] = "DATA_SAVE_FILE";
    constexpr char const BUSINESS_SECTION[] = "BUSINESS";
    constexpr char const UICONTENT_SECTION[] = "UICONTENT";

}

namespace NCommonConfig
{
    // DEFAULT Section
    constexpr char const COMMON_CONFIG_LANGUAGE[] = "language";
    constexpr char const COMMON_CONFIG_THEME[] = "theme";
    constexpr char const COMMON_CONFIG_FONT_SIZE[] = "fontSize";
    constexpr char const COMMON_CONFIG_AUTO_UPDATE[] = "autoUpdate";
    constexpr char const COMMON_CONFIG_CHECK_UPDATE_INTERVAL[] = "checkUpdateInterval";

    // SYSTEM Section
    constexpr char const COMMON_CONFIG_VERSION[] = "version";
    constexpr char const COMMON_CONFIG_APP_NAME[] = "appName";
    constexpr char const COMMON_CONFIG_AUTHOR[] = "author";
    constexpr char const COMMON_CONFIG_CREATE_DATE[] = "createDate";
    constexpr char const COMMON_CONFIG_LAST_UPDATE[] = "lastUpdate";
    constexpr char const COMMON_CONFIG_AUTO_SAVE_INTERVAL[] = "autoSaveInterval";
    constexpr char const COMMON_CONFIG_MAX_BACKUP_COUNT[] = "maxBackupCount";

    // LOG Section
    constexpr char const COMMON_CONFIG_ENABLE_LOG[] = "enableLog";
    constexpr char const COMMON_CONFIG_LOG_LEVEL[] = "logLevel";
    constexpr char const COMMON_CONFIG_LOG_PATH[] = "logPath";
    constexpr char const COMMON_CONFIG_MAX_LOG_FILE_SIZE[] = "maxLogFileSize";
    constexpr char const COMMON_CONFIG_MAX_LOG_FILE_COUNT[] = "maxLogFileCount";
    constexpr char const COMMON_CONFIG_ENABLE_CONSOLE_OUTPUT[] = "enableConsoleOutput";

    // DATA Section
    constexpr char const COMMON_CONFIG_DATA_PATH[] = "dataPath";
    constexpr char const COMMON_CONFIG_FILE_EXTENSION[] = "fileExtension";
    constexpr char const COMMON_CONFIG_BACKUP_PATH[] = "backupPath";
    constexpr char const COMMON_CONFIG_ENABLE_COMPRESSION[] = "enableCompression";
    constexpr char const COMMON_CONFIG_ENCRYPTION_ENABLED[] = "encryptionEnabled";

    // UI Section
    constexpr char const COMMON_CONFIG_WINDOW_WIDTH[] = "windowWidth";
    constexpr char const COMMON_CONFIG_WINDOW_HEIGHT[] = "windowHeight";
    constexpr char const COMMON_CONFIG_WINDOW_X[] = "windowX";
    constexpr char const COMMON_CONFIG_WINDOW_Y[] = "windowY";
    constexpr char const COMMON_CONFIG_IS_MAXIMIZED[] = "isMaximized";
    constexpr char const COMMON_CONFIG_SHOW_TOOLBAR[] = "showToolbar";
    constexpr char const COMMON_CONFIG_SHOW_STATUS_BAR[] = "showStatusBar";
    constexpr char const COMMON_CONFIG_ANIMATION_ENABLED[] = "animationEnabled";
    constexpr char const COMMON_CONFIG_NOTE_CATEGORIES[] = "noteCategories";

    // BUSINESS Section
    constexpr char const COMMON_CONFIG_ENABLE_AUTO_SAVE[] = "enableAutoSave";
    constexpr char const COMMON_CONFIG_ENABLE_SYNC[] = "enableSync";
    constexpr char const COMMON_CONFIG_DEFAULT_NOTE_TYPE[] = "defaultNoteType";
    constexpr char const COMMON_CONFIG_MAX_NOTE_COUNT_PER_FILE[] = "maxNoteCountPerFile";
    constexpr char const COMMON_CONFIG_DATA_RETENTION_DAYS[] = "dataRetentionDays";
    constexpr char const COMMON_CONFIG_MAX_RETRY_COUNT[] = "maxRetryCount";
    constexpr char const COMMON_CONFIG_TIMEOUT_SECONDS[] = "timeoutSeconds";

    // SYSTEM Section (补充)
    constexpr char const COMMON_CONFIG_ENVIRONMENT[] = "environment";
}

namespace NUserConfig
{
    // UICONTENT Section - Window Config
    constexpr char const USER_CONFIG_WINDOW_POSITION_X[] = "windowConfig.positionX";
    constexpr char const USER_CONFIG_WINDOW_POSITION_Y[] = "windowConfig.positionY";
    constexpr char const USER_CONFIG_WINDOW_WIDTH[] = "windowConfig.width";
    constexpr char const USER_CONFIG_WINDOW_HEIGHT[] = "windowConfig.height";
    constexpr char const USER_CONFIG_WINDOW_IS_MAXIMIZED[] = "windowConfig.isMaximized";
    constexpr char const USER_CONFIG_WINDOW_BACKGROUND_PATH[] = "windowConfig.backgroundPath";

    // UICONTENT Section - Shortcuts
    constexpr char const USER_CONFIG_SHORTCUTS_PREFIX[] = "shortcuts.";
    constexpr char const USER_CONFIG_SHORTCUT_SAVE_NOTE[] = "shortcuts.saveNote";
    constexpr char const USER_CONFIG_SHORTCUT_NEW_NOTE[] = "shortcuts.newNote";
    constexpr char const USER_CONFIG_SHORTCUT_DELETE_NOTE[] = "shortcuts.deleteNote";
    constexpr char const USER_CONFIG_SHORTCUT_SEARCH_NOTES[] = "shortcuts.searchNotes";
    constexpr char const USER_CONFIG_SHORTCUT_TOGGLE_CATEGORY[] = "shortcuts.toggleCategory";

    // UICONTENT Section - Preferences
    constexpr char const USER_CONFIG_PREFERENCES_PREFIX[] = "preferences.";
    constexpr char const USER_CONFIG_PREFERENCES_LANGUAGE[] = "preferences.language";
    constexpr char const USER_CONFIG_PREFERENCES_THEME[] = "preferences.theme";
    constexpr char const USER_CONFIG_PREFERENCES_DEFAULT_CATEGORY_TYPE[] = "preferences.defaultCategoryType";
    constexpr char const USER_CONFIG_PREFERENCES_AUTO_SAVE_ENABLED[] = "preferences.autoSaveEnabled";
    constexpr char const USER_CONFIG_PREFERENCES_AUTO_SAVE_INTERVAL_SECONDS[] = "preferences.autoSaveIntervalSeconds";
}
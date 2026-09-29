 /*
 * @file: CMainConfigModel.h
 * @brief: 
 * @author: nuo
 * @date: 2026/9/18
 * @Detail:
 */

#pragma once
#include <memory>
#include <QObject>

class CMainConfigModelPrivate;
class CMainConfigModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int  windowWidth     READ windowWidth   WRITE setWindowWidth     NOTIFY configChanged)
    Q_PROPERTY(int  windowHeight    READ windowHeight  WRITE setWindowHeight    NOTIFY configChanged)
    Q_PROPERTY(int  windowX         READ windowX       WRITE setWindowX         NOTIFY configChanged)
    Q_PROPERTY(int  windowY         READ windowY       WRITE setWindowY         NOTIFY configChanged)
    Q_PROPERTY(int  windowMaximized READ windowMaximized                        NOTIFY configChanged)
    Q_PROPERTY(QString shortcutSave           READ shortcutSave           WRITE setShortcutSave           NOTIFY configChanged)
    Q_PROPERTY(QString shortcutNew            READ shortcutNew            WRITE setShortcutNew            NOTIFY configChanged)
    Q_PROPERTY(QString shortcutDelete         READ shortcutDelete         WRITE setShortcutDelete         NOTIFY configChanged)
    Q_PROPERTY(QString shortcutSearch         READ shortcutSearch         WRITE setShortcutSearch         NOTIFY configChanged)
    Q_PROPERTY(QString shortcutToggleCategory READ shortcutToggleCategory WRITE setShortcutToggleCategory NOTIFY configChanged)

public:
    explicit CMainConfigModel();

    ~CMainConfigModel();

    int  windowWidth() const;
    int  windowHeight() const;
    int  windowX() const;
    int  windowY() const;
    int  setWindowWidth(int s32Width) const;
    int  setWindowHeight(int s32Height) const;
    int  setWindowX(int s32X) const;
    int  setWindowY(int s32Y) const;

    int windowMaximized() const;

    QString shortcutSave() const;
    QString shortcutNew() const;
    QString shortcutDelete() const;
    QString shortcutSearch() const;
    QString shortcutToggleCategory() const;

    void setShortcutSave(const QString& strKey);
    void setShortcutNew(const QString& strKey);
    void setShortcutDelete(const QString& strKey);
    void setShortcutSearch(const QString& strKey);
    void setShortcutToggleCategory(const QString& strKey);

signals:
    void configChanged();
private:
    std::shared_ptr<CMainConfigModelPrivate> d_ptr;
};

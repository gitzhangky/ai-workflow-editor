#pragma once

#include <QListWidget>
#include <QPixmap>
#include <QRectF>

class NodeLibraryListWidget : public QListWidget
{
    Q_OBJECT

public:
    enum class ItemKind
    {
        SectionHeader = 0,
        NodeEntry = 1
    };

    enum class CardVisualState
    {
        Standalone = 0,
        SectionTop = 1,
        NodeMiddle = 2,
        NodeBottom = 3
    };

    static inline const char *MimeType = "application/x-ai-workflow-node-type";
    static constexpr int NodeTypeRole = Qt::UserRole + 1;
    static constexpr int ItemKindRole = Qt::UserRole + 2;
    static constexpr int SectionCollapsedRole = Qt::UserRole + 3;
    static constexpr int CardVisualStateRole = Qt::UserRole + 4;

    explicit NodeLibraryListWidget(QWidget *parent = nullptr);
    static QPixmap dragPreviewPixmap();
    static QRectF alignedCardRect(QRectF const &rect);
    QListWidgetItem *addSectionHeader(QString const &title, QIcon const &icon);
    QListWidgetItem *addNodeEntry(QString const &typeKey,
                                  QString const &displayName,
                                  QString const &description,
                                  QIcon const &icon);
    void setFilterText(QString const &filterText);
    QString filterText() const;
    void toggleSection(QListWidgetItem *headerItem);

protected:
    QStringList mimeTypes() const override;
    QMimeData *mimeData(QList<QListWidgetItem *> items) const override;
    Qt::DropActions supportedDragActions() const;
    void startDrag(Qt::DropActions supportedActions) override;

private:
    void applyVisibility();

    QString _filterText;
};

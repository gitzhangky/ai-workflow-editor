#pragma once

#include <QColor>
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
    static constexpr int InPortCountRole = Qt::UserRole + 5;
    static constexpr int OutPortCountRole = Qt::UserRole + 6;

    explicit NodeLibraryListWidget(QWidget *parent = nullptr);
    static QPixmap dragPreviewPixmap();
    static QRectF alignedCardRect(QRectF const &rect);
    static int scrollIndicatorWidth();
    static qreal scrollIndicatorCornerRadius();
    static QColor scrollIndicatorTrackColor();
    static QColor scrollIndicatorThumbColor();
    QListWidgetItem *addSectionHeader(QString const &title, QIcon const &icon);
    QListWidgetItem *addNodeEntry(QString const &typeKey,
                                  QString const &displayName,
                                  QString const &description,
                                  QIcon const &icon,
                                  int inPortCount = 0,
                                  int outPortCount = 0);
    void setFilterText(QString const &filterText);
    QString filterText() const;
    int visibleNodeCount() const;
    void toggleSection(QListWidgetItem *headerItem);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(QList<QListWidgetItem *> items) const override;
    Qt::DropActions supportedDragActions() const;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void startDrag(Qt::DropActions supportedActions) override;

private:
    void applyVisibility();
    void queueScrollIndicatorRefresh();
    void refreshScrollIndicator();
    void updateScrollIndicatorGeometry();

    QString _filterText;
    QWidget *_scrollIndicator;
    bool _viewportHovered;
    bool _scrollIndicatorHovered;
};

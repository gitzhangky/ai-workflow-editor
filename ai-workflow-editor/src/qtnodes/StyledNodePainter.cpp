#include "qtnodes/StyledNodePainter.hpp"

#include <QtNodes/AbstractGraphModel>
#include <QtNodes/BasicGraphicsScene>
#include <QtNodes/ConnectionIdUtils>
#include <QtNodes/Definitions>
#include <QtNodes/NodeStyle>
#include <QtNodes/internal/NodeDelegateModel.hpp>
#include <QtNodes/internal/ConnectionGraphicsObject.hpp>
#include <QtNodes/internal/NodeGraphicsObject.hpp>

#include <QFont>
#include <QFontMetricsF>
#include <QJsonDocument>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>

#include <cmath>

namespace
{
QtNodes::NodeStyle styleFor(QtNodes::AbstractGraphModel const &model, QtNodes::NodeId nodeId)
{
    return QtNodes::NodeStyle(QJsonDocument::fromVariant(model.nodeData(nodeId, QtNodes::NodeRole::Style)).object());
}

QColor headerColorFor(QtNodes::NodeStyle const &style, bool selected)
{
    return selected ? style.SelectedBoundaryColor : style.NormalBoundaryColor;
}

QString badgeTextFor(QString const &typeKey)
{
    if (typeKey == QStringLiteral("start") || typeKey == QStringLiteral("condition")
        || typeKey == QStringLiteral("output")) {
        return QStringLiteral("FLOW");
    }

    if (typeKey == QStringLiteral("prompt") || typeKey == QStringLiteral("llm"))
        return QStringLiteral("AI");

    if (typeKey == QStringLiteral("tool"))
        return QStringLiteral("TOOL");

    return typeKey.toUpper();
}

int hintLineCount(QString const &typeKey)
{
    if (typeKey == QStringLiteral("start") || typeKey == QStringLiteral("output"))
        return 1;

    if (typeKey == QStringLiteral("condition"))
        return 2;

    return 3;
}

QRectF cardRectFor(QtNodes::AbstractNodeGeometry const &geometry, QtNodes::NodeId nodeId)
{
    return QRectF(QPointF(0.0, 0.0), geometry.size(nodeId));
}

QtNodes::NodeValidationState validationStateFor(QtNodes::AbstractGraphModel const &model, QtNodes::NodeId nodeId)
{
    return model.nodeData(nodeId, QtNodes::NodeRole::ValidationState).value<QtNodes::NodeValidationState>();
}
}

void StyledNodePainter::paint(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const
{
    painter->setRenderHint(QPainter::Antialiasing);

    drawCard(painter, ngo);
    drawHeader(painter, ngo);
    drawTitle(painter, ngo);
    drawContentHints(painter, ngo);
    drawValidationBadge(painter, ngo);
    drawPorts(painter, ngo);
}

void StyledNodePainter::drawCard(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const
{
    auto const &model = ngo.graphModel();
    auto const nodeId = ngo.nodeId();
    auto const &geometry = ngo.nodeScene()->nodeGeometry();
    auto const rect = cardRectFor(geometry, nodeId);
    auto const nodeStyle = styleFor(model, nodeId);
    auto const validationState = validationStateFor(model, nodeId);
    const bool hasValidationIssue = validationState._state != QtNodes::NodeValidationState::State::Valid;
    const QColor validationAccent = validationState._state == QtNodes::NodeValidationState::State::Error
                                        ? nodeStyle.ErrorColor
                                        : nodeStyle.WarningColor;

    QPainterPath shadowPath;
    shadowPath.addRoundedRect(rect.translated(0.0, 4.0), CardCornerRadius, CardCornerRadius);
    painter->fillPath(shadowPath, QColor(nodeStyle.ShadowColor.red(),
                                         nodeStyle.ShadowColor.green(),
                                         nodeStyle.ShadowColor.blue(),
                                         90));

    QPainterPath cardPath;
    cardPath.addRoundedRect(rect, CardCornerRadius, CardCornerRadius);

    QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
    gradient.setColorAt(0.0, nodeStyle.GradientColor0);
    gradient.setColorAt(0.45, nodeStyle.GradientColor1);
    gradient.setColorAt(0.85, nodeStyle.GradientColor2);
    gradient.setColorAt(1.0, nodeStyle.GradientColor3);
    painter->fillPath(cardPath, gradient);

    if (hasValidationIssue) {
        QColor indicator = validationAccent;
        indicator.setAlpha(validationState._state == QtNodes::NodeValidationState::State::Error ? 165 : 135);
        painter->setPen(Qt::NoPen);
        painter->setBrush(indicator);
        painter->drawRoundedRect(QRectF(rect.left() + 14.0, rect.bottom() - 10.0, rect.width() - 28.0, 3.0), 1.5, 1.5);
    }

    const QColor borderColor = hasValidationIssue && !ngo.isSelected()
                                   ? validationAccent
                                   : (ngo.isSelected() ? nodeStyle.SelectedBoundaryColor : nodeStyle.NormalBoundaryColor);
    QPen borderPen(borderColor, ngo.isSelected() ? 2.3 : (hasValidationIssue ? 1.7 : 1.4));
    painter->setPen(borderPen);
    painter->drawPath(cardPath);
}

void StyledNodePainter::drawHeader(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const
{
    auto const &model = ngo.graphModel();
    auto const nodeId = ngo.nodeId();
    auto const &geometry = ngo.nodeScene()->nodeGeometry();
    auto const rect = cardRectFor(geometry, nodeId);
    auto const nodeStyle = styleFor(model, nodeId);
    auto const headerColor = headerColorFor(nodeStyle, ngo.isSelected());

    QRectF headerRect(rect.left() + 1.0, rect.top() + 1.0, rect.width() - 2.0, 28.0);
    QPainterPath headerPath;
    headerPath.moveTo(headerRect.left() + CardCornerRadius, headerRect.top());
    headerPath.lineTo(headerRect.right() - CardCornerRadius, headerRect.top());
    headerPath.quadTo(headerRect.right(),
                      headerRect.top(),
                      headerRect.right(),
                      headerRect.top() + CardCornerRadius);
    headerPath.lineTo(headerRect.right(), headerRect.bottom());
    headerPath.lineTo(headerRect.left(), headerRect.bottom());
    headerPath.lineTo(headerRect.left(), headerRect.top() + CardCornerRadius);
    headerPath.quadTo(headerRect.left(),
                      headerRect.top(),
                      headerRect.left() + CardCornerRadius,
                      headerRect.top());

    painter->fillPath(headerPath, headerColor);

    painter->setPen(QPen(QColor(255, 255, 255, 80), 1.0));
    painter->drawLine(QPointF(headerRect.left() + 12.0, headerRect.bottom()),
                      QPointF(headerRect.right() - 12.0, headerRect.bottom()));

    QFont badgeFont = painter->font();
    badgeFont.setPointSizeF(9.0);
    badgeFont.setBold(true);
    painter->setFont(badgeFont);
    painter->setPen(QColor(255, 255, 255, 220));
    painter->drawText(headerRect.adjusted(12.0, 0.0, -12.0, 0.0),
                      Qt::AlignVCenter | Qt::AlignLeft,
                      badgeTextFor(model.nodeData(nodeId, QtNodes::NodeRole::Type).toString()));
}

void StyledNodePainter::drawTitle(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const
{
    auto const &model = ngo.graphModel();
    auto const nodeId = ngo.nodeId();
    auto const &geometry = ngo.nodeScene()->nodeGeometry();
    auto const rect = cardRectFor(geometry, nodeId);
    auto const nodeStyle = styleFor(model, nodeId);

    QFont titleFont = painter->font();
    titleFont.setPointSizeF(11.5);
    titleFont.setBold(true);
    painter->setFont(titleFont);
    painter->setPen(nodeStyle.FontColor);

    QRectF titleRect(rect.left() + 14.0, rect.top() + 42.0, rect.width() - 28.0, 22.0);
    painter->drawText(titleRect,
                      Qt::AlignLeft | Qt::AlignVCenter,
                      model.nodeData(nodeId, QtNodes::NodeRole::Caption).toString());
}

void StyledNodePainter::drawContentHints(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const
{
    auto const &model = ngo.graphModel();
    auto const nodeId = ngo.nodeId();
    auto const &geometry = ngo.nodeScene()->nodeGeometry();
    auto const rect = cardRectFor(geometry, nodeId);
    auto const nodeStyle = styleFor(model, nodeId);
    auto const typeKey = model.nodeData(nodeId, QtNodes::NodeRole::Type).toString();

    painter->setPen(Qt::NoPen);

    QColor hintColor(nodeStyle.FontColorFaded.red(),
                     nodeStyle.FontColorFaded.green(),
                     nodeStyle.FontColorFaded.blue(),
                     60);
    QColor strongHintColor(nodeStyle.FontColorFaded.red(),
                           nodeStyle.FontColorFaded.green(),
                           nodeStyle.FontColorFaded.blue(),
                           95);

    const int lineCount = hintLineCount(typeKey);
    const qreal contentTop = rect.top() + 74.0;
    const qreal contentBottom = rect.bottom() - 12.0;
    qreal currentTop = contentTop;
    for (int index = 0; index < lineCount; ++index) {
        const qreal lineHeight = index == 0 ? 6.0 : 5.0;
        if (currentTop + lineHeight > contentBottom)
            break;

        const qreal width = index == 0 ? rect.width() - 34.0 : rect.width() - 54.0 - (index * 8.0);
        const QRectF lineRect(rect.left() + 14.0, currentTop, width, lineHeight);
        painter->setBrush(index == 0 ? strongHintColor : hintColor);
        painter->drawRoundedRect(lineRect, 2.5, 2.5);
        currentTop += 14.0;
    }
}

void StyledNodePainter::drawPorts(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const
{
    auto const &model = ngo.graphModel();
    auto const nodeId = ngo.nodeId();
    auto const &geometry = ngo.nodeScene()->nodeGeometry();
    auto const nodeStyle = styleFor(model, nodeId);

    for (QtNodes::PortType portType : {QtNodes::PortType::Out, QtNodes::PortType::In}) {
        auto const count = model.nodeData(nodeId,
                                          portType == QtNodes::PortType::Out ? QtNodes::NodeRole::OutPortCount
                                                                             : QtNodes::NodeRole::InPortCount)
                               .toUInt();

        for (QtNodes::PortIndex index = 0; index < count; ++index) {
            const QPointF position = geometry.portPosition(nodeId, portType, index);
            const bool connected = !model.connections(nodeId, portType, index).empty();
            qreal portRadius = 6.5;
            bool drawHalo = false;

            if (auto const *cgo = ngo.nodeState().connectionForReaction()) {
                const auto requiredPort = cgo->connectionState().requiredPort();
                if (requiredPort == portType) {
                    const auto possibleConnectionId = QtNodes::makeCompleteConnectionId(cgo->connectionId(),
                                                                                        nodeId,
                                                                                        index);
                    const bool possible = model.connectionPossible(possibleConnectionId);
                    const QPointF connectionPoint =
                        ngo.sceneTransform().inverted().map(cgo->sceneTransform().map(cgo->endPoint(requiredPort)));
                    const QPointF diff = connectionPoint - position;
                    const qreal distance = std::sqrt(QPointF::dotProduct(diff, diff));

                    if (possible && distance < 44.0) {
                        const qreal influence = 1.0 - (distance / 44.0);
                        portRadius += 3.0 * influence;
                        drawHalo = true;
                    }
                }
            }

            if (drawHalo) {
                QColor halo = nodeStyle.SelectedBoundaryColor;
                halo.setAlpha(70);
                painter->setPen(Qt::NoPen);
                painter->setBrush(halo);
                painter->drawEllipse(position, portRadius + 4.5, portRadius + 4.5);
            }

            painter->setPen(QPen(connected ? nodeStyle.FilledConnectionPointColor : nodeStyle.NormalBoundaryColor,
                                 1.4));
            painter->setBrush(QColor(QStringLiteral("#FFFDF9")));
            painter->drawEllipse(position, portRadius, portRadius);

            if (connected) {
                painter->setPen(Qt::NoPen);
                painter->setBrush(nodeStyle.FilledConnectionPointColor);
                painter->drawEllipse(position, 3.2, 3.2);
            }
        }
    }

    if (ngo.nodeState().connectionForReaction())
        ngo.nodeState().resetConnectionForReaction();
}

void StyledNodePainter::drawValidationBadge(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const
{
    auto const &model = ngo.graphModel();
    auto const nodeId = ngo.nodeId();
    const auto validationState = validationStateFor(model, nodeId);
    if (validationState._state == QtNodes::NodeValidationState::State::Valid)
        return;

    const QColor accent = validationState._state == QtNodes::NodeValidationState::State::Error
                              ? QColor(QStringLiteral("#C15757"))
                              : QColor(QStringLiteral("#D48C2F"));

    auto const &geometry = ngo.nodeScene()->nodeGeometry();
    auto const rect = cardRectFor(geometry, nodeId);

    QRectF badgeRect(rect.right() - 28.0, rect.top() + 8.0, 16.0, 16.0);
    painter->setPen(Qt::NoPen);
    painter->setBrush(accent);
    painter->drawEllipse(badgeRect);

    QFont badgeFont = painter->font();
    badgeFont.setPointSizeF(10.0);
    badgeFont.setBold(true);
    painter->setFont(badgeFont);
    painter->setPen(QColor(QStringLiteral("#FFFDF9")));
    painter->drawText(badgeRect,
                      Qt::AlignCenter,
                      validationState._state == QtNodes::NodeValidationState::State::Error ? QStringLiteral("x")
                                                                                            : QStringLiteral("!"));
}

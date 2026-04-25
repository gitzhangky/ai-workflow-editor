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

    QRectF headerRect(rect.left() + 1.0, rect.top() + 1.0, rect.width() - 2.0, 34.0);
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

    QFont headerFont = painter->font();
    headerFont.setPointSizeF(12.5);
    headerFont.setBold(true);
    painter->setFont(headerFont);
    painter->setPen(QColor(255, 255, 255, 240));
    painter->drawText(headerRect.adjusted(12.0, 0.0, -12.0, 0.0),
                      Qt::AlignVCenter | Qt::AlignLeft,
                      model.nodeData(nodeId, QtNodes::NodeRole::Caption).toString());
}

void StyledNodePainter::drawContentHints(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const
{
    Q_UNUSED(painter);
    Q_UNUSED(ngo);
}

void StyledNodePainter::drawPorts(QPainter *painter, QtNodes::NodeGraphicsObject &ngo) const
{
    auto const &model = ngo.graphModel();
    auto const nodeId = ngo.nodeId();
    auto const &geometry = ngo.nodeScene()->nodeGeometry();
    auto const nodeStyle = styleFor(model, nodeId);
    auto const rect = cardRectFor(geometry, nodeId);

    QFont labelFont = painter->font();
    labelFont.setPointSizeF(9.5);
    labelFont.setWeight(QFont::Light);

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

            const QString label = model.portData(nodeId, portType, index, QtNodes::PortRole::Caption).toString();
            if (!label.isEmpty()) {
                painter->setFont(labelFont);
                painter->setPen(nodeStyle.FontColorFaded);
                const qreal labelOffset = portRadius + 6.0;
                if (portType == QtNodes::PortType::In) {
                    QRectF labelRect(position.x() + labelOffset, position.y() - 8.0,
                                     rect.width() * 0.5, 16.0);
                    painter->drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, label);
                } else {
                    QRectF labelRect(position.x() - labelOffset - rect.width() * 0.5, position.y() - 8.0,
                                     rect.width() * 0.5, 16.0);
                    painter->drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, label);
                }
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

    QRectF badgeRect(rect.right() - 28.0, rect.top() + 10.0, 16.0, 16.0);
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

#include "mapoverlay.h"

#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QPen>
#include <QPointer>

constexpr int NO_TRANSITABLE = 0;
constexpr int TRANSITABLE = 1;
constexpr int SALIDA = 2;
constexpr int META = 3;
constexpr int CHECKPOINT = 4;
constexpr int SEMITRANSITABLE = 5;
constexpr int SENSOR_ALT1 = 7;
constexpr int SENSOR_ALT0 = 9;
constexpr int SALIDA_ALT0 = 10;
constexpr int SALIDA_ALT1 = 11;
constexpr int META_ALT0 = 12;
constexpr int META_ALT1 = 13;
constexpr int CHECK_ALT0 = 14;
constexpr int CHECK_ALT1 = 15;

MapOverlay::MapOverlay(QGraphicsScene* scene_, QObject* parent): QObject(parent), scene(scene_) {}

MapOverlay::~MapOverlay() { clear(); }

void MapOverlay::setScene(QGraphicsScene* s) {
    if (scene == s)
        return;

    if (scene && sceneDestroyedConnection)
        QObject::disconnect(sceneDestroyedConnection);

    deleteHoverPreviewPool();
    removeOverlayItemsFromScene();
    clearHoverArrow();

    scene = s;

    if (scene) {
        sceneDestroyedConnection =
                QObject::connect(scene, &QObject::destroyed, this, &MapOverlay::onSceneDestroyed);
    } else {
        sceneDestroyedConnection = QMetaObject::Connection();
    }
}

void MapOverlay::onSceneDestroyed() {
    scene = nullptr;
    sceneDestroyedConnection = QMetaObject::Connection();

    hoverPreviewPool.clear();
    overlayItems.clear();
    hoverOutline = nullptr;
    hoverArrow = nullptr;
}

void MapOverlay::setGridReference(const QVector<QVector<int>>* grid, int rows, int cols,
                                  int cellSizePx) {
    gridPtr = grid;
    m_rows = rows;
    m_cols = cols;
    cellSize = cellSizePx;
}

void MapOverlay::setCheckpointOrderReference(
        const QList<QPair<int, QVector<QPoint>>>* checkpointOrder) {
    checkpointOrderPtr = checkpointOrder;
}

void MapOverlay::setCellSize(int px) { cellSize = px; }

void MapOverlay::setRowsCols(int rows, int cols) {
    m_rows = rows;
    m_cols = cols;
}

void MapOverlay::updateOverlay() {
    if (!scene || !gridPtr)
        return;

    removeOverlayItemsFromScene();

    const int cs = cellSize;
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            int val = (*gridPtr)[r][c];
            QRectF rect(c * cs, r * cs, cs, cs);
            QColor fill;
            QPen pen(Qt::NoPen);
            bool draw = false;

            switch (val) {
                case SALIDA:
                case SALIDA_ALT0:
                case SALIDA_ALT1:
                    fill = QColor(0, 255, 0, 120);
                    draw = true;
                    break;
                case META:
                case META_ALT0:
                case META_ALT1:
                    fill = QColor(0, 250, 250, 120);
                    draw = true;
                    break;
                case CHECKPOINT:
                case CHECK_ALT0:
                case CHECK_ALT1:
                    fill = QColor(0, 0, 255, 120);
                    draw = true;
                    break;
                default:
                    break;
            }

            if (draw) {
                QGraphicsRectItem* rectItem = scene->addRect(rect, pen, QBrush(fill));
                rectItem->setZValue(5);
                overlayItems.push_back(rectItem);
            }
        }
    }

    if (checkpointOrderPtr) {
        for (const auto& pair: *checkpointOrderPtr) {
            int index = pair.first;
            const QVector<QPoint>& cells = pair.second;
            if (cells.isEmpty())
                continue;

            QPoint target = cells.first();
            for (const QPoint& p: cells) {
                if (p.y() < target.y() || (p.y() == target.y() && p.x() < target.x()))
                    target = p;
            }

            QGraphicsTextItem* textItem = scene->addText(QString::number(index));
            textItem->setDefaultTextColor(Qt::white);
            textItem->setPos(target.x() * cs + cs / 4, target.y() * cs + cs / 4);
            textItem->setZValue(7);
            overlayItems.push_back(textItem);
        }
    }
}

void MapOverlay::showHoverPreviewRect(int topRow, int leftCol, int widthCells, int heightCells,
                                      int penWidth) {
    deleteHoverPreviewPool();
    hoverOutline = nullptr;

    if (!scene || !gridPtr)
        return;

    for (int dr = 0; dr < heightCells; ++dr) {
        for (int dc = 0; dc < widthCells; ++dc) {
            int rr = topRow + dr;
            int cc = leftCol + dc;
            if (rr < 0 || rr >= m_rows || cc < 0 || cc >= m_cols)
                continue;

            int val = (*gridPtr)[rr][cc];
            QColor color = (val == TRANSITABLE || val == SENSOR_ALT0 || val == SENSOR_ALT1) ?
                                   QColor(0, 255, 0, 60) :
                                   QColor(255, 0, 0, 60);

            QRectF rect(cc * cellSize, rr * cellSize, cellSize, cellSize);

            QGraphicsRectItem* cell = scene->addRect(rect, QPen(Qt::DashLine), QBrush(color));
            cell->setZValue(50);
            hoverPreviewPool.append(cell);
        }
    }

    QPen pen(Qt::yellow);
    pen.setWidth(penWidth);
    pen.setStyle(Qt::DashLine);

    QGraphicsRectItem* outline =
            scene->addRect(leftCol * cellSize, topRow * cellSize, widthCells * cellSize,
                           heightCells * cellSize, pen, Qt::NoBrush);
    outline->setZValue(50);
    hoverOutline = outline;
    hoverPreviewPool.append(outline);
}

void MapOverlay::clearHoverPreview() { deleteHoverPreviewPool(); }

void MapOverlay::showExitArrow(int topRow, int leftCol, int widthCells, int heightCells,
                               int direction) {
    clearHoverArrow();
    if (!scene)
        return;

    QPointF center((leftCol + widthCells / 2.0) * cellSize,
                   (topRow + heightCells / 2.0) * cellSize);

    const qreal s = cellSize * 2.5;
    QPolygonF arrow;
    arrow << QPointF(0, -s / 2) << QPointF(s / 4, 0) << QPointF(-s / 4, 0);

    QPen pen(Qt::NoPen);
    QBrush brush(QColor(0, 255, 0, 200));
    QGraphicsPolygonItem* arrowItem = scene->addPolygon(arrow, pen, brush);
    arrowItem->setZValue(60);
    arrowItem->setPos(center);
    arrowItem->setRotation(direction * 90);

    hoverArrow = arrowItem;
}

void MapOverlay::deleteHoverPreviewPool() {
    if (scene) {
        for (auto* it: hoverPreviewPool) {
            if (!it)
                continue;
            if (it->scene() == scene)
                scene->removeItem(it);
            delete it;
        }
    }

    hoverPreviewPool.clear();
    hoverOutline = nullptr;
}

void MapOverlay::removeOverlayItemsFromScene() {
    if (scene) {
        for (auto* item: overlayItems) {
            if (!item)
                continue;
            if (item->scene() == scene)
                scene->removeItem(item);
            delete item;
        }
    }
    overlayItems.clear();
}

void MapOverlay::clearHoverArrow() {
    if (!hoverArrow)
        return;

    if (scene) {
        if (hoverArrow->scene() == scene)
            scene->removeItem(hoverArrow);
        delete hoverArrow;
    }
    hoverArrow = nullptr;
}

void MapOverlay::clear() {
    deleteHoverPreviewPool();
    clearHoverArrow();
    removeOverlayItemsFromScene();
}

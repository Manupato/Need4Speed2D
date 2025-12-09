#ifndef MAPOVERLAY_H
#define MAPOVERLAY_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QPair>
#include <QPoint>
#include <QPointer>
#include <QVector>

class MapOverlay: public QObject {
    Q_OBJECT
public:
    explicit MapOverlay(QGraphicsScene* scene = nullptr, QObject* parent = nullptr);
    ~MapOverlay() override;

    void setScene(QGraphicsScene* s);
    void setGridReference(const QVector<QVector<int>>* grid, int rows, int cols, int cellSizePx);
    void setCheckpointOrderReference(const QList<QPair<int, QVector<QPoint>>>* checkpointOrder);

    void updateOverlay();
    void clear();

    void showHoverPreviewRect(int topRow, int leftCol, int widthCells, int heightCells,
                              int penWidth = 1);
    void clearHoverPreview();

    void showExitArrow(int topRow, int leftCol, int widthCells, int heightCells, int direction);
    void clearHoverArrow();

    void setCellSize(int px);
    void setRowsCols(int rows, int cols);


private:
    QPointer<QGraphicsScene> scene = nullptr;
    QMetaObject::Connection sceneDestroyedConnection;

    const QVector<QVector<int>>* gridPtr = nullptr;
    const QList<QPair<int, QVector<QPoint>>>* checkpointOrderPtr = nullptr;

    QList<QGraphicsItem*> overlayItems;
    QList<QGraphicsItem*> hoverPreviewPool;
    QGraphicsItem* hoverOutline = nullptr;
    QGraphicsItem* hoverArrow = nullptr;

    int m_rows = 0;
    int m_cols = 0;
    int cellSize = 16;

    void removeOverlayItemsFromScene();
    void deleteHoverPreviewPool();

    Q_SLOT void onSceneDestroyed();
};

#endif

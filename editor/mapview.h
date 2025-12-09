#ifndef MAP_VIEW_H
#define MAP_VIEW_H

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPair>
#include <QPixmap>
#include <QScrollBar>
#include <QShortcut>
#include <QStack>
#include <QString>
#include <QVector>
#include <QWheelEvent>

#include "filemanagement.h"
#include "mapoverlay.h"

static constexpr int NO_TRANSITABLE = 0;
static constexpr int TRANSITABLE = 1;
static constexpr int SALIDA = 2;
static constexpr int META = 3;
static constexpr int CHECKPOINT = 4;
static constexpr int SEMITRANSITABLE = 5;

static constexpr int SENSOR_ALT1 = 7;
static constexpr int SENSOR_ALT0 = 9;

static constexpr int SALIDA_ALT0 = 10;
static constexpr int SALIDA_ALT1 = 11;

static constexpr int META_ALT0 = 12;
static constexpr int META_ALT1 = 13;

static constexpr int CHECK_ALT0 = 14;
static constexpr int CHECK_ALT1 = 15;

struct CellChange {
    int r;
    int c;
    int oldValue;
    int newValue;
};

struct UndoState {
    QVector<CellChange> changes;
    QList<QPair<int, QVector<QPoint>>> oldCheckpointOrder;
    bool affectedCheckpoints = false;
    bool affectedDireccionSalida = false;
    int oldDireccionSalida = 0;
};

class MapView: public QGraphicsView {
    Q_OBJECT

public:
    enum class Tool { None, PlaceStart, PlaceFinish, AddCheckpoint, EraseObject };
    explicit MapView(QWidget* parent = nullptr);
    void loadMap(const QString& filePath);
    void setTool(Tool t) { currentTool = t; }
    Tool tool() const { return currentTool; }
    bool loadMatrixFromJson(const QString& filePath);
    int rows() const { return m_rows; }
    int cols() const { return m_cols; }
    int valueAt(int r, int c) const;
    void setValue(int r, int c, int v);
    bool saveMatrixToJson(const QString& filePath, const QString& baseMapFile);
    bool setCellWithRadius(int row, int col, int value, int radius);
    void updateOverlay();
    void addCheckpointOrdered(int row, int col);
    void updateHoverPreview(int row, int col);
    QString getCurrentMapImageFile() const { return currentMapImageFile; }
    void undoLastAction();
    bool hasSalida() const;
    bool hasMeta() const;
    bool hasCheckpoint() const;
    bool isPlayable() const;
    bool isTransitable(int v) const;
    int mapPlacementValue(int base, int tool);
    bool isMapModified() const { return mapModified; }
    void setMapModified(bool modified) { mapModified = modified; }
    void recordCheckpointChange();
    void recordCellChange(int r, int c, int oldV, int newV);
    void recordDireccionChange();
    void endUndoBlock();
    void beginUndoBlock();
    UndoState tempUndoState;
    bool undoBlockOpen = false;

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QGraphicsScene* scene;
    QGraphicsPixmapItem* mapItem;
    MapOverlay* overlay = nullptr;
    QPoint lastMousePos;
    QList<QPair<int, QVector<QPoint>>> checkpointOrder;
    double scaleFactor = 1.0;
    Tool currentTool = Tool::None;
    int cellSizePx = 16;
    QVector<QVector<int>> grid;
    int m_rows = 0, m_cols = 0;
    QString currentMapImageFile;
    const int salidaAncho = 15;
    const int salidaAlto = 4;
    QRect currentHoverRect;
    bool salidaVertical = true;
    int direccionSalida = 0;  // 0=arriba, 1=derecha, 2=abajo, 3=izquierda
    int hoverDireccionSalida = 0;
    bool checkpointVertical = true;
    int direccionCheckpoint = 0;  // 0=arriba, 1=derecha, 2=abajo, 3=izquierda
    QStack<UndoState> undoStack;
    QString direccionSalidaStr() const {
        switch (direccionSalida) {
            case 0:
                return "arriba";
            case 1:
                return "derecha";
            case 2:
                return "abajo";
            case 3:
                return "izquierda";
            default:
                return "arriba";
        }
    }
    bool mapModified = false;
    QJsonObject extraJsonFields;
};
#endif

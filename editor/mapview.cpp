#include "mapview.h"

#include <utility>

#include "../common/resource_paths.h"

#include "mapoverlay.h"

MapView::MapView(QWidget* parent):
        QGraphicsView(parent), scene(new QGraphicsScene(this)), mapItem(nullptr) {
    setScene(scene);
    setDragMode(QGraphicsView::NoDrag);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setFocusPolicy(Qt::StrongFocus);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    QShortcut* undoShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z), this);
    connect(undoShortcut, &QShortcut::activated, this, &MapView::undoLastAction);

    overlay = new MapOverlay(scene, this);
    overlay->setCellSize(cellSizePx);
    overlay->setRowsCols(m_rows, m_cols);
    overlay->setGridReference(&grid, m_rows, m_cols, cellSizePx);
    overlay->setCheckpointOrderReference(&checkpointOrder);
}

static bool isCheckpointCell(int v) {
    return v == CHECKPOINT || v == CHECK_ALT0 || v == CHECK_ALT1;
}
static bool isSalidaCell(int v) { return v == SALIDA || v == SALIDA_ALT0 || v == SALIDA_ALT1; }
static bool isMetaCell(int v) { return v == META || v == META_ALT0 || v == META_ALT1; }
static int direccionSalidaFromString(const QString& dir) {
    if (dir == "arriba")
        return 0;
    if (dir == "derecha")
        return 1;
    if (dir == "abajo")
        return 2;
    if (dir == "izquierda")
        return 3;
    return 0;
}


void MapView::loadMap(const QString& filePath) {
    if (overlay) {
        overlay->clear();
    }

    if (mapItem) {
        if (mapItem->scene() == scene)
            scene->removeItem(mapItem);
        delete mapItem;
        mapItem = nullptr;
    }


    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        qWarning() << "Error al cargar el mapa:" << filePath;
        return;
    }

    mapItem = scene->addPixmap(pixmap);

    scene->setSceneRect(pixmap.rect());
    fitInView(mapItem, Qt::KeepAspectRatio);

    scaleFactor = 1.0;
    centerOn(mapItem);

    if (overlay) {
        overlay->setScene(scene);
        overlay->updateOverlay();
    }
}

void MapView::wheelEvent(QWheelEvent* event) {
    constexpr double zoomFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(zoomFactor, zoomFactor);
        scaleFactor *= zoomFactor;
    } else {
        scale(1.0 / zoomFactor, 1.0 / zoomFactor);
        scaleFactor /= zoomFactor;
    }
}

static int restoreOriginalValue(int v) {
    switch (v) {
        case SALIDA_ALT0:
        case META_ALT0:
        case CHECK_ALT0:
            return SENSOR_ALT0;

        case SALIDA_ALT1:
        case META_ALT1:
        case CHECK_ALT1:
            return SENSOR_ALT1;

        case SALIDA:
        case META:
        case CHECKPOINT:
            return TRANSITABLE;

        default:
            return TRANSITABLE;
    }
}

void MapView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (currentTool == Tool::None) {
            qDebug() << "Ninguna herramienta seleccionada.";
            return;
        }

        QPointF sp = mapToScene(event->pos());
        if (!scene || !scene->sceneRect().contains(sp)) {
            QGraphicsView::mousePressEvent(event);
            return;
        }

        int c = int(sp.x()) / cellSizePx;
        int r = int(sp.y()) / cellSizePx;
        if (r < 0 || c < 0 || r >= m_rows || c >= m_cols)
            return;

        switch (currentTool) {
            case Tool::PlaceStart: {
                int widthCells = salidaVertical ? 4 : 15;
                int heightCells = salidaVertical ? 15 : 4;
                bool allWalkable = true;
                for (int dr = 0; dr < heightCells && allWalkable; ++dr)
                    for (int dc = 0; dc < widthCells; ++dc)
                        if (r + dr >= m_rows || c + dc >= m_cols ||
                            !isTransitable(valueAt(r + dr, c + dc)))
                            allWalkable = false;
                if (!allWalkable) {
                    qDebug() << "La salida debe colocarse completamente sobre celdas transitables.";
                    return;
                }
                beginUndoBlock();
                recordDireccionChange();
                for (int i = 0; i < m_rows; ++i)
                    for (int j = 0; j < m_cols; ++j)
                        if (isSalidaCell(grid[i][j]))
                            setValue(i, j, restoreOriginalValue(grid[i][j]));

                for (int dr = 0; dr < heightCells; ++dr)
                    for (int dc = 0; dc < widthCells; ++dc) {
                        int cur = valueAt(r + dr, c + dc);
                        int newv = mapPlacementValue(cur, SALIDA);
                        setValue(r + dr, c + dc, newv);
                    }
                direccionSalida = hoverDireccionSalida;

                endUndoBlock();
                updateOverlay();
                qDebug() << "Nueva salida creada en (" << r << "," << c << ")";
                break;
            }

            case Tool::PlaceFinish: {
                beginUndoBlock();
                for (int i = 0; i < m_rows; ++i)
                    for (int j = 0; j < m_cols; ++j)
                        if (isMetaCell(grid[i][j]))
                            setValue(i, j, restoreOriginalValue(grid[i][j]));

                int radius = 1;
                if (setCellWithRadius(r, c, META, radius))
                    qDebug() << "Nueva meta creada en (" << r << "," << c << ")";

                endUndoBlock();
                updateOverlay();

                break;
            }

            case Tool::AddCheckpoint: {
                int widthCells = checkpointVertical ? 1 : 5;
                int heightCells = checkpointVertical ? 5 : 1;
                bool atLeastOneValid = false;
                beginUndoBlock();
                recordCheckpointChange();

                for (int dr = 0; dr < heightCells; ++dr)
                    for (int dc = 0; dc < widthCells; ++dc)
                        if (r + dr < m_rows && c + dc < m_cols &&
                            isTransitable(grid[r + dr][c + dc])) {
                            int cur = valueAt(r + dr, c + dc);
                            int newv = mapPlacementValue(cur, CHECKPOINT);
                            setValue(r + dr, c + dc, newv);
                            atLeastOneValid = true;
                        }

                if (!atLeastOneValid) {
                    endUndoBlock();
                    qDebug() << "No hay celdas transitables bajo el checkpoint.";
                    return;
                }

                addCheckpointOrdered(r, c);
                endUndoBlock();
                updateOverlay();
                qDebug() << "Checkpoint agregado en (" << r << "," << c << ")";
                break;
            }
            case Tool::EraseObject: {
                int val = grid[r][c];

                if (isSalidaCell(val)) {
                    beginUndoBlock();
                    recordDireccionChange();

                    for (int i = 0; i < m_rows; ++i)
                        for (int j = 0; j < m_cols; ++j)
                            if (isSalidaCell(grid[i][j])) {
                                int oldV = grid[i][j];
                                int newV = restoreOriginalValue(oldV);
                                setValue(i, j, newV);
                            }

                    endUndoBlock();
                    updateOverlay();
                    qDebug() << "Salida eliminada (undo funcionando correctamente).";
                    break;
                }

                if (isMetaCell(val)) {
                    beginUndoBlock();

                    for (int i = 0; i < m_rows; ++i)
                        for (int j = 0; j < m_cols; ++j)
                            if (isMetaCell(grid[i][j])) {
                                int oldV = grid[i][j];
                                int newV = restoreOriginalValue(oldV);
                                setValue(i, j, newV);
                            }

                    endUndoBlock();
                    updateOverlay();
                    qDebug() << "Meta eliminada (undo funcionando correctamente).";
                    break;
                }

                if (isCheckpointCell(val)) {
                    beginUndoBlock();
                    recordCheckpointChange();

                    auto it = std::find_if(checkpointOrder.begin(), checkpointOrder.end(),
                                           [c, r](const QPair<int, QVector<QPoint>>& pair) {
                                               return std::any_of(
                                                       pair.second.begin(), pair.second.end(),
                                                       [c, r](const QPoint& p) {
                                                           return p.x() == c && p.y() == r;
                                                       });
                                           });

                    if (it == checkpointOrder.end()) {
                        qDebug() << "Error: checkpoint no encontrado.";
                        endUndoBlock();
                        updateOverlay();
                        break;
                    }

                    int removedOrder = it->first;

                    for (const QPoint& p: it->second) {
                        int oldV = grid[p.y()][p.x()];
                        int newV = restoreOriginalValue(oldV);
                        setValue(p.y(), p.x(), newV);
                    }

                    checkpointOrder.erase(it);

                    for (int i = 0; i < checkpointOrder.size(); ++i)
                        checkpointOrder[i].first = i + 1;

                    endUndoBlock();
                    updateOverlay();
                    qDebug() << "Checkpoint eliminado (#" << removedOrder
                             << "). Órdenes reenumeradas.";
                    break;
                }

                break;
            }


            default:
                break;
        }
    }

    if (overlay)
        overlay->clearHoverPreview();
    QGraphicsView::mousePressEvent(event);
}

bool MapView::setCellWithRadius(int row, int col, int value, int radius) {
    bool result = false;
    int numRows = grid.size();
    int numCols = grid[0].size();

    for (int dr = -radius; dr <= radius; ++dr) {
        for (int dc = -radius; dc <= radius; ++dc) {
            int r = row + dr;
            int c = col + dc;
            if (r >= 0 && r < numRows && c >= 0 && c < numCols && isTransitable(grid[r][c])) {
                setValue(r, c, mapPlacementValue(grid[r][c], value));
                result = true;
            }
        }
    }
    return result;
}

void MapView::mouseMoveEvent(QMouseEvent* event) {
    QPointF sp = mapToScene(event->pos());
    int c = int(sp.x()) / cellSizePx;
    int r = int(sp.y()) / cellSizePx;

    if (r >= 0 && c >= 0 && r < m_rows && c < m_cols) {
        if (currentTool == Tool::PlaceStart || currentTool == Tool::AddCheckpoint ||
            currentTool == Tool::PlaceFinish) {
            updateHoverPreview(r, c);
        }
    } else if (overlay) {
        overlay->clearHoverPreview();
        overlay->clearHoverArrow();
    }

    lastMousePos = event->pos();
    QGraphicsView::mouseMoveEvent(event);
}

void MapView::keyPressEvent(QKeyEvent* event) {
    const int moveStep = 64;

    switch (event->key()) {
        case Qt::Key_W:
        case Qt::Key_Up:
            verticalScrollBar()->setValue(verticalScrollBar()->value() - moveStep);
            break;
        case Qt::Key_S:
        case Qt::Key_Down:
            verticalScrollBar()->setValue(verticalScrollBar()->value() + moveStep);
            break;
        case Qt::Key_A:
        case Qt::Key_Left:
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - moveStep);
            break;
        case Qt::Key_D:
        case Qt::Key_Right:
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() + moveStep);
            break;
        case Qt::Key_R:
            if (currentTool == Tool::PlaceStart) {
                hoverDireccionSalida = (hoverDireccionSalida + 1) % 4;
                salidaVertical = (hoverDireccionSalida % 2 == 0);
            } else if (currentTool == Tool::AddCheckpoint) {
                direccionCheckpoint = (direccionCheckpoint + 1) % 4;
                checkpointVertical = (direccionCheckpoint % 2 == 0);
            }

            if (overlay) {
                QPointF sp = mapToScene(lastMousePos);
                int c = int(sp.x()) / cellSizePx;
                int r = int(sp.y()) / cellSizePx;
                updateHoverPreview(r, c);
            }
            break;
        default:
            QGraphicsView::keyPressEvent(event);
    }
}

bool MapView::loadMatrixFromJson(const QString& filePath) {
    QVector<QVector<int>> newGrid;
    QList<QPair<int, QVector<QPoint>>> newCheckpoints;
    QString baseMapName;
    QString direccionSalidaCargada;

    if (!FileManagement::loadMatrixFromJson(filePath, newGrid, newCheckpoints, baseMapName,
                                            extraJsonFields, direccionSalidaCargada)) {
        return false;
    }

    if (!baseMapName.isEmpty()) {
        QString assetsPrefix = QString::fromStdString(ResourcePaths::assets());
        QString originalMapsDir = QDir(assetsPrefix + "/original_maps/").absolutePath();
        QString mapPath = QDir(originalMapsDir).filePath(baseMapName);

        loadMap(mapPath);
        currentMapImageFile = mapPath;
    }

    grid = std::move(newGrid);
    checkpointOrder = std::move(newCheckpoints);

    m_rows = grid.size();
    m_cols = (m_rows ? grid[0].size() : 0);

    direccionSalida = direccionSalidaFromString(direccionSalidaCargada);

    updateOverlay();

    undoStack.clear();
    return true;
}


int MapView::valueAt(int r, int c) const {
    if (r < 0 || c < 0 || r >= m_rows || c >= m_cols)
        return -1;
    return grid[r][c];
}

void MapView::setValue(int r, int c, int v) {
    if (r < 0 || c < 0 || r >= m_rows || c >= m_cols)
        return;

    int oldV = grid[r][c];
    if (oldV == v)
        return;

    recordCellChange(r, c, oldV, v);
    grid[r][c] = v;
}

bool MapView::saveMatrixToJson(const QString& filePath, const QString& baseMapFile) {
    QString direccion = hasSalida() ? direccionSalidaStr() : "";
    mapModified = false;
    return FileManagement::saveMatrixToJson(filePath, grid, checkpointOrder, baseMapFile, direccion,
                                            extraJsonFields);
}

void MapView::updateOverlay() {
    if (overlay) {
        overlay->setGridReference(&grid, m_rows, m_cols, cellSizePx);
        overlay->setCheckpointOrderReference(&checkpointOrder);
        overlay->updateOverlay();
    }
}

void MapView::addCheckpointOrdered(int row, int col) {
    int widthCells = checkpointVertical ? 1 : 5;
    int heightCells = checkpointVertical ? 5 : 1;

    int topRow = row;
    int leftCol = col;

    int nextOrder = checkpointOrder.size() + 1;
    QVector<QPoint> areaCells;

    for (int dr = 0; dr < heightCells; ++dr) {
        for (int dc = 0; dc < widthCells; ++dc) {
            int rr = topRow + dr;
            int cc = leftCol + dc;

            if (rr < m_rows && cc < m_cols && isCheckpointCell(grid[rr][cc])) {
                areaCells.append(QPoint(cc, rr));
            }
        }
    }

    if (areaCells.isEmpty()) {
        qDebug() << "[WARN] No se agregó checkpoint #" << nextOrder << ": área vacía.";
        return;
    }

    checkpointOrder.append({nextOrder, areaCells});
    qDebug() << "Checkpoint #" << nextOrder << " agregado con" << areaCells.size() << " celdas.";
}


void MapView::updateHoverPreview(int row, int col) {
    if (!overlay)
        return;

    overlay->clearHoverPreview();
    overlay->clearHoverArrow();

    switch (currentTool) {
        case Tool::PlaceStart: {
            int widthCells = salidaVertical ? 4 : 15;
            int heightCells = salidaVertical ? 15 : 4;
            overlay->showHoverPreviewRect(row, col, widthCells, heightCells);
            overlay->showExitArrow(row, col, widthCells, heightCells, hoverDireccionSalida);
            break;
        }
        case Tool::PlaceFinish: {
            overlay->showHoverPreviewRect(row - 1, col - 1, 3, 3);
            break;
        }
        case Tool::AddCheckpoint: {
            int widthCells = checkpointVertical ? 1 : 5;
            int heightCells = checkpointVertical ? 5 : 1;
            overlay->showHoverPreviewRect(row, col, widthCells, heightCells);
            break;
        }
        default:
            break;
    }
}

void MapView::beginUndoBlock() {
    if (undoBlockOpen)
        return;
    undoBlockOpen = true;
    tempUndoState = UndoState();
}

void MapView::endUndoBlock() {
    if (!undoBlockOpen)
        return;

    undoBlockOpen = false;

    if (tempUndoState.changes.isEmpty() && !tempUndoState.affectedCheckpoints)
        return;

    undoStack.push(tempUndoState);
    if (undoStack.size() > 10)
        undoStack.removeFirst();

    mapModified = true;
}

void MapView::recordCellChange(int r, int c, int oldV, int newV) {
    if (!undoBlockOpen)
        return;
    if (oldV == newV)
        return;

    CellChange ch;
    ch.r = r;
    ch.c = c;
    ch.oldValue = oldV;
    ch.newValue = newV;
    tempUndoState.changes.append(ch);
}

void MapView::recordCheckpointChange() {
    if (!undoBlockOpen)
        return;

    tempUndoState.affectedCheckpoints = true;
    tempUndoState.oldCheckpointOrder = checkpointOrder;
}

void MapView::recordDireccionChange() {
    if (!undoBlockOpen)
        return;
    tempUndoState.affectedDireccionSalida = true;
    tempUndoState.oldDireccionSalida = direccionSalida;
}

void MapView::undoLastAction() {
    if (undoStack.isEmpty()) {
        qDebug() << "Nada para deshacer.";
        return;
    }

    UndoState st = undoStack.pop();

    for (int i = int(st.changes.size()) - 1; i >= 0; --i) {
        const CellChange& ch = st.changes[i];
        grid[ch.r][ch.c] = ch.oldValue;
    }

    if (st.affectedCheckpoints) {
        checkpointOrder = st.oldCheckpointOrder;
    }

    if (st.affectedDireccionSalida) {
        direccionSalida = st.oldDireccionSalida;
        salidaVertical = (direccionSalida % 2 == 0);
    }

    updateOverlay();
    qDebug() << "Acción deshecha.";
}

bool MapView::hasSalida() const {
    return std::any_of(grid.begin(), grid.end(), [](const auto& row) {
        return std::any_of(row.begin(), row.end(), [](int val) { return isSalidaCell(val); });
    });
}

bool MapView::hasMeta() const {
    return std::any_of(grid.begin(), grid.end(), [](const auto& row) {
        return std::any_of(row.begin(), row.end(), [](int val) { return isMetaCell(val); });
    });
}

bool MapView::hasCheckpoint() const {
    return std::any_of(grid.begin(), grid.end(), [](const auto& row) {
        return std::any_of(row.begin(), row.end(), [](int val) { return isCheckpointCell(val); });
    });
}

bool MapView::isPlayable() const {
    if (!hasSalida())
        return false;
    if (!hasMeta())
        return false;
    if (!hasCheckpoint())
        return false;

    return true;
}

bool MapView::isTransitable(int v) const {
    return (v == TRANSITABLE || v == SENSOR_ALT1 || v == SENSOR_ALT0 || (v >= 10 && v <= 15));
}

int MapView::mapPlacementValue(int base, int tool) {
    if (!isTransitable(base))
        return -1;

    switch (tool) {
        case SALIDA:
            if (base == SENSOR_ALT0)
                return SALIDA_ALT0;
            if (base == SENSOR_ALT1)
                return SALIDA_ALT1;
            return SALIDA;

        case META:
            if (base == SENSOR_ALT0)
                return META_ALT0;
            if (base == SENSOR_ALT1)
                return META_ALT1;
            return META;

        case CHECKPOINT:
            if (base == SENSOR_ALT0)
                return CHECK_ALT0;
            if (base == SENSOR_ALT1)
                return CHECK_ALT1;
            return CHECKPOINT;
    }

    return -1;
}

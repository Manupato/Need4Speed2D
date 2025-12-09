#include "filemanagement.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <utility>

bool FileManagement::loadMatrixFromJson(const QString& filePath, QVector<QVector<int>>& outGrid,
                                        QList<QPair<int, QVector<QPoint>>>& outCheckpoints,
                                        QString& outBaseMapName, QJsonObject& outExtraFields,
                                        QString& outDireccionSalida) {
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "No se pudo abrir el archivo:" << filePath;
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) {
        qWarning() << "JSON inválido:" << filePath;
        return false;
    }

    QJsonObject root = doc.object();

    outBaseMapName.clear();
    if (root.contains("base_map"))
        outBaseMapName = root["base_map"].toString();

    outExtraFields = QJsonObject();
    for (auto it = root.begin(); it != root.end(); ++it) {
        QString k = it.key();
        if (k != "grid" && k != "checkpoints_order" && k != "base_map" && k != "direccion_salida") {
            outExtraFields.insert(k, it.value());
        }
    }

    if (!root.contains("grid") || !root["grid"].isArray())
        return false;

    QJsonArray outer = root["grid"].toArray();
    QVector<QVector<int>> tmp;
    tmp.reserve(outer.size());
    int expectedCols = -1;

    for (const auto& rowVal: outer) {
        if (!rowVal.isArray())
            return false;

        QJsonArray rowArr = rowVal.toArray();
        QVector<int> row;
        row.reserve(rowArr.size());

        for (const auto& cell: rowArr) row.append(cell.toInt());

        if (expectedCols == -1)
            expectedCols = row.size();
        else if (row.size() != expectedCols)
            return false;

        tmp.push_back(std::move(row));
    }

    outGrid = std::move(tmp);

    outCheckpoints.clear();
    if (root.contains("checkpoints_order") && root["checkpoints_order"].isArray()) {
        QJsonArray arr = root["checkpoints_order"].toArray();
        for (const auto& val: arr) {
            if (!val.isObject())
                continue;

            QJsonObject obj = val.toObject();
            int order = obj["order"].toInt();

            QVector<QPoint> cells;
            if (obj.contains("cells") && obj["cells"].isArray()) {
                for (const auto& cellVal: obj["cells"].toArray()) {
                    QJsonObject cellObj = cellVal.toObject();
                    cells.append(QPoint(cellObj["col"].toInt(), cellObj["row"].toInt()));
                }
            }
            outCheckpoints.append({order, cells});
        }
    }

    outDireccionSalida =
            root.contains("direccion_salida") ? root["direccion_salida"].toString() : QString();

    return true;
}

bool FileManagement::saveMatrixToJson(const QString& filePath, const QVector<QVector<int>>& grid,
                                      const QList<QPair<int, QVector<QPoint>>>& checkpoints,
                                      const QString& baseMapFile, const QString& direccionSalida,
                                      const QJsonObject& extraFields) {
    if (grid.isEmpty())
        return false;

    QJsonObject root;

    for (auto it = extraFields.begin(); it != extraFields.end(); ++it)
        root.insert(it.key(), it.value());

    if (!baseMapFile.isEmpty())
        root["base_map"] = QFileInfo(baseMapFile).fileName();

    QJsonArray outer;
    for (const auto& row: grid) {
        QJsonArray rowArr;
        for (int v: row) rowArr.append(v);
        outer.append(rowArr);
    }
    root["grid"] = outer;

    QJsonArray checkpointsArr;
    for (const auto& pair: checkpoints) {
        QJsonObject obj;
        obj["order"] = pair.first;

        QJsonArray cells;
        for (const QPoint& p: pair.second) {
            QJsonObject cell;
            cell["row"] = p.y();
            cell["col"] = p.x();
            cells.append(cell);
        }

        obj["cells"] = cells;
        checkpointsArr.append(obj);
    }
    root["checkpoints_order"] = checkpointsArr;

    root["direccion_salida"] = direccionSalida;

    QJsonDocument doc(root);
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly))
        return false;

    f.write(doc.toJson(QJsonDocument::Compact));
    return true;
}

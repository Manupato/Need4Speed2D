#ifndef FILEMANAGEMENT_H
#define FILEMANAGEMENT_H

#include <QJsonObject>
#include <QList>
#include <QPair>
#include <QPoint>
#include <QString>
#include <QVector>

class FileManagement {
public:
    static bool loadMatrixFromJson(const QString& filePath, QVector<QVector<int>>& outGrid,
                                   QList<QPair<int, QVector<QPoint>>>& outCheckpoints,
                                   QString& outBaseMapName, QJsonObject& outExtraFields,
                                   QString& outDireccionSalida);

    static bool saveMatrixToJson(const QString& filePath, const QVector<QVector<int>>& grid,
                                 const QList<QPair<int, QVector<QPoint>>>& checkpoints,
                                 const QString& baseMapFile, const QString& direccionSalida,
                                 const QJsonObject& extraFields);
};

#endif

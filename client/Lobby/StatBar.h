#ifndef STATBAR_H
#define STATBAR_H

#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QMap>
#include <QObject>
#include <QWidget>


class StatBar: public QObject {
    Q_OBJECT

public:
    explicit StatBar(QWidget* parent = nullptr);

    static void loadCarStatsFromConfig(const QString& path);

    void init(QWidget* parent);

    void addToLayout(QLayout* layout, int barWidth = 150);

    void update(int carId);

    static QStringList carNames();

private:
    struct StatWidgets {
        QLabel* label;
        QFrame* bg;
        QFrame* fill;
    };

    QMap<QString, StatWidgets> widgets;

    QFormLayout* layout = nullptr;

    static QVector<QMap<QString, int>> CAR_STATS;
    static QStringList CAR_NAMES;

    const QStringList statKeys = {"speed", "engine_force", "handling", "weight", "shield"};
    const QStringList statNames = {"Velocidad", "Caballos de fuerza", "Maniobrabilidad", "Peso",
                                   "Escudo"};
};

#endif

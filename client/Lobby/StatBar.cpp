#include "StatBar.h"

#include <QDir>
#include <QVBoxLayout>

#include <yaml-cpp/yaml.h>

QVector<QMap<QString, int>> StatBar::CAR_STATS;
QStringList StatBar::CAR_NAMES;

StatBar::StatBar(QWidget* parent): QObject(parent) {}

void StatBar::loadCarStatsFromConfig(const QString& path) {
    try {
        YAML::Node config = YAML::LoadFile(path.toStdString());
        const YAML::Node& cars = config["cars"];

        if (!cars) {
            qWarning() << "StatBar: config.yaml no tiene nodo 'cars'";
            return;
        }

        CAR_STATS.clear();

        for (YAML::const_iterator it = cars.begin(); it != cars.end(); ++it) {
            const YAML::Node& carNode = it->second;
            const YAML::Node& stats = carNode["stats"];

            QMap<QString, int> entry;
            entry["speed"] = stats["speed"].as<int>();
            entry["engine_force"] = stats["engine_force"].as<int>();
            entry["handling"] = stats["handling"].as<int>();
            entry["weight"] = stats["weight"].as<int>();
            entry["shield"] = stats["shield"].as<int>();

            CAR_STATS.append(entry);
        }

    } catch (...) {
        qWarning() << "Error cargando YAML en:" << path;
    }
}

QStringList StatBar::carNames() { return CAR_NAMES; }

void StatBar::init(QWidget* parent) {
    layout = new QFormLayout();
    layout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    for (int i = 0; i < statKeys.size(); ++i) {
        StatWidgets w;

        w.label = new QLabel(statNames[i] + ":", parent);

        w.bg = new QFrame(parent);
        w.bg->setFixedHeight(10);
        w.bg->setStyleSheet("background: rgba(255,255,255,0.15); border-radius:5px;");

        w.fill = new QFrame(w.bg);
        w.fill->setGeometry(0, 0, 0, 10);
        w.fill->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
                              "stop:0 #FF3399, stop:1 #A020F0); border-radius: 5px;");

        widgets[statKeys[i]] = w;
    }
}

void StatBar::addToLayout(QLayout* outerLayout, int barWidth) {
    for (auto it = widgets.begin(); it != widgets.end(); ++it) {
        auto& w = it.value();
        w.bg->setFixedWidth(barWidth);
        layout->addRow(w.label, w.bg);
        layout->setSpacing(8);
    }

    outerLayout->addItem(layout);
}

void StatBar::update(int carId) {
    if (carId < 0 || carId >= CAR_STATS.size())
        return;

    const auto& stats = CAR_STATS[carId];

    for (auto it = stats.begin(); it != stats.end(); ++it) {
        const QString& key = it.key();
        int value = it.value();

        StatWidgets& w = widgets[key];

        int fullWidth = w.bg->width();
        int fillWidth = fullWidth * value / 100;

        w.fill->setGeometry(0, 0, fillWidth, 10);
    }
}

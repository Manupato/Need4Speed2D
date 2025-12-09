#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QActionGroup>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

#include "mapview.h"


class MainWindow: public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;


private:
    MapView mapView{this};

    QAction guardarMatrizAction{"Guardar", this};
    QAction guardarMapaJugableAction{"Guardar Como", this};
    QAction abrirMapaJugableAction{"Abrir Mapa", this};

    QAction salida{"Salida", this};
    QAction llegada{"Meta", this};
    QAction checkpoint{"Checkpoint", this};
    QAction borrar{"Borrar", this};

    QString currentMatrixFile;
    QString currentMapImageFile;

    QString baseMaps;
    QString baseJson;
    QString userMaps;

    QStringList protectedMaps;

    void setupPaths();
    void setupMenus();
    void setupActions();
    void connectActions();
    void createToolsToolbar();

    void onGuardar();
    void onGuardarMapaJugable();
    void onAbrirMapaJugable();
    void loadBaseMap(const QString& cityName);
};

#endif

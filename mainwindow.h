#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "eca.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    //private methods
    void setupAutomaton();
    void runAutomaton(ECA *automaton, uint numGenerations);
    void redrawState(std::vector< std::vector<bool> > state, uint pixelSize, uint lines = 0);
    QString generateBgColorStylesheet(QColor color);
    int convertBinToDec(QString binary);
    QString convertDecToBin(int decimal);
    void addImageToGraphicsView(QImage *image);

    //private members
    Ui::MainWindow *ui;
    ECA *automaton;
    std::vector<bool> randInit;
    QImage *imageBuffer;
    bool imageWasGenerated;
    QColor alive;
    QColor dead;
    QColor background;
    QString buttonStylesheet;

private slots:
    void start();
    void zoomIn();
    void zoomOut();
    void changeColorAlive();
    void changeColorDead();
    void changeColorBackground();
    void switchInitRandom(bool randEnabled);
    void generateRandInit();
    void readBinaryRuleset(QString binary);
    void readDecimalRuleset(int decimal);
    void saveImage();
};

#endif // MAINWINDOW_H

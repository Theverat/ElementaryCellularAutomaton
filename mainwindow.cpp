#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QColorDialog>
#include <QImage>
#include <QPixmap>
#include <QRgb>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //hide progressbar (only shows up when needed, i.e. at calculations
    ui->progressBar_drawProgress->hide();

    //connect gui elements with mainwindow slots
    connect(ui->pushButton_start, SIGNAL(clicked()), this, SLOT(start()));
    connect(ui->pushButton_stop, SIGNAL(clicked()), this, SLOT(stop()));
    connect(ui->pushButton_reset, SIGNAL(clicked()), this, SLOT(reset()));

    connect(ui->checkBox_randEnabled, SIGNAL(clicked(bool)), this, SLOT(switchInitRandom(bool)));
    connect(ui->pushButton_randInit_generate, SIGNAL(clicked()), this, SLOT(generateRandInit()));

    connect(ui->lineEdit_ruleset, SIGNAL(textChanged(QString)), this, SLOT(readBinaryRuleset(QString)));
    connect(ui->spinBox_rulesetDec, SIGNAL(valueChanged(int)), this, SLOT(readDecimalRuleset(int)));

    connect(ui->pushButton_zoomPlus, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(ui->pushButton_zoomMinus, SIGNAL(clicked()), this, SLOT(zoomOut()));

    connect(ui->pushButton_colAlive, SIGNAL(clicked()), this, SLOT(changeColorAlive()));
    connect(ui->pushButton_colDead, SIGNAL(clicked()), this, SLOT(changeColorDead()));
    connect(ui->pushButton_colBackground, SIGNAL(clicked()), this, SLOT(changeColorBackground()));

    connect(ui->pushButton_saveImage, SIGNAL(clicked()), this, SLOT(saveImage()));

    //enable changes of background color to the color choser pushbuttons
    ui->pushButton_colAlive->setAutoFillBackground(true);
    ui->pushButton_colDead->setAutoFillBackground(true);
    ui->pushButton_colBackground->setAutoFillBackground(true);

    //default colors for cell types and background
    alive = QColor(250, 250, 250);
    dead = QColor(0, 0, 0);
    background = QColor(70, 70, 70);
    //default button stylesheet
    buttonStylesheet = "border-radius: 5px; padding: 5px; ";

    //apply stylesheets
    ui->pushButton_colAlive->setStyleSheet(generateBgColorStylesheet(alive) + buttonStylesheet);
    ui->pushButton_colDead->setStyleSheet(generateBgColorStylesheet(dead) + buttonStylesheet);
    ui->pushButton_colBackground->setStyleSheet(generateBgColorStylesheet(background) + buttonStylesheet);

    //match binary ruleset textbox with decimal spinbox value
    readDecimalRuleset(ui->spinBox_rulesetDec->value());

    //setup QGraphicsScene
    QGraphicsScene *scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);
    scene->clear();
    scene->setBackgroundBrush(QBrush(background));

    //setup automaton
    stopDrawingProcess = false;
    this->randInit.reserve(1);
    setupAutomaton();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupAutomaton() {
    std::vector<bool> init;
    std::vector<bool> rules;

    this->automaton = new ECA();

    if(ui->checkBox_randEnabled->isChecked()) {
        //generate random init line if none exists yet
        //(user has not created one)
        //if(randInit.size() == 1)
            generateRandInit();

        init = randInit;
    }
    else {
        //read manual init line
        QString initString = ui->lineEdit_manualInit->text();
        for(int i = 0; i < initString.length(); i++) {
            if(initString.at(i) == '0')
                init.push_back(false);
            else
                init.push_back(true);
        }
    }

    //parse rules
    QString ruleString = ui->lineEdit_ruleset->text();
    for(int i = 0; i < 8; i++) {
        if(ruleString.at(i) == '0')
            rules.push_back(false);
        else
            rules.push_back(true);
    }

    this->automaton = new ECA(init, rules);
}

void MainWindow::runAutomaton(ECA *automaton, uint numGenerations) {
    ui->progressBar_drawProgress->show();
    ui->graphicsView->setBackgroundBrush(QBrush(background));

    automaton->computeNextGeneration((unsigned int)numGenerations);
    redrawState(automaton->getState(), ui->spinBox_pixelSize->value());

    if(ui->checkBox_autoScale->isChecked())
        ui->graphicsView->fitInView(ui->graphicsView->sceneRect(), Qt::KeepAspectRatio);

    ui->progressBar_drawProgress->hide();
}

//draw current state of the automaton into the graphicsview
void MainWindow::redrawState(std::vector< std::vector<bool> > state, uint pixelSize, uint lines) {
    if(state.empty())
        return;

    int imgSizeX = state.at(0).size() * pixelSize;
    int imgSizeY = state.size() * pixelSize;
    imageBuffer = new QImage(imgSizeX, imgSizeY, QImage::Format_RGB32);

    uint maxLines;
    if(lines == 0 || lines != state.size())
        maxLines = state.size();
    else
        maxLines = lines;

    ui->graphicsView->scene()->clear();

    #pragma omp parallel for  // OpenMP
    for(int y = 0; y < (int)maxLines; y++) {
        //draw image
        //method 1: via "setPixel" (not that great performance-wise)
        for(uint x = 0; x < state.at(0).size(); x++) {
            //draw subpixels into QImage
            for(uint imgY = 0; imgY < pixelSize; imgY++) {
                for(uint imgX = 0; imgX < pixelSize; imgX++) {
                    if(state.at(y).at(x)) {
                        //cell is alive, use living cell color
                        imageBuffer->setPixel(x * pixelSize + imgX, y * pixelSize + imgY, alive.rgb());
                    }
                    else {
                        //cell is dead, use dead cell color
                        imageBuffer->setPixel(x * pixelSize + imgX, y * pixelSize + imgY, dead.rgb());
                    }
                }
            } //end subpixel drawing
        }

        /*
        //method 2: via scanline
        //not finished, is not better than setPixel performance- and memory-wise
        QRgb *pxPointer = (QRgb*)image->scanLine(y);
        for(int x = 0; x < state.at(0).size(); x++) {
            if(state.at(y).at(x)) {
                //cell is alive, use living cell color
                // *pxPointer = qRgb(qRed(alive.red()), qGreen(alive.green()), qBlue(alive.blue()));
                *pxPointer = qRgb(qRed(255), qGreen(255), qBlue(255));
            }
            else {
                //cell is dead, use dead cell color
                // *pxPointer = qRgb(qRed(alive.red()), qGreen(alive.green()), qBlue(alive.blue()));
                *pxPointer = qRgb(qRed(0), qGreen(0), qBlue(0));
            }

            //increment pointer
            pxPointer++;
        }
        */

        /* DISABLED because of OpenMP
        //invoke processEvents() (i.e. update the generations displayed)
        //each 100 generations
        if(y % 100 == 0) {
            //update progressbar
            //ui->progressBar_drawProgress->setValue((int)(100.0 / (double)state.size() * (double)y));
            //update progressbar, OpenMP version:
            emit updateProgressBar((int)(100.0 / (double)state.size() * (double)y));

            //pass calculated QImage to graphicsview
            //addImageToGraphicsView(imageBuffer);

            QCoreApplication::processEvents();
        }
        */
        /* DISABLED because of OpenMP
        if(stopDrawingProcess) {
            stopDrawingProcess = false;
            //pass calculated QImage to graphicsview
            addImageToGraphicsView(imageBuffer);
            return;
        }
        */
    }

    //pass calculated QImage to graphicsview
    addImageToGraphicsView(imageBuffer);
    imageWasGenerated = true;
}

void MainWindow::addImageToGraphicsView(QImage *image) {
    QPixmap pixmap = QPixmap();
    pixmap.convertFromImage(*image);
    ui->graphicsView->scene()->addPixmap(pixmap);
}

void MainWindow::start() {
    ui->graphicsView->scene()->clear();
    setupAutomaton();
    runAutomaton(automaton, ui->spinBox_numGens->value());
}

//interrupt drawing process of the qgraphicsview
//doesn't interrupt the calculation of generations!! Only the drawing of the scene!
void MainWindow::stop() {
    stopDrawingProcess = true;
}

//reset graphicsview
void MainWindow::reset() {
    ui->graphicsView->scene()->clear();
}

//set manual input to en-/disabled and random init buttons to opposite
void MainWindow::switchInitRandom(bool randEnabled) {
    ui->label_manualEnabled->setEnabled(!randEnabled);
    ui->lineEdit_manualInit->setEnabled(!randEnabled);
    ui->spinBox_percentAlive->setEnabled(randEnabled);
    ui->spinBox_initRandLength->setEnabled(randEnabled);
    ui->pushButton_randInit_generate->setEnabled(randEnabled);
}

//generate random init line
void MainWindow::generateRandInit() {
    this->randInit.clear();

    for(int i = 0; i < ui->spinBox_initRandLength->value(); i++) {
        //write random values to the init line vector
        //percentage of living cells is taken from the percentAlive spinbox
        randInit.push_back((rand() % 100) < ui->spinBox_percentAlive->value());
    }

    if(automaton->getState().empty()) {
        automaton->getState().push_back(randInit);
    }
    else {
        automaton->getState().at(0) = randInit;
    }
}

//convert string of binary digits to decimal number
int MainWindow::convertBinToDec(QString binary) {
    int decimal = 0;
    for(int i = 0; i < binary.length();  i++) {
        if(binary.at(i) == '1')
            decimal += (int)pow(2, binary.length() - i - 1);
    }
    return decimal;
}

//convert decimal number to QString of 0's and 1's
//the leading 0's are important for the ruleset
QString MainWindow::convertDecToBin(int decimal) {
    QString binary;

    if(decimal == 0)
        return QString("00000000");

    while(binary.length() < 8) {
        if(decimal % 2 == 1)
            binary.insert(0, '1');
        else
            binary.insert(0, '0');

        decimal /= 2;
    }
    return binary;
}

//update decimal input field after binary ruleset was changed
void MainWindow::readBinaryRuleset(QString binary) {
    bool signalsBlocked = ui->spinBox_rulesetDec->blockSignals(true);
    ui->spinBox_rulesetDec->setValue(convertBinToDec(binary));
    ui->spinBox_rulesetDec->blockSignals(signalsBlocked);
}

//update binary ruleset input field after decimal ruleset was changed
void MainWindow::readDecimalRuleset(int decimal) {
    bool signalsBlocked = ui->lineEdit_ruleset->blockSignals(true);
    ui->lineEdit_ruleset->setText(convertDecToBin(decimal));
    ui->lineEdit_ruleset->blockSignals(signalsBlocked);
}

void MainWindow::zoomIn() {
    ui->graphicsView->scale(1.2, 1.2);
}

void MainWindow::zoomOut() {
    ui->graphicsView->scale(0.8, 0.8);
}

void MainWindow::changeColorAlive() {
    //open color picker
    QColor tempAlive = QColorDialog::getColor(alive, this, "Choose color of living cells");
    if(tempAlive.isValid())
        alive = tempAlive;

    //adjust color
    ui->pushButton_colAlive->setStyleSheet(generateBgColorStylesheet(alive) + buttonStylesheet);
}

void MainWindow::changeColorDead() {
    //open color picker
    QColor tempDead = QColorDialog::getColor(dead, this, "Choose color of dead cells");
    if(tempDead.isValid())
        dead = tempDead;

    //adjust color
    ui->pushButton_colDead->setStyleSheet(generateBgColorStylesheet(dead) + buttonStylesheet);
}

void MainWindow::changeColorBackground() {
    //open color picker
    QColor tempBackground = QColorDialog::getColor(background, this, "Choose background color");
    if(tempBackground.isValid())
        background = tempBackground;

    //adjust color
    ui->pushButton_colBackground->setStyleSheet(generateBgColorStylesheet(background) + buttonStylesheet);
}

//generate stylesheet with color and white/black text color for buttons
QString MainWindow::generateBgColorStylesheet(QColor color) {
    QColor textColor;
    if(color.value() >= 140)
        textColor = QColor(Qt::black);
    else
        textColor = QColor(Qt::white);
    return QString("background-color: " + color.name() + "; color: " + textColor.name() + "; ");
}

void MainWindow::saveImage() {
    if(!imageWasGenerated) {
        QMessageBox::information(this, "No image generated yet", "very funny...");
        return;
    }

    QString filename = QFileDialog::getSaveFileName();
    std::cout << "saving image to " << filename.toStdString() << std::endl;

    if(!imageBuffer->save(filename)) {
        QMessageBox::information(this, "Error while saving image", "Image not saved!");
    }
}

/* OLD: using rectangles
//update all pixels/generations
void MainWindow::redrawState(std::vector< std::vector<bool> > state, uint pixelSize, uint lines) {
    uint maxLines;
    if(lines == 0)
        maxLines = state.size();
    else
        maxLines = lines;

    ui->graphicsView->scene()->clear();

    for(unsigned int y = 0; y < maxLines; y++) {
        for(unsigned int x = 0; x < state.at(0).size(); x++) {
            QGraphicsRectItem* item = new QGraphicsRectItem(x*pixelSize, y*pixelSize, pixelSize, pixelSize);

            //set color of cell (green for alive, dark red for dead)
            if(state.at(y).at(x))
                item->setBrush(QBrush(alive));
            else
                item->setBrush(QBrush(dead));

            //add pixel to graphicsview scene
            ui->graphicsView->scene()->addItem(item);
        }
        //invoke processEvents() (i.e. update the generations displayed)
        //each 100 generations
        if(y % 100 == 0) {
            //update progressbar
            ui->progressBar_drawProgress->setValue((int)(100.0 / (double)state.size() * (double)y));
            QCoreApplication::processEvents();
        }
        if(stopDrawingProcess) {
            stopDrawingProcess = false;
            return;
        }
    }
}
*/

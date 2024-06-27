#include "mainwindow.h"
#include "qjsonobject.h"
#include "ui_mainwindow.h"

#include <QClipboard>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QActionGroup>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <dcmtk/dcmimgle/dcmimage.h>

cv::Mat readDicom(const QString &filename)
{
    cv::Mat mat;
    DicomImage *image = new DicomImage(filename.toUtf8().constData());
    if (image != NULL) {
        if (image->getStatus() == EIS_Normal) {
            if (image->isMonochrome()) {
                image->setMinMaxWindow();
                Uint8 *pixelData = (Uint8 *)(image->getOutputData(8/*bits*/));
                if (pixelData != NULL)
                    mat = cv::Mat(image->getHeight(),image->getWidth(),CV_8UC1, pixelData).clone();
            }
        }
    }
    delete image;
    return mat;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    cpos(-1)
{
    ui->setupUi(this);
    ui->radius_slider->setFocusPolicy(Qt::NoFocus);
    setWindowTitle(QString("%1 v%2").arg(APP_NAME, APP_VERSION));

    QDir _dir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation).append("/%1").arg(APP_NAME));
    if(!_dir.exists())
        _dir.mkpath(_dir.absolutePath());
    settings = new QSettings(_dir.absolutePath().append("/%1.ini").arg(APP_NAME),QSettings::IniFormat);
    ui->radius_slider->setValue(settings->value("Radius",3).toInt());
    ui->widget->setRadius(ui->radius_slider->value());

    ui->actionEqualizeImage->setChecked(settings->value("EqualizeImage",false).toBool());

    connect(ui->widget,&QImageWidget::fileDropped,this,&MainWindow::openDirectory);

    QMap<int, QColor> colors;
    colors[101] = Qt::green;
    colors[102] = Qt::red;
    colors[103] = Qt::blue;
    colors[104] = Qt::yellow;
    colors[105] = Qt::cyan;
    colors[106] = Qt::magenta;
    colors[107] = QColor(200,25,100);
    colors[108] = QColor(100,100,220);
    colors[109] = QColor(0,100,200);
    colors[110] = QColor(255,200,50);
    colors[111] = QColor(50,200,255);
    colors[112] = QColor(200,50,100);
    colors[113] = QColor(30,200,150);
    ui->widget->setColors(colors);
    ui->actionlabel_1->trigger();
    openDirectory(settings->value("Directory",QString()).toString());
}

MainWindow::~MainWindow()
{
    commit_markup();
    delete ui;
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(ui->actioneraser);
    menu.addSeparator();
    menu.addAction(ui->actionlabel_1);
    menu.addAction(ui->actionlabel_2);
    menu.addAction(ui->actionlabel_3);
    menu.addSeparator();
    menu.addAction(ui->actionlabel_4);
    menu.addAction(ui->actionlabel_5);
    menu.addAction(ui->actionlabel_6);
    menu.addAction(ui->actionlabel_7);
    menu.addAction(ui->actionlabel_8);
    menu.addAction(ui->actionlabel_9);
    menu.addAction(ui->actionlabel_10);
    menu.addAction(ui->actionlabel_11);
    menu.addAction(ui->actionlabel_12);
    menu.addAction(ui->actionlabel_13);
    QActionGroup ag(&menu);
    ag.addAction(ui->actioneraser);
    ag.addAction(ui->actionlabel_1);
    ag.addAction(ui->actionlabel_2);
    ag.addAction(ui->actionlabel_3);
    ag.addAction(ui->actionlabel_4);
    ag.addAction(ui->actionlabel_5);
    ag.addAction(ui->actionlabel_6);
    ag.addAction(ui->actionlabel_7);
    ag.addAction(ui->actionlabel_8);
    ag.addAction(ui->actionlabel_9);
    ag.addAction(ui->actionlabel_10);
    ag.addAction(ui->actionlabel_11);
    ag.addAction(ui->actionlabel_12);
    ag.addAction(ui->actionlabel_13);
    menu.addSeparator();
    menu.addAction(ui->actionclearPoints);
    /*menu.addSeparator();
    menu.addAction(ui->actionFlipImageHorizontally);
    menu.addAction(ui->actionFlipImageVertically);
    menu.addAction(ui->actionRotateImage180);
    menu.addAction(ui->actionRotateImage90);
    menu.addAction(ui->actionRotateImageMinus90);
    menu.addSeparator();
    menu.addAction(ui->actionDeleteImage);*/
    menu.exec(event->globalPos());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(files.size() != 0) {
        commit_markup();
        if(event->key() == Qt::Key_Left)
            cpos--;
        else if(event->key() == Qt::Key_Right)
            cpos++;
        if(cpos < 0)
            cpos = 0;
        else if(cpos >= files.size())
            cpos = files.size() - 1;
        open_file_with_index(cpos);
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::on_actionclearPoints_triggered()
{
    ui->widget->clearHotmap();
    commit_markup();
}

void MainWindow::on_actionShowAbout_triggered()
{
    QDialog aboutDialog(this);
    aboutDialog.setPalette(palette());
    aboutDialog.setFont(font());
    int pointSize = aboutDialog.font().pointSize();
    QVBoxLayout layout;
    layout.setMargin(pointSize);
    QLabel appLabel(QString("%1 v.%2").arg(APP_NAME, APP_VERSION));
    appLabel.setAlignment(Qt::AlignCenter);
    QLabel authorLabel(tr("%1").arg(APP_DESIGNER));
    authorLabel.setAlignment(Qt::AlignCenter);
    QLabel infoLabel(tr("Was designed to be a handy tool for manual dicom images segmentation"));
    infoLabel.setAlignment(Qt::AlignCenter);
    infoLabel.setWordWrap(true);
    infoLabel.setMargin(pointSize);
    aboutDialog.setLayout(&layout);
    layout.addWidget(&appLabel);
    layout.addWidget(&authorLabel);
    layout.addWidget(&infoLabel);
    aboutDialog.setFixedSize(pointSize * 30, pointSize * 15);
    aboutDialog.exec();
}


void MainWindow::on_actionactionopenDirectory_triggered()
{
    const QString dirname = QFileDialog::getExistingDirectory(this,APP_NAME,settings->value("Directory",QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString());
    if(!dirname.isEmpty())
        openDirectory(dirname);
}

void MainWindow::openDirectory(const QString &name)
{
    if(!name.isEmpty()) {
        files.clear();
        settings->setValue("Directory",name);
        dir.setPath(name);
        QStringList allfiles = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
        files.reserve(allfiles.size());
        for(int i = 0; i < allfiles.size(); ++i) {
            if(allfiles.at(i).contains(".dicom") || !allfiles.at(i).contains(".png"))
                files.append(std::move(allfiles[i]));
        }
        cpos = 0;
        open_file_with_index(cpos);
    }
}

int count_markup_files(const QDir &dir)
{
    const QStringList filters = {"*.png"};
    QStringList markup_files = dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    return markup_files.size();
}

void MainWindow::open_file_with_index(int cpos)
{
    if(cpos >= 0 && cpos < files.size()) {
        const QString filename = dir.absoluteFilePath(files.at(cpos));

        cv::Mat mat = readDicom(filename);
        if(!mat.empty()) {
            if(ui->actionEqualizeImage->isChecked()) {
                std::vector<cv::Mat> channels;
                cv::split(mat,channels);
                for(auto channel: channels)
                    cv::equalizeHist(channel,channel);
                cv::merge(channels,mat);
            }
            QImage qimage;
            switch(mat.channels()) {
                case 1:
                    qimage = QImage(mat.data,mat.cols,mat.rows,mat.cols,QImage::Format_Grayscale8);
                    break;
                case 3:
                    qimage = QImage(mat.data,mat.cols,mat.rows,mat.cols*3,QImage::Format_BGR888);
                    break;
            }
            if(!qimage.isNull()) {
                ui->widget->setImage(qimage.copy());
                const QString markup_filename = filename.contains(".") ? QString("%1.png").arg(filename.section(".",0,0)) : QString("%1.png").arg(filename);
                if(QFile::exists(markup_filename)) {
                    qimage = QImage(markup_filename);
                    ui->widget->setHotmap(qimage);
                } else {
                    ui->widget->clearHotmap();
                }
            } else
                QMessageBox::warning(this,APP_NAME,tr("Can not open '%1'!").arg(filename));
        }
        ui->statusLabel->setText(filename);
        this->statusBar()->showMessage(tr("Marked up samples in current dir: %1 / %2").arg(QString::number(count_markup_files(dir)),QString::number(files.size())));
    }
}

void MainWindow::flip_current_image_vertically()
{
    if(cpos < files.size()) {
        const QString filename = dir.absoluteFilePath(files.at(cpos));
        cv::Mat bgrmat = cv::imread(filename.toStdString(),cv::IMREAD_COLOR);
        cv::flip(bgrmat,bgrmat,0);
        cv::imwrite(filename.toStdString(),bgrmat);
        open_file_with_index(cpos);
    }
}

void MainWindow::flip_current_image_horizontally()
{
    if(cpos < files.size()) {
        const QString filename = dir.absoluteFilePath(files.at(cpos));
        cv::Mat bgrmat = cv::imread(filename.toStdString(),cv::IMREAD_COLOR);
        cv::flip(bgrmat,bgrmat,1);
        cv::imwrite(filename.toStdString(),bgrmat);
        open_file_with_index(cpos);
    }
}

void MainWindow::rotate_current_image_180()
{
    if(cpos < files.size()) {
        const QString filename = dir.absoluteFilePath(files.at(cpos));
        cv::Mat bgrmat = cv::imread(filename.toStdString(),cv::IMREAD_COLOR);
        cv::rotate(bgrmat,bgrmat,cv::ROTATE_180);
        cv::imwrite(filename.toStdString(),bgrmat);
        open_file_with_index(cpos);
    }
}

void MainWindow::rotate_current_image_clockwise_90()
{
    if(cpos < files.size()) {
        const QString filename = dir.absoluteFilePath(files.at(cpos));
        cv::Mat bgrmat = cv::imread(filename.toStdString(),cv::IMREAD_COLOR);
        cv::rotate(bgrmat,bgrmat,cv::ROTATE_90_CLOCKWISE);
        cv::imwrite(filename.toStdString(),bgrmat);
        open_file_with_index(cpos);
    }
}

void MainWindow::rotate_current_image_counterclockwise_90()
{
    if(cpos < files.size()) {
        const QString filename = dir.absoluteFilePath(files.at(cpos));
        cv::Mat bgrmat = cv::imread(filename.toStdString(),cv::IMREAD_COLOR);
        cv::rotate(bgrmat,bgrmat,cv::ROTATE_90_COUNTERCLOCKWISE);
        cv::imwrite(filename.toStdString(),bgrmat);
        open_file_with_index(cpos);
    }
}

void MainWindow::delete_current_image()
{
    if(cpos < files.size()) {
        const QString filename = dir.absoluteFilePath(files.at(cpos));
        QFile::remove(filename);
        openDirectory(dir.absolutePath());
    }
}

void MainWindow::commit_markup()
{
    if(dir.exists() && files.size() > 0 && cpos < files.size()) {
        const QString extension = files.at(cpos).section('.',-1,-1);
        QString markup_filename;
        if(extension == files.at(cpos))
            markup_filename = dir.absoluteFilePath(QString("%1.png").arg(files.at(cpos)));
        else
            markup_filename = dir.absoluteFilePath(QString("%1png").arg(files.at(cpos).section(extension,0,0)));
        const QImage qimage = ui->widget->getHotmap();
        if(!qimage.isNull()) {
            qimage.save(markup_filename,"png");
        } else if(QFile::exists(markup_filename)) {
            QFile::remove(markup_filename);
        }
    }
}


void MainWindow::on_actionEqualizeImage_triggered(bool checked)
{
    settings->setValue("EqualizeImage",checked);
    open_file_with_index(cpos);
}

void MainWindow::on_actionFlipImageVertically_triggered()
{
    flip_current_image_vertically();
}


void MainWindow::on_actionFlipImageHorizontally_triggered()
{
    flip_current_image_horizontally();
}


void MainWindow::on_actionDeleteImage_triggered()
{
    delete_current_image();
}


void MainWindow::on_actionRotateImage180_triggered()
{
    rotate_current_image_180();
}


void MainWindow::on_actionRotateImage90_triggered()
{
    rotate_current_image_clockwise_90();
}


void MainWindow::on_actionRotateImageMinus90_triggered()
{
    rotate_current_image_counterclockwise_90();
}


void MainWindow::on_radius_slider_valueChanged(int value)
{
    ui->widget->setRadius(value);
    ui->radius_value_label->setText(QString::number(value));
    settings->setValue("Radius",value);
}


void MainWindow::on_actioneraser_triggered()
{
    ui->widget->setLabel(0);
}


void MainWindow::on_actionlabel_1_triggered()
{
    ui->widget->setLabel(101); // Плечеголовной ствол аорты
}

void MainWindow::on_actionlabel_2_triggered()
{
    ui->widget->setLabel(102); // Левая общая сонная артерия
}

void MainWindow::on_actionlabel_3_triggered()
{
    ui->widget->setLabel(103); // Левая подключичная артерия
}

void MainWindow::on_actionlabel_4_triggered()
{
    ui->widget->setLabel(104); // Нижняя диафрагмальная артерия левая/правая
}

void MainWindow::on_actionlabel_5_triggered()
{
    ui->widget->setLabel(105); // Нижняя диафрагмальная артерия левая/правая
}

void MainWindow::on_actionlabel_6_triggered()
{
    ui->widget->setLabel(106); // Плечеголовной ствол аорты
}

void MainWindow::on_actionlabel_7_triggered()
{
    ui->widget->setLabel(107); // Левая общая сонная артерия
}

void MainWindow::on_actionlabel_8_triggered()
{
    ui->widget->setLabel(108); // Левая подключичная артерия
}

void MainWindow::on_actionlabel_9_triggered()
{
    ui->widget->setLabel(109); // Нижняя диафрагмальная артерия левая/правая
}

void MainWindow::on_actionlabel_10_triggered()
{
    ui->widget->setLabel(110); // Нижняя диафрагмальная артерия левая/правая
}

void MainWindow::on_actionlabel_11_triggered()
{
    ui->widget->setLabel(111); // Плечеголовной ствол аорты
}


void MainWindow::on_actionlabel_12_triggered()
{
    ui->widget->setLabel(112); // Левая общая сонная артерия
}


void MainWindow::on_actionlabel_13_triggered()
{
    ui->widget->setLabel(113); // Левая подключичная артерия
}

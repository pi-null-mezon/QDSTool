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
    ui->intensitySlider->setFocusPolicy(Qt::NoFocus);
    ui->toolButton->setFocusPolicy(Qt::NoFocus);
    setWindowTitle(QString("%1 v%2").arg(APP_NAME, APP_VERSION));

    QDir _dir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation).append("/%1").arg(APP_NAME));
    if(!_dir.exists())
        _dir.mkpath(_dir.absolutePath());
    settings = new QSettings(_dir.absolutePath().append("/%1.ini").arg(APP_NAME),QSettings::IniFormat);
    ui->radius_slider->setValue(settings->value("Radius",3).toInt());
    ui->widget->setRadius(ui->radius_slider->value());
    ui->intensitySlider->setValue(settings->value("Intensity",100).toInt());
    ui->widget->setIntensity(ui->intensitySlider->value());
    ui->toolButton->setDefaultAction(ui->actionEqualizeImage);
    ui->actionEqualizeImage->setChecked(settings->value("EqualizeImage",false).toBool());

    connect(ui->widget,&QImageWidget::fileDropped,this,&MainWindow::openDirectory);

    QMap<int, QColor> colors;
    for(int i = 101; i <= 150; ++i) { // normal vessels
        int c = i - 100;
        colors[i] = QColor(c % 2 ? 5*c : 3*c,
                           c % 2 ? 100 + ((c * 17) % 155) : 255 - ((c * 17) % 155),
                           c % 2 ? 255 - ((c * 17) % 155) : 100 + ((c * 17) % 155));
    } for(int i = 201; i <= 250; ++i) { // aneurysm
        int c = i - 200;
        colors[i] = QColor(255,
                           c % 2 ? 0 + ((c * 17) % 155) : 100 - ((c * 17) % 100),
                           c % 2 ? 100 - ((c * 17) % 100) : 0 + ((c * 17) % 155));
    }
    ui->widget->setColors(colors);
    ui->actionlabel_14->trigger();
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
    //menu.addSeparator();
    menu.addAction(ui->actionaneurysm);
    menu.addSeparator();
    menu.addAction(ui->actionlabel_14);
    menu.addAction(ui->actionlabel_15);
    menu.addAction(ui->actionlabel_16);
    menu.addSeparator();
    menu.addAction(ui->actionlabel_1);
    menu.addAction(ui->actionlabel_2);
    menu.addAction(ui->actionlabel_3);
    menu.addSeparator();
    menu.addAction(ui->actionlabel_4);
    menu.addAction(ui->actionlabel_17);
    menu.addAction(ui->actionlabel_5);
    menu.addAction(ui->actionlabel_6);
    menu.addAction(ui->actionlabel_7);
    menu.addAction(ui->actionlabel_8);   
    menu.addAction(ui->actionlabel_9);
    menu.addAction(ui->actionlabel_10);
    menu.addAction(ui->actionlabel_11);
    menu.addAction(ui->actionlabel_19);
    menu.addAction(ui->actionlabel_12);
    menu.addAction(ui->actionlabel_13);
    menu.addAction(ui->actionlabel_18);
    QActionGroup ag(&menu);
    ag.addAction(ui->actioneraser);
    ag.addAction(ui->actionlabel_1);
    ag.addAction(ui->actionlabel_2);
    ag.addAction(ui->actionlabel_3);
    ag.addAction(ui->actionlabel_4);
    ag.addAction(ui->actionlabel_17);
    ag.addAction(ui->actionlabel_5);
    ag.addAction(ui->actionlabel_6);
    ag.addAction(ui->actionlabel_7);
    ag.addAction(ui->actionlabel_8);
    ag.addAction(ui->actionlabel_9);
    ag.addAction(ui->actionlabel_10);
    ag.addAction(ui->actionlabel_11);
    ag.addAction(ui->actionlabel_19);
    ag.addAction(ui->actionlabel_12);
    ag.addAction(ui->actionlabel_13);
    ag.addAction(ui->actionlabel_18);
    ag.addAction(ui->actionlabel_14);
    ag.addAction(ui->actionlabel_15);
    ag.addAction(ui->actionlabel_16);
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
    commit_markup();
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
    ui->curentlblLabel->setText(ui->actioneraser->text());
}

void MainWindow::on_actionlabel_1_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 201 : 101);
    ui->curentlblLabel->setText(ui->actionlabel_1->text());
}

void MainWindow::on_actionlabel_2_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 202 : 102);
    ui->curentlblLabel->setText(ui->actionlabel_2->text());
}

void MainWindow::on_actionlabel_3_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 203 : 103);
    ui->curentlblLabel->setText(ui->actionlabel_3->text());
}

void MainWindow::on_actionlabel_4_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 204 : 104);
    ui->curentlblLabel->setText(ui->actionlabel_4->text());
}

void MainWindow::on_actionlabel_5_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 205 : 105);
    ui->curentlblLabel->setText(ui->actionlabel_5->text());
}

void MainWindow::on_actionlabel_6_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 206 : 106);
    ui->curentlblLabel->setText(ui->actionlabel_6->text());
}

void MainWindow::on_actionlabel_7_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 207 : 107);
    ui->curentlblLabel->setText(ui->actionlabel_7->text());
}

void MainWindow::on_actionlabel_8_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 208 : 108);
    ui->curentlblLabel->setText(ui->actionlabel_8->text());
}

void MainWindow::on_actionlabel_9_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 209 : 109);
    ui->curentlblLabel->setText(ui->actionlabel_9->text());
}

void MainWindow::on_actionlabel_10_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 210 : 110);
    ui->curentlblLabel->setText(ui->actionlabel_10->text());
}

void MainWindow::on_actionlabel_11_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 211 : 111);
    ui->curentlblLabel->setText(ui->actionlabel_11->text());
}

void MainWindow::on_actionlabel_12_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 212 : 112);
    ui->curentlblLabel->setText(ui->actionlabel_12->text());
}

void MainWindow::on_actionlabel_13_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 213 : 113);
    ui->curentlblLabel->setText(ui->actionlabel_13->text());
}

void MainWindow::on_actionlabel_14_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 214 : 114);
    ui->curentlblLabel->setText(ui->actionlabel_14->text());
}

void MainWindow::on_actionlabel_15_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 215 : 115);
    ui->curentlblLabel->setText(ui->actionlabel_15->text());
}

void MainWindow::on_actionlabel_16_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 216 : 116);
    ui->curentlblLabel->setText(ui->actionlabel_16->text());
}

void MainWindow::on_actionlabel_17_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 217 : 117);
    ui->curentlblLabel->setText(ui->actionlabel_17->text());
}

void MainWindow::on_actionlabel_18_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 218 : 118);
    ui->curentlblLabel->setText(ui->actionlabel_18->text());
}

void MainWindow::on_actionlabel_19_triggered()
{
    ui->widget->setLabel(ui->actionaneurysm->isChecked() ? 219 : 119);
    ui->curentlblLabel->setText(ui->actionlabel_19->text());
}

void MainWindow::on_intensitySlider_valueChanged(int value)
{
    ui->widget->setIntensity(value);
    settings->setValue("Intensity",value);
}

void MainWindow::on_actionaneurysm_triggered()
{
    const uint8_t label = ui->widget->getLabel();
    QPalette palette = ui->curentlblLabel->palette();
    if(label > 0) {
        if(ui->actionaneurysm->isChecked()) {
            ui->widget->setLabel(label + 100);
            palette.setBrush(QPalette::WindowText, Qt::red);
        } else {
            ui->widget->setLabel(label - 100);
            palette.setBrush(QPalette::WindowText, Qt::white);
        }
        ui->curentlblLabel->setPalette(palette);
    }
}

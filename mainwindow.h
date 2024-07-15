#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTextStream>

#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QDir>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void on_actionclearPoints_triggered();
    void on_actionShowAbout_triggered();
    void on_actionactionopenDirectory_triggered();

    void openDirectory(const QString &filename);
    void open_file_with_index(int pos);
    void flip_current_image_vertically();
    void flip_current_image_horizontally();
    void rotate_current_image_180();
    void rotate_current_image_clockwise_90();
    void rotate_current_image_counterclockwise_90();
    void delete_current_image();
    void commit_markup();

    void on_actionEqualizeImage_triggered(bool checked);

    void on_actionFlipImageVertically_triggered();

    void on_actionFlipImageHorizontally_triggered();

    void on_actionRotateImage180_triggered();

    void on_actionDeleteImage_triggered();

    void on_actionRotateImage90_triggered();

    void on_actionRotateImageMinus90_triggered();

    void on_radius_slider_valueChanged(int value);

    void on_actioneraser_triggered();

    void on_actionlabel_1_triggered();

    void on_actionlabel_2_triggered();

    void on_actionlabel_3_triggered();

    void on_actionlabel_4_triggered();

    void on_actionlabel_5_triggered();

    void on_actionlabel_6_triggered();

    void on_actionlabel_7_triggered();

    void on_actionlabel_8_triggered();

    void on_actionlabel_9_triggered();

    void on_actionlabel_10_triggered();

    void on_actionlabel_11_triggered();

    void on_actionlabel_12_triggered();

    void on_actionlabel_13_triggered();

    void on_intensitySlider_valueChanged(int value);

    void on_actionlabel_14_triggered();

    void on_actionlabel_15_triggered();

    void on_actionlabel_16_triggered();

    void on_actionaneurysm_triggered();

private:
    Ui::MainWindow *ui;
    QSettings *settings;

    QDir dir;
    QStringList files;
    int cpos;
};
#endif // MAINWINDOW_H

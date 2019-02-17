#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QMainWindow>

QT_FORWARD_DECLARE_CLASS(QOpenGLShader)
struct ShaderProgramContext;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void showFileOpenDialog();
    void notifyShaderCompilation(const QOpenGLShader* shader, bool result);
    void updateFpsLabel(float fps);
    void updateWindowTitle(const ShaderProgramContext& ctx);
    void setPause(bool enabled);
private:
    Ui::MainWindow *_ui;
    QDir _lastOpenDir;
};

#endif // MAINWINDOW_H

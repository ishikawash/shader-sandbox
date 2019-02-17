#include <QFileInfo>
#include <QFileDialog>
#include <QStyle>
#include <QOpenGLShader>
#include <QtDebug>
#include "ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _lastOpenDir(QDir::homePath())
{
    _ui->setupUi(this);

    connect(_ui->fileOpenAction, SIGNAL(triggered(bool)), this, SLOT(showFileOpenDialog()));
    connect(_ui->shaderCompileAction, SIGNAL(triggered(bool)), _ui->glWidget->builder(), SLOT(compileShader()));
    connect(_ui->playButton, SIGNAL(toggled(bool)), this, SLOT(setPause(bool)));
    connect(_ui->glWidget, SIGNAL(fpsUpdated(float)), this, SLOT(updateFpsLabel(float)));
    connect(_ui->glWidget, &GLWidget::initialized, this, [this] { this->setPause(false); });
    connect(_ui->glWidget->builder(), SIGNAL(shaderCompiled(const QOpenGLShader*, bool)), this, SLOT(notifyShaderCompilation(const QOpenGLShader*, bool)));
    connect(_ui->glWidget->builder(), SIGNAL(shaderProgramUpdated(const ShaderProgramContext&)), this, SLOT(updateWindowTitle(const ShaderProgramContext&)));

    _ui->dockWidget->hide();
    _ui->viewMenu->addAction(_ui->dockWidget->toggleViewAction());
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::showFileOpenDialog()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open Shader File"), _lastOpenDir.path(), tr("GLSL (*.shader *.frag *.glsl)"));
    if (path.length() == 0)
    {
        return;
    }
    QFileInfo info(path);
    _lastOpenDir = info.absoluteDir();
    _ui->glWidget->builder()->compileShader(path);
}

void MainWindow::notifyShaderCompilation(const QOpenGLShader* shader, bool result)
{
    QPlainTextEdit* edit = _ui->logMessageEdit;
    if (result)
    {
        edit->appendPlainText("Compilation was successful.");
        return;
    }
    const QString message = shader->log();
    if (message.length() > 0) {
        edit->appendPlainText(message);
        _ui->dockWidget->show();
    }
}

void MainWindow::updateFpsLabel(float fps)
{
    QString text = QString::asprintf("%.1f FPS", fps);
    _ui->fpsLabel->setText(text);
}

void MainWindow::updateWindowTitle(const ShaderProgramContext& ctx)
{
    QFileInfo info(ctx.fragmentShaderFilePath);
    setWindowTitle(info.fileName());
}

void MainWindow::setPause(bool enabled)
{
    _ui->glWidget->setPause(enabled);
    if (enabled) {
        _ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    } else {
        _ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }
}

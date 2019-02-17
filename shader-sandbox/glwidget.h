#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <memory>
#include <chrono>
#include <QQueue>
#include <QTime>
#include <QTimer>
#include <QVector2D>
#include <QMatrix4x4>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include "shaderprogrambuilder.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShader)
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLWidget(QWidget* parent = nullptr);
    ~GLWidget() override;
    ShaderProgramBuilder* builder();
signals:
    void initialized();
    void fpsUpdated(float fps);
public slots:
    void setPause(bool enabled);
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
private:
    typedef std::unique_ptr<QOpenGLBuffer> GeometoryBufferPointer;
    struct TimeStat
    {
        int frameCount = 0;
        int64_t duration = 0;
        int64_t elapsedTime = 0;
        std::chrono::time_point<std::chrono::system_clock> lastFrameTime;
    };

    void createBillboardBuffer();
    void render();
    int64_t updateTimeStat();
    void measureFrameRate(int64_t dt);

    GeometoryBufferPointer _vertexBuffer = nullptr;
    GeometoryBufferPointer _indexBuffer = nullptr;
    QMatrix4x4 _projectionMatrix;
    QVector2D _resolution;
    QVector2D _mouse;
    QTimer _timer;
    TimeStat _timeStat;
    ShaderProgramBuilder _builder;
};

#endif // GLWIDGET_H

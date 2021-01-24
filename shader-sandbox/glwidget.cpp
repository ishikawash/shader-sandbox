#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTimerEvent>
#include <QMimeData>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QtDebug>
#include "glwidget.h"

#define FRAME_COUNT_INTERVAL  1000

GLWidget::GLWidget(QWidget* parent) : QOpenGLWidget (parent), _timer(this), _builder(this)
{

}

GLWidget::~GLWidget()
{
    _vertexBuffer->destroy();
    _indexBuffer->destroy();
}

ShaderProgramBuilder* GLWidget::builder()
{
    return &_builder;
}

void GLWidget::setPause(bool enabled)
{
    if (enabled)
    {
        _timer.stop();
    } else
    {
        if (!_timer.isActive())
        {
            _timeStat.lastFrameTime = std::chrono::system_clock::now();
            _timer.start(16);
        }
    }
}

void GLWidget::createBillboardBuffer()
{
    const float vertices[] = {
        0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
    };
    const int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    _vertexBuffer.reset(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer));
    _vertexBuffer->create();
    _vertexBuffer->bind();
    _vertexBuffer->allocate(vertices, sizeof(vertices));
    _vertexBuffer->release();

    _indexBuffer.reset(new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer));
    _indexBuffer->create();
    _indexBuffer->bind();
    _indexBuffer->allocate(indices, sizeof(indices));
    _indexBuffer->release();
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    qDebug() << "OpenGL version: " << QLatin1String(reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    _projectionMatrix.ortho(-0.5f, 0.5f, -0.5f, 0.5f, 0.1f, 10.0f);
    _builder.compileShader(":/default.vert", ":/default.frag");
    createBillboardBuffer();

    connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
    emit(initialized());
}

void GLWidget::paintGL()
{
    updateTimeStat();
    _builder.swapContext();
    render();
}

int64_t GLWidget::updateTimeStat()
{
    auto t = std::chrono::system_clock::now();
    auto diff = t - _timeStat.lastFrameTime;
    int64_t dt = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
    if (_timer.isActive())
    {
        _timeStat.elapsedTime += dt;
        measureFrameRate(dt);
    }
    _timeStat.lastFrameTime = t;
    return dt;
}

void GLWidget::measureFrameRate(int64_t dt)
{
    _timeStat.duration += dt;
    if (_timeStat.duration > FRAME_COUNT_INTERVAL)
    {
        float fps = _timeStat.frameCount / (FRAME_COUNT_INTERVAL / 1000.0f);
        _timeStat.frameCount = 0;
        _timeStat.duration -= FRAME_COUNT_INTERVAL;
        emit(fpsUpdated(fps));
    } else {
        _timeStat.frameCount++;
    }
}

void GLWidget::render()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    QMatrix4x4 modelView;
    modelView.translate(-0.5f, -0.5f, -1.0f);

    ShaderProgramContext& ctx = _builder.currentContext();
    ctx.program->bind();
    {
        ctx.program->setUniformValue(ctx.localToCameraMatrixLocation, modelView);
        ctx.program->setUniformValue(ctx.projectionMatrixLocation, _projectionMatrix);
        ctx.program->setUniformValue(ctx.resolutionLocation, _resolution);
        ctx.program->setUniformValue(ctx.mouseLocation, _mouse);
        ctx.program->setUniformValue(ctx.timeLocation, _timeStat.elapsedTime / 1000.0f); // ms -> sec
        _vertexBuffer->bind();
        _indexBuffer->bind();
        {
            ctx.program->enableAttributeArray(ctx.vertexPositionLocation);
            ctx.program->setAttributeBuffer(ctx.vertexPositionLocation, GL_FLOAT, 0, 4, 0);
            const size_t indexCount = _indexBuffer->size() / sizeof (int);
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        }
        _indexBuffer->release();
        _vertexBuffer->release();
    }
    ctx.program->release();

    glDisable(GL_DEPTH_TEST);
}

void GLWidget::resizeGL(int width, int height)
{
    float pixelRatio = QApplication::primaryScreen()->devicePixelRatio();
    float _width = static_cast<float>(width);
    float _height = static_cast<float>(height);
    _resolution.setX(_width);
    _resolution.setY(_height);
    _resolution *= pixelRatio;

    glViewport(0, 0, width, height);
}

void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
    float _width = static_cast<float>(width());
    float _height = static_cast<float>(height());
    _mouse.setX(event->x() / _width);
    _mouse.setY(event->y() / _height);
}

void GLWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (!event->mimeData()->hasUrls()) {
        return;
    }
    event->setDropAction(Qt::LinkAction);
    event->setAccepted(true);
}

void GLWidget::dropEvent(QDropEvent* event)
{
    if (!event->mimeData()->hasUrls()) {
        return;
    }
    auto urls = event->mimeData()->urls();
    if (urls.empty()) {
        return;
    }
    const QUrl& url = urls.first();
    event->setAccepted(true);

    const QString path = url.toLocalFile();
    _builder.compileShader(path);
}

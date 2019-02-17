#ifndef SHADERPROGRAMBUILD_H
#define SHADERPROGRAMBUILD_H

#include <memory>
#include <QObject>
#include <QString>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

typedef std::unique_ptr<QOpenGLShader> ShaderObjectPointer;
typedef std::unique_ptr<QOpenGLShaderProgram> ShaderProgramPointer;

struct ShaderProgramContext
{
    ShaderProgramPointer program = nullptr;
    ShaderObjectPointer vertexShader = nullptr;
    ShaderObjectPointer fragmentShader = nullptr;
    QString vertexShaderFilePath;
    QString fragmentShaderFilePath;
    int vertexPositionLocation = 0;
    int localToCameraMatrixLocation = 0;
    int projectionMatrixLocation = 0;
    int timeLocation = 0;
    int mouseLocation = 0;
    int resolutionLocation = 0;
    bool available = false;
};

class ShaderProgramBuilder : public QObject
{
    Q_OBJECT
public:
    explicit ShaderProgramBuilder(QObject* parent = nullptr);
    ShaderProgramContext& currentContext();
    void swapContext();
signals:
    void shaderCompiled(const QOpenGLShader* shader, bool result);
    void shaderProgramUpdated(const ShaderProgramContext& ctx);
public slots:
    void compileShader();
    void compileShader(const QString& path);
    void compileShader(const QString& vsPath, const QString& fsPath);
private:
    bool buildProgram(ShaderProgramContext& ctx);
    int nextContextIndex() const;
    ShaderProgramContext _contexts[2];
    int _contextIndex = 0;
};

#endif // SHADERPROGRAMBUILD_H

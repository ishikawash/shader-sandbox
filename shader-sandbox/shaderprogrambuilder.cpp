#include "shaderprogrambuilder.h"

ShaderProgramBuilder::ShaderProgramBuilder(QObject *parent) : QObject(parent)
{

}

void ShaderProgramBuilder::compileShader()
{
    compileShader(currentContext().fragmentShaderFilePath);
}

void ShaderProgramBuilder::compileShader(const QString& path)
{
    compileShader(currentContext().vertexShaderFilePath, path);
}

void ShaderProgramBuilder::compileShader(const QString& vsPath, const QString& fsPath)
{
    int i = nextContextIndex();
    ShaderProgramContext& ctx = _contexts[i];
    ctx.vertexShaderFilePath = vsPath;
    ctx.fragmentShaderFilePath = fsPath;
    buildProgram(ctx);
}

ShaderProgramContext& ShaderProgramBuilder::currentContext()
{
    return _contexts[_contextIndex];
}

int ShaderProgramBuilder::nextContextIndex() const
{
    return (_contextIndex + 1) % 2;
}

void ShaderProgramBuilder::swapContext()
{
    int i = nextContextIndex();
    if (_contexts[i].available) {
        currentContext().available = false;
        _contextIndex = i;
        emit(shaderProgramUpdated(currentContext()));
    }
}

bool ShaderProgramBuilder::buildProgram(ShaderProgramContext& ctx)
{
    bool result = false;
    ctx.available = false;

    ShaderObjectPointer vs(new QOpenGLShader(QOpenGLShader::Vertex));
    result = vs->compileSourceFile(ctx.vertexShaderFilePath);
    emit(shaderCompiled(vs.get(), result));
    if (!result) {
        return false;
    }

    ShaderObjectPointer fs(new QOpenGLShader(QOpenGLShader::Fragment));
    result = fs->compileSourceFile(ctx.fragmentShaderFilePath);
    emit(shaderCompiled(fs.get(), result));
    if (!result) {
        return false;
    }

    ShaderProgramPointer program(new QOpenGLShaderProgram);
    program->addShader(vs.get());
    program->addShader(fs.get());
    if (!program->link()) {
        return false;
    }

    ctx.program.swap(program);
    ctx.vertexShader.swap(vs);
    ctx.fragmentShader.swap(fs);
    ctx.program->bind();
    {
        ctx.vertexPositionLocation = ctx.program->attributeLocation("_VertexPosition");
        ctx.localToCameraMatrixLocation = ctx.program->uniformLocation("_LocalToCameraMatrix");
        ctx.projectionMatrixLocation = ctx.program->uniformLocation("_ProjectionMatrix");
        ctx.timeLocation = ctx.program->uniformLocation("_Time");
        ctx.mouseLocation = ctx.program->uniformLocation("_Mouse");
        ctx.resolutionLocation = ctx.program->uniformLocation("_Resolution");
    }
    ctx.program->release();
    ctx.available = true;

    return true;
}

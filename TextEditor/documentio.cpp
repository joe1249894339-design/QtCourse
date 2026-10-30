#include "documentio.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif

namespace {

constexpr qint64 kMaxFileSize = 50 * 1024 * 1024; // 50 MB

QByteArray utf8Bom()
{
    return QByteArray::fromHex("EFBBBF");
}

QByteArray utf16LeBom()
{
    return QByteArray::fromHex("FFFE");
}

QByteArray utf16BeBom()
{
    return QByteArray::fromHex("FEFF");
}

bool decodeUtf8(const QByteArray &bytes, QString &out)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStringDecoder decoder(QStringConverter::Utf8);
    out = decoder.decode(bytes);
    return !decoder.hasError();
#else
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    if (!codec)
        return false;
    QTextCodec::ConverterState state;
    out = codec->toUnicode(bytes.constData(), bytes.size(), &state);
    return state.invalidChars == 0 && state.remainingChars == 0;
#endif
}

bool decodeSystem(const QByteArray &bytes, QString &out)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStringDecoder decoder(QStringConverter::System);
    out = decoder.decode(bytes);
    return !decoder.hasError();
#else
    QTextCodec *codec = QTextCodec::codecForLocale();
    if (!codec)
        return false;
    QTextCodec::ConverterState state;
    out = codec->toUnicode(bytes.constData(), bytes.size(), &state);
    return state.invalidChars == 0 && state.remainingChars == 0;
#endif
}

bool decodeUtf16(const QByteArray &bytes, bool littleEndian, QString &out)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStringDecoder decoder(littleEndian ? QStringConverter::Utf16LE
                                        : QStringConverter::Utf16BE);
    out = decoder.decode(bytes);
    return !decoder.hasError();
#else
    QTextCodec *codec = QTextCodec::codecForName(littleEndian ? "UTF-16LE" : "UTF-16BE");
    if (!codec)
        return false;
    QTextCodec::ConverterState state;
    out = codec->toUnicode(bytes.constData(), bytes.size(), &state);
    return state.invalidChars == 0 && state.remainingChars == 0;
#endif
}

QByteArray encodeSystem(const QString &text, bool *ok)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStringEncoder encoder(QStringConverter::System);
    const QByteArray bytes = encoder.encode(text);
    if (ok)
        *ok = !encoder.hasError();
    return bytes;
#else
    QTextCodec *codec = QTextCodec::codecForLocale();
    if (!codec) {
        if (ok)
            *ok = false;
        return QByteArray();
    }
    if (ok)
        *ok = true;
    return codec->fromUnicode(text);
#endif
}

QByteArray encodeUtf16(const QString &text, bool littleEndian)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStringEncoder encoder(littleEndian ? QStringConverter::Utf16LE
                                        : QStringConverter::Utf16BE);
    return encoder.encode(text);
#else
    QTextCodec *codec = QTextCodec::codecForName(littleEndian ? "UTF-16LE" : "UTF-16BE");
    return codec ? codec->fromUnicode(text) : QByteArray();
#endif
}

} // namespace

bool DocumentIO::read(const QString &path, DocumentData &data, QString &error)
{
    data = DocumentData();
    error.clear();

    if (path.isEmpty()) {
        error = QStringLiteral("文件路径为空。");
        return false;
    }

    const QFileInfo info(path);
    if (!info.exists()) {
        error = QStringLiteral("文件不存在：%1").arg(QDir::toNativeSeparators(path));
        return false;
    }
    if (!info.isFile()) {
        error = QStringLiteral("所选路径不是普通文件：%1").arg(QDir::toNativeSeparators(path));
        return false;
    }
    if (info.size() > kMaxFileSize) {
        error = QStringLiteral("文件过大（超过 50 MB），暂不支持打开。");
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        error = QStringLiteral("无法打开文件：%1").arg(file.errorString());
        return false;
    }

    QByteArray bytes = file.readAll();
    if (file.error() != QFile::NoError) {
        error = QStringLiteral("读取文件失败：%1").arg(file.errorString());
        return false;
    }
    file.close();

    data.path = info.absoluteFilePath();
    data.encoding = TextEncoding::Utf8;

    if (bytes.isEmpty()) {
        data.text.clear();
        return true;
    }

    QString text;
    if (bytes.startsWith(utf8Bom())) {
        bytes.remove(0, utf8Bom().size());
        if (!decodeUtf8(bytes, text)) {
            error = QStringLiteral("文件带有 UTF-8 BOM，但内容不是合法的 UTF-8 文本。");
            return false;
        }
        data.encoding = TextEncoding::Utf8Bom;
    } else if (bytes.startsWith(utf16LeBom())) {
        bytes.remove(0, utf16LeBom().size());
        if (!decodeUtf16(bytes, true, text)) {
            error = QStringLiteral("文件不是合法的 UTF-16 LE 文本。");
            return false;
        }
        data.encoding = TextEncoding::Utf16LE;
    } else if (bytes.startsWith(utf16BeBom())) {
        bytes.remove(0, utf16BeBom().size());
        if (!decodeUtf16(bytes, false, text)) {
            error = QStringLiteral("文件不是合法的 UTF-16 BE 文本。");
            return false;
        }
        data.encoding = TextEncoding::Utf16BE;
    } else {
        // 没有 BOM 时，含 NUL 字节基本可以判定为二进制文件。
        if (bytes.contains('\0')) {
            error = QStringLiteral("该文件包含二进制数据，可能不是文本文件。");
            return false;
        }
        if (decodeUtf8(bytes, text)) {
            data.encoding = TextEncoding::Utf8;
        } else if (decodeSystem(bytes, text)) {
            // 中文 Windows 下通常是 GBK/GB18030，保存时沿用该编码。
            data.encoding = TextEncoding::System;
        } else {
            error = QStringLiteral("无法识别文件编码（既不是 UTF-8，也不是系统编码）。");
            return false;
        }
    }

    data.text = text;
    return true;
}

bool DocumentIO::write(const QString &path, const QString &text,
                       TextEncoding encoding, QString &error)
{
    error.clear();

    if (path.isEmpty()) {
        error = QStringLiteral("保存路径为空。");
        return false;
    }

    QByteArray bytes;
    switch (encoding) {
    case TextEncoding::Utf8:
        bytes = text.toUtf8();
        break;
    case TextEncoding::Utf8Bom:
        bytes = utf8Bom() + text.toUtf8();
        break;
    case TextEncoding::System: {
        bool ok = false;
        bytes = encodeSystem(text, &ok);
        if (!ok) {
            error = QStringLiteral("当前文本包含系统编码无法表示的字符，请改用 UTF-8 保存。");
            return false;
        }
        break;
    }
    case TextEncoding::Utf16LE:
        bytes = utf16LeBom() + encodeUtf16(text, true);
        break;
    case TextEncoding::Utf16BE:
        bytes = utf16BeBom() + encodeUtf16(text, false);
        break;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        error = QStringLiteral("无法写入文件：%1").arg(file.errorString());
        return false;
    }

    if (file.write(bytes) != bytes.size()) {
        error = QStringLiteral("写入文件失败：%1").arg(file.errorString());
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        error = QStringLiteral("保存文件失败：%1").arg(file.errorString());
        return false;
    }

    return true;
}

QString DocumentIO::encodingName(TextEncoding encoding)
{
    switch (encoding) {
    case TextEncoding::Utf8:
        return QStringLiteral("UTF-8");
    case TextEncoding::Utf8Bom:
        return QStringLiteral("UTF-8 (BOM)");
    case TextEncoding::System:
        return QStringLiteral("系统编码 (GBK)");
    case TextEncoding::Utf16LE:
        return QStringLiteral("UTF-16 LE");
    case TextEncoding::Utf16BE:
        return QStringLiteral("UTF-16 BE");
    }
    return QStringLiteral("UTF-8");
}

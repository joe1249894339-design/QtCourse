#ifndef DOCUMENTIO_H
#define DOCUMENTIO_H

#include <QString>

/** 文件保存时使用的文本编码。打开文件时自动识别，保存时沿用原编码。 */
enum class TextEncoding {
    Utf8,
    Utf8Bom,
    System,
    Utf16LE,
    Utf16BE
};

struct DocumentData {
    QString text;
    QString path;
    TextEncoding encoding = TextEncoding::Utf8;
};

/**
 * 文本文件读写封装：
 * - 优先按 UTF-8 解码，失败后回退到系统编码（中文 Windows 下通常是 GBK）
 * - 识别 UTF-8 BOM / UTF-16 BOM
 * - 拒绝二进制文件，避免把乱码塞进编辑器
 * - 使用 QSaveFile 原子写入，写入失败不会破坏原文件
 */
class DocumentIO
{
public:
    static bool read(const QString &path, DocumentData &data, QString &error);
    static bool write(const QString &path, const QString &text,
                      TextEncoding encoding, QString &error);
    static QString encodingName(TextEncoding encoding);
};

#endif // DOCUMENTIO_H

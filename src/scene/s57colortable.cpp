#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>

#include "scene/s57colortable.h"

S57ColorTable::S57ColorTable(const QString &chartsymbolsPath)
{
    QFile file(chartsymbolsPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "S57ColorTable: cannot open" << chartsymbolsPath;
        return;
    }

    QXmlStreamReader xml(&file);
    bool inDayBright = false;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QStringLiteral("color-table")) {
                inDayBright = xml.attributes().value(QStringLiteral("name")) == QStringLiteral("DAY_BRIGHT");
            } else if (inDayBright && xml.name() == QStringLiteral("color")) {
                const auto attrs = xml.attributes();
                const QString name = attrs.value(QStringLiteral("name")).toString();
                const int r = attrs.value(QStringLiteral("r")).toInt();
                const int g = attrs.value(QStringLiteral("g")).toInt();
                const int b = attrs.value(QStringLiteral("b")).toInt();
                m_colors[name] = QColor(r, g, b);
            }
        } else if (xml.isEndElement()) {
            if (xml.name() == QStringLiteral("color-table") && inDayBright) {
                break; // Done with DAY_BRIGHT table
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "S57ColorTable: XML parse error:" << xml.errorString();
    }
}

QColor S57ColorTable::color(const QString &name) const
{
    return m_colors.value(name, QColor());
}

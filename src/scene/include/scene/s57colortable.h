#pragma once

#include <QColor>
#include <QMap>
#include <QString>

#include "scene_export.h"

// Parses the DAY_BRIGHT color table from OpenCPN's chartsymbols.xml and
// provides named S-52 colors for use in chart rendering.
class SCENE_EXPORT S57ColorTable
{
public:
    explicit S57ColorTable(const QString &chartsymbolsPath);

    // Returns the named color (e.g. "LANDA", "DEPDW"). Returns an invalid
    // QColor if the name is not found.
    QColor color(const QString &name) const;

private:
    QMap<QString, QColor> m_colors;
};

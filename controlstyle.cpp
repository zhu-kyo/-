#include "controlstyle.h"

#include <QString>

ControlStyle::ControlStyle()
{

}

QString ControlStyle::scrollBarStyle()
{
    return QStringLiteral("QScrollBar:vertical {"
                            "width: 8px;"
                            "background: transparent;"
                            "padding: 12px 0;"
                          "}"

                          "QScrollBar::handle:vertical {"
                            "background: rgba(255, 255, 255, 100);"
                            "border-radius: 4px;"
                            "min-height: 20px;"
                          "}"

                          "QScrollBar::handle:vertical:hover {"
                            "background: rgba(255, 255, 255, 150);"
                            "border-radius: 4px;"
                            "min-height: 20px;"
                          "}"

                          "QScrollBar::sub-line:vertical {"
                            "width: 12px;"
                            "height: 12px;"
                            "background: transparent;"
                            "subcontrol-position: top;"
                            "subcontrol-origin: padding;"
                          "}"

                          "QScrollBar::sub-line:vertical:hover {"
                            "width: 12px;"
                            "height: 12px;"
                            "border-radius: 6px;"
                            "background: rgba(255, 255, 255, 128);"
                            "subcontrol-position: top;"
                            "subcontrol-origin: padding;"
                          "}"

                          "QScrollBar::add-line:vertical {"
                            "width: 12px;"
                            "height: 12px;"
                            "background: transparent;"
                            "subcontrol-position: bottom;"
                            "subcontrol-origin: padding;"
                          "}"

                          "QScrollBar::add-line:vertical:hover {"
                            "width: 12px;"
                            "height: 12px;"
                            "border-radius: 6px;"
                            "background: rgba(255, 255, 255, 128);"
                            "subcontrol-position: bottom;"
                            "subcontrol-origin: padding;"
                          "}"

                          "QScrollBar::up-arrow:vertical {"
                            "width: 12px;"
                            "height: 12px;"
                            "border-image: url(:/images/upArrow.png);"
                          "}"

                          "QScrollBar::down-arrow:vertical {"
                            "width: 12px;"
                            "height: 12px;"
                            "border-image: url(:/images/downArrow.png);"
                          "}"

                          "QScrollBar::sub-page:vertical, QScrollBar::add-page:vertical {"
                            "background: transparent;"
                          "}");
}

QString ControlStyle::windowBackgroundStyle(const QString &filePath)
{
    return QStringLiteral("QWidget#Widget {"
                            "border-image: url(%1);"
                            "color: white"
                          "}").arg(filePath);
}

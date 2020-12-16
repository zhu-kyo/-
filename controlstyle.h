#ifndef CONTROLSTYLE_H
#define CONTROLSTYLE_H

class QString;

class ControlStyle
{
public:
    ControlStyle();

    static QString scrollBarStyle();
    static QString windowBackgroundStyle(const QString &filePath);
};

#endif // CONTROLSTYLE_H

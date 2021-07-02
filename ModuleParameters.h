#ifndef MODULEPARAMETERS_H
#define MODULEPARAMETERS_H

#include <QObject>

class ModuleParameters : public QObject
{
    Q_OBJECT
public:
    explicit ModuleParameters(QObject *parent = nullptr);

    bool defined;
    char m_Class[40];
    char m_Model[40];
    int m_BaseAddress;
signals:

};

#endif // MODULEPARAMETERS_H

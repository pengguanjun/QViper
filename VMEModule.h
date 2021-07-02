#ifndef VMEMODULE_H
#define VMEMODULE_H

#include "VMEInterface.h"
#include "VMEInterface_XXUSB.h"

class VMEModule : public QObject
{
public:
    enum MODELTYPE {NONE=0,MAXV,VME58,TIP114, VSC8, VSC16, SIS3820, IP470A};
    VMEModule(MODELTYPE moduleType);
    virtual ~VMEModule();

    bool defined;

    MODELTYPE m_ModelType;
    static QString *MODELTYPESTR[];
    static MODELTYPE GetModelType(QString& moduleName);

    QString m_ModuleNumber;
    QString m_Model;

    enum ADDRESSSPACE {A16=0, A24, A32};
    static QString * ADDRESSSPACESTR[];

    unsigned int m_BaseAddress;
    QString      m_BaseAddressStr;

    ADDRESSSPACE m_AddressSpace;
    QString      m_AddressSpaceStr;

    bool Init(QString& moduleNumber, QString& model, QString& baseaddress, QString& addressspace);

    VMEInterface * m_pVMEInterface;
    void SetVMEInterface(VMEInterface *pVMEInterface);

    virtual bool InitBoard() = 0;
    bool m_InitFailedFlag;
};

#endif // VMEMODULE_H

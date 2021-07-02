#include "VMEModule.h"

//NONE=0,MAXV,VME58,TIP114, VSC8, VSC16, SIS3820, IP470A

QString *VMEModule::ADDRESSSPACESTR[] = {new QString("A16"),
                                         new QString("A24"),
                                         new QString("A32")};

QString *VMEModule::MODELTYPESTR[] = {new QString("NONE"),
                                     new QString("MAXv"),
                                     new QString("VME58"),
                                     new QString("TIP114"),
                                     new QString("VSC8"),
                                     new QString("VSC16"),
                                     new QString("SIS3820"),
                                     new QString("IP470A")};

VMEModule::VMEModule(MODELTYPE type) : m_ModelType(type), m_pVMEInterface(NULL), m_InitFailedFlag(false)
{

}

VMEModule::~VMEModule()
{

}

bool VMEModule::Init(QString& moduleNumber, QString& model, QString& baseaddress, QString& addressspace)
{
    //m_ModuleNumber = QString::number(moduleNumber);
    m_ModuleNumber = moduleNumber;

    m_Model = model;
    bool opStatus;
    m_BaseAddress = baseaddress.toUInt(&opStatus, 16);
    if (!opStatus)
        return false;
    m_BaseAddressStr = "0x" + QString::number(m_BaseAddress, 16);

    m_AddressSpaceStr = addressspace;
    if (addressspace == "A16")
        m_AddressSpace = A16;
    else if (addressspace == "A24")
        m_AddressSpace = A24;
    else if (addressspace == "A32")
        m_AddressSpace = A32;
    else return false;

    if (model == MODELTYPESTR[0])
        m_ModelType = NONE;
    else if (model == MODELTYPESTR[1])
        m_ModelType = MAXV;
    else if (model == MODELTYPESTR[2])
        m_ModelType = VME58;
    else if (model == MODELTYPESTR[3])
        m_ModelType = VSC8;
    else
        return false;

    return true;
}

void VMEModule::SetVMEInterface(VMEInterface *pVMEInterface)
{
    this->m_pVMEInterface = pVMEInterface;
}


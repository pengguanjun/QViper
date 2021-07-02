#ifndef VMEINTERFACE_H
#define VMEINTERFACE_H

#include <QString>

/*
 * Address Modifiers
 * 0x29 = A16 non privileged access
 * 0x24 = A24 supervisory data access
 * 0x0d = A32 supervisory data access
 *
 */


class VMEInterface
{
public:
    enum ADDRESSMODIFIERCODE {A16=0x29, A24=0x3d, A32=0x0d};
    VMEInterface(ADDRESSMODIFIERCODE vmeAddressModifier);
    virtual ~VMEInterface();

    ADDRESSMODIFIERCODE m_AM;

    virtual bool Init() = 0;

    /*
     * AddressSpace = A16, A24, A32
     * Address      = address for transaction
     * *Data        = data to be transferred
     * NElems       = number of elements to be transferred
     * DataWidth    = width in bytes of an individual element (D08=1, D16=2, D32=4)
     */
    int vsleep(unsigned int usec);

    virtual int Read08 (long Address, unsigned char  *pData, int length) = 0;
    virtual int Read16 (long Address, unsigned short *pData, int length) = 0;
    virtual int Read32 (long Address, unsigned int   *pData, int length) = 0;
    virtual int Write08(long Address, unsigned char  *pData, int length) = 0;
    virtual int Write16(long Address, unsigned short *pData, int length) = 0;
    virtual int Write32(long Address, unsigned int   *pData, int length) = 0;

};

#endif // VMEINTERFACE_H

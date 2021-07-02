#ifndef VMEINTERFACE_XXUSB_H
#define VMEINTERFACE_XXUSB_H

#include <QObject>
#include <QMutex>
#include <vector>
#include "VMEInterface.h"
#include "libxxusb.h"

class VMEInterface_XXUSB : public VMEInterface
{
public:
    VMEInterface_XXUSB(ADDRESSMODIFIERCODE vmeAddressModifier);
    virtual ~VMEInterface_XXUSB();

    xxusb_device_type xxdev[10];
    bool Init() override;

    struct DEVICEHANDLE
    {
        xxusb_device_type    m_USBDevice;
        libusb_device        *usbDev;
        libusb_device_handle *m_Handle;
    };

    std::vector<DEVICEHANDLE *> m_VMEDevices;

    // Even xxusb library has been designed for multiple devices (that is why
    // there is usb_dev_handle concept) for our application we are using ONE USB interface
    static libusb_device_handle *m_Handle;


    union ShortAndByte
    {
        unsigned short  m_Short;
        unsigned char   m_Char[2];
    } SaB;

    bool SwapBytes(unsigned char *pData, int length);

    QMutex m_Mutex;

    int Read08 (long Address, unsigned char  *pData, int length) override;
    int Read16 (long Address, unsigned short *pData, int length) override;
    int Read32 (long Address, unsigned int   *pData, int length) override;
    int Write08(long Address, unsigned char  *pData, int length) override;
    int Write16(long Address, unsigned short *pData, int length) override;
    int Write32(long Address, unsigned int   *pData, int length) override;
};

#endif // VMEINTERFACE_XXUSB_H

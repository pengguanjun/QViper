#include "VMEInterface_XXUSB.h"
#include <QDebug>

libusb_device_handle *VMEInterface_XXUSB::m_Handle = NULL;

VMEInterface_XXUSB::VMEInterface_XXUSB(ADDRESSMODIFIERCODE vmeAddressModifier) : VMEInterface(vmeAddressModifier)
{

}

VMEInterface_XXUSB::~VMEInterface_XXUSB()
{
    for (unsigned int i = 0; i < m_VMEDevices.size(); i++)
    {
        delete m_VMEDevices[i];
    }

    m_VMEDevices.clear();
}

bool VMEInterface_XXUSB::Init() {
    if (m_Handle != NULL)
        return true;  // has been initialized

    int n = xxusb_devices_find(xxdev);
    //qDebug() << "-- m_ModuleNumber= " << n;

    for (int i=0; i < n; i++)
    {
        //qDebug() << "usbdevice:  " << xxdev[i].SerialString;
        libusb_device_handle *handle = xxusb_device_open(xxdev[i].usbdev);
        if (handle == NULL)
        {
            //QMessageBox::warning(NULL, QString("Cannot open device"), xxdev[i].SerialString, QMessageBox::Ok);
            return false;
        }
        else
        {
            DEVICEHANDLE *pdh = new DEVICEHANDLE();
            pdh->m_USBDevice = xxdev[i];
            pdh->m_Handle = handle;
            m_VMEDevices.push_back(pdh);
        }
    }
    if (n > 0)
    {
        m_Handle = m_VMEDevices[0]->m_Handle;
        long RevisionData;
        xxusb_register_read(m_Handle, XXUSB_FIRMWARE_REGISTER, &RevisionData);
        return true;
    }
    return false;
}

bool VMEInterface_XXUSB::SwapBytes(unsigned char *pData, int length)
{
    if ((length & 1) != 0)
        return false;

    for (int i=0; i<(length >> 1); i++)
    {
        char t = pData[0];
        pData[0] = pData[1];
        pData[1] = t;
        pData += 2;
    }
    return true;
}

int VMEInterface_XXUSB::Read08 (long Address, unsigned char  *pData, int length) {
    int nread = 0;
    int bytesToRead = length;

    // read 1 byte if readIndex is alignment is odd
    if (Address & 1) {
        Read16(Address-1, &SaB.m_Short, 1);
        SwapBytes(&SaB.m_Char[0], 2);
        *pData = SaB.m_Char[1];
        bytesToRead--;
        pData++;
        Address++;
        nread++;
    }

    int shortToRead = bytesToRead >> 1;
    if (shortToRead > 0) {
        Read16(Address, (unsigned short *)pData, shortToRead);
        SwapBytes(pData, shortToRead << 1);
        bytesToRead -= (shortToRead<<1);
        Address += (shortToRead<<1);
        pData += (shortToRead<<1);
        nread += (shortToRead<<1);
    }

    if (bytesToRead > 0)
    {
        // one more byte
        Read16(Address, &SaB.m_Short, 1);
        SwapBytes(&SaB.m_Char[0], 2);
        *pData = SaB.m_Char[0];
        nread++;
    }

    return nread;
}

// Return number of data items read
int VMEInterface_XXUSB::Read16 (long Address, unsigned short *pData, int length) {
    int nread;

    QMutexLocker lg(&m_Mutex);

    if (m_Handle == NULL) return -1;

    long dataLong;
    nread = 0;
    for (int i=0; i<length; i++) {
        if (VME_read_16(m_Handle, m_AM, Address, &dataLong) < 0) {
            // todo process exception
            break;
        }
        Address += 2;

        *pData = *(unsigned short *)&dataLong;
        pData++;
        nread++;
    }
    return nread;
}

int VMEInterface_XXUSB::Read32 (long Address, unsigned int   *pData, int length) {
    int nread;
    long dataLong;

    QMutexLocker lg(&m_Mutex);
    if (m_Handle == NULL) return -1;

    nread = 0;
    for (int i=0; i<length; i++) {
        if (VME_read_32(m_Handle, m_AM, Address, &dataLong) < 0) {
                // todo process exception
                *pData = 0;
                return false;
         }
        Address += 4;

        *pData = *(unsigned int *)&dataLong;
        pData++;
        nread++;
    }
    return nread;
}

int VMEInterface_XXUSB::Write08(long Address, unsigned char  *pData, int length) {
    int nwritten=0;
    int bytesToWrite = length;

    // read 1 byte is not aligned on short or longword
    if (Address & 1) {
        Read16(Address-1, &SaB.m_Short, 1);
        SwapBytes(&SaB.m_Char[0], 2);
        SaB.m_Char[1] = *pData;
        SwapBytes(&SaB.m_Char[0], 2);
        Write16(Address-1, &SaB.m_Short, 1);
        bytesToWrite--;
        pData++;
        Address++;
    }

    int shortToWrite = bytesToWrite >> 1;
    if (shortToWrite > 0)
    {
        SwapBytes(pData, bytesToWrite);
        Write16(Address, (unsigned short *)pData, shortToWrite);
        bytesToWrite -= (shortToWrite << 1);
        pData += (shortToWrite << 1);
        Address += (shortToWrite<<1);
    }

    if (bytesToWrite > 0)
    {
        // one more byte
        Read16(Address, &SaB.m_Short, 1);
        SwapBytes(&SaB.m_Char[0], 2);
        SaB.m_Char[0] = *pData;
        SwapBytes(&SaB.m_Char[0], 2);;
        Write16(Address, &SaB.m_Short, 1);
    }

    return nwritten;
}


int VMEInterface_XXUSB::Write16(long Address, unsigned short *pData, int length) {
    int nwritten;
    QMutexLocker lg(&m_Mutex);

    if (m_Handle == NULL) return -1;

    nwritten = 0;
    for (int i=0; i<length; i++) {
        long dataLong = ((unsigned short *)pData)[i];
        if (VME_write_16(m_Handle, m_AM, Address, dataLong) < 0) {
                // todo process exception
            break;
        }
        Address += 2;
        pData++;
        nwritten++;
    }

    return nwritten;
}

int VMEInterface_XXUSB::Write32(long Address, unsigned int   *pData, int length) {
    int nwritten;

    QMutexLocker lg(&m_Mutex);

    if (m_Handle == NULL) return -1;

    nwritten=0;
    for (int i=0; i<length; i++) {
        if (VME_write_32(m_Handle, m_AM, Address, *pData) < 0) {
                // todo process exception
                return false;
        }
        Address += 4;
        pData++;
        nwritten++;
    }

    return nwritten;
}

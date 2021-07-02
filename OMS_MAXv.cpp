#include "OMS_MAXv.h"
#include <QDebug>
#include <QThread>

#define AXIS_OFFSET 0x0004;


OMS_MAXv::OMS_MAXv() : VMEMotor()
{
    int i;

    // Populate internal state variables
    for (i=0;i<8;i++) {
        dscale[i] = 1.0;
        MotorStepsPerRevolution[i] = 25000;
        EncoderStepsPerRevolution[i] = 4096;
        topvelocity[i]    = 60000;
        basevelocity[i]   = 25000;
        deadband[i]       = 5;
        moving[i]         = false;
        use_encoder[i]    = false;
        axis_enabled[i]   = false;
        limits_enabled[i] = false;
        fault[i]          = false;
    }
}

OMS_MAXv::~OMS_MAXv()
{

}

void OMS_MAXv::pbin(char buf[], long val, int len)
{
    int j;
    char ch, *pbuf;
    pbuf = &buf[0];
    for(j=len;j>0;j--){
        if (!(j%4)) { *pbuf++ = ' '; }
        ch = ((val & (0x00000001 << (j-1))) ? '1' : '0');
        *pbuf++ = ch;

    }
    *pbuf++ = 0; // Terminate
}

int OMS_MAXv::SendCommand(char * command)
{
    unsigned int addr, status;
    unsigned int InsertIdx, ProcessIdx;
    int FreeBytes, len, CharsSent;
    unsigned short sval;
    char *pCom, ch;

    addr = m_BaseAddress + COMMAND_INSERT;
    m_pVMEInterface->Read32(addr,&InsertIdx,1);

    addr = m_BaseAddress + COMMAND_PROC;
    m_pVMEInterface->Read32(addr,&ProcessIdx,1);

    CharsSent = 0;


    len = strlen(command); // Assume null terminated
    pCom = command;

    if (len > 0) {
        if (InsertIdx == ProcessIdx) {
            FreeBytes = COMMAND_BUF_SIZE - 1;
        } else if (InsertIdx > ProcessIdx) {
            FreeBytes = COMMAND_BUF_SIZE - (InsertIdx - ProcessIdx) - 1;
        } else {
            FreeBytes = (ProcessIdx - InsertIdx) - 1;
        }

        if (FreeBytes >= len) {
            if (strlen(pCom)%2) {
                addr = m_BaseAddress + COMMAND_BUF + InsertIdx;
                ch = *pCom++;
                sval = (' ' << 8) + ch;
                m_pVMEInterface->Write16(addr,&sval,1);
                InsertIdx+=2;
                CharsSent++;
                if (InsertIdx >= COMMAND_BUF_SIZE) InsertIdx = 0;
            }
            while (*pCom != (char) 0) {
                addr = m_BaseAddress + COMMAND_BUF + InsertIdx;
                ch = *pCom++;
                sval = (((short)ch) << 8);
                ch = *pCom++;
                sval += ch;
                m_pVMEInterface->Write16(addr,&sval,1);
                CharsSent+=2;
                InsertIdx+=2;
                if (InsertIdx >= COMMAND_BUF_SIZE) InsertIdx = 0;
            }

            // Store updated insert pointer
            addr = m_BaseAddress + COMMAND_INSERT;
            m_pVMEInterface->Write32(addr,&InsertIdx,1);
        }
    }
    //qDebug() << "SendCommand " << QString(command) << " -- CharsSent = " << CharsSent;

    QThread::usleep(50);
    m_pVMEInterface->Read32(m_BaseAddress+MAXV_STAT1_F,&status,1);
    if (status & MAXV_COMMAND_ERROR) {
        qDebug() << "MAXv Command Error: offending command = " + QString(command);

        // reset the error
        status = MAXV_COMMAND_ERROR;
        m_pVMEInterface->Write32(m_BaseAddress+MAXV_STAT1_F,&status,1);
        // Eat it and "fail" silently
    }
    return CharsSent;
}

int OMS_MAXv::SendAndGetString(char * command, char *pResponse, int maxLen)
{
    unsigned int addr;
    int Length,ResponseLength;
    int TimeOut, CharsSent, ComLength;
    unsigned int ProcessIdx, msgsem, status;
    unsigned int ProcIdx,InsertIdx;
    unsigned short ResponseShort;
    char * Response, ResponseChar;

    Response = pResponse;
    *Response = (char) 0; /* If response fails, return null string */
    ResponseLength = 0;
    ComLength = strlen(command);

    // Clear message semaphore
    addr = m_BaseAddress + MAXV_MSG_SEM;
    msgsem = 0;
    if (m_pVMEInterface->Write32(addr,&msgsem,1) < 0) return -1;

    // Need to test for success of this step
    CharsSent = SendCommand(command);
    // If the command string was sent successfully
    if (CharsSent >= ComLength) {
        QThread::usleep(50);
        TimeOut = RESPONSE_RETRY_LIM;
        do {
            status = 0;
            addr = m_BaseAddress + MAXV_STAT1_F;
            m_pVMEInterface->Read32(addr,&status,1);
            if (status & MAXV_COMMAND_ERROR) {
                qDebug() << "MAXv Command Error: offending command = " + QString(command);

                // reset the error
                status = MAXV_COMMAND_ERROR;
                m_pVMEInterface->Write32(addr,&status,1);
                return -100;
            }
            QThread::usleep(50);
        } while((--TimeOut >0) && ((status & MAXV_RESPONSE_AVAILABLE)==0));
        if (TimeOut > 0) {
            addr = m_BaseAddress + MAXV_MSG_SEM;
            m_pVMEInterface->Read32(addr,&msgsem,1);

            if ((msgsem & 0xff)!=0) {
                addr = m_BaseAddress + RESPONSE_INSERT;
                m_pVMEInterface->Read32(addr,&InsertIdx,1);

                addr = m_BaseAddress + RESPONSE_PROC;
                m_pVMEInterface->Read32(addr,&ProcIdx,1);

                //qDebug() << "SendAndGetString: ProcIdx = " << ProcIdx << " InsertIdx = " << InsertIdx << " Length = " << Length;

                ProcessIdx = (msgsem >> 8) & 0xFFF;
                Length = (msgsem >> 20) & 0xFFF;
                //qDebug() << "SendAndGetString: ProcessIdx = " << ProcessIdx << " Length = " << Length;
                while ((Length-=2) > 0) {
                    addr = m_BaseAddress + RESPONSE_BUF + ProcessIdx;
                    m_pVMEInterface->Read16(addr,&ResponseShort,1);

                    if ((ProcessIdx+=2) >= RESPONSE_BUF_SIZE) ProcessIdx = 0;

                    ResponseChar = (char) (((0xff00) & ResponseShort) >> 8);
                    if (ResponseChar == '\n') ResponseChar = '\0';
                    *Response++ = ResponseChar;
                    ResponseLength++;
                    ResponseChar = (char) (0x00ff & ResponseShort);
                    if (ResponseChar == '\n') ResponseChar = '\0';
                    *Response++ = ResponseChar;
                    ResponseLength++;
                }
                addr = m_BaseAddress + RESPONSE_PROC;
                m_pVMEInterface->Write32(addr,&ProcessIdx,1);
            }
        }
    }
    pResponse[ResponseLength] = (char) 0;
    //qDebug() << "Received : " + QString(pResponse);
    //qDebug() << "SendAndGetString: ResponseLength = " << ResponseLength;
    return ResponseLength;
}

void OMS_MAXv::ClearCommandError()
{
    if (m_pVMEInterface == NULL)
        return;
    unsigned int val = 1<<24;
    this->m_pVMEInterface->Write32(m_BaseAddress + MAXV_STAT1_F, &val, 1);
}
void OMS_MAXv::ClearTextResponseAvailable()
{
    if (m_pVMEInterface == NULL)
        return;
    unsigned val = 1<<25;
    m_pVMEInterface->Write32(m_BaseAddress + MAXV_STAT1_F, &val, 1);
}
void OMS_MAXv::ClearRequestedDataAvailable()
{
    if (m_pVMEInterface == NULL)
        return;
    unsigned int val = 1<<26;
    m_pVMEInterface->Write32(m_BaseAddress + MAXV_STAT1_F, &val, 1);
}
void OMS_MAXv::ClearMessageSemaphore()
{
    if (m_pVMEInterface == NULL)
        return;
    unsigned int val = 0;
    m_pVMEInterface->Write32(m_BaseAddress + MAXV_MSG_SEM, &val, 1); // clear Message semaphore
}

quint32 OMS_MAXv::GetStatusWord1()
{
    if (m_pVMEInterface == NULL)
        return 0xFFFFFFFF;

    unsigned int ret;
    m_pVMEInterface->Read32(m_BaseAddress+MAXV_STAT1_F, &ret, 1);
    return ret;
}

quint32 OMS_MAXv::GetStatusWord2()
{
    if (m_pVMEInterface == NULL)
        return 0xFFFFFFFF;

    unsigned int ret;
    m_pVMEInterface->Read32(m_BaseAddress+MAXV_STAT2_F, &ret, 1);
    return ret;
}

bool OMS_MAXv::GetAxisLimitStatus(int index, bool * onposlimit, bool * onneglimit)
{
    int poslim, neglim;
    bool limit_high = false; // Short circuit for now

    if (this->m_pVMEInterface == NULL) return false;

    unsigned int ret;
    m_pVMEInterface->Read32(m_BaseAddress+MAXV_LIMIT_STAT, &ret, 1);

    if (limit_high) {
        neglim = (ret & (0x01 << index)) ? 1 : 0;
        poslim = (ret & (0x01 << (index+8))) ? 1 : 0;
    } else {
        neglim = (ret & (0x01 << index)) ? 0 : 1;
        poslim = (ret & (0x01 << (index+8))) ? 0 : 1;
    }

    *onposlimit = (poslim) ? true : false;
    *onneglimit = (neglim) ? true : false;

    return true;
}

bool OMS_MAXv::GetAxisEnableStatus(int index)
{
    unsigned int ret;
    int i;
    bool enabled;

    if (this->m_pVMEInterface == NULL) return false;
    m_pVMEInterface->Read32(m_BaseAddress+MAXV_GPIO_STAT, &ret, 1);

    for (i=0;i<8;i++) {
        axis_enabled[i] = (ret & (0x1 << (i+8))) ? true : false;
    }
    enabled = (ret & (0x1 << (index+8))) ? true : false;
    return enabled;
}

bool OMS_MAXv::GetAxisFaultStatus(int index)
{
    unsigned int ret;
    int i;
    bool faulted;

    if (this->m_pVMEInterface == NULL) return false;
    m_pVMEInterface->Read32(m_BaseAddress+MAXV_GPIO_STAT, &ret, 1);

    for (i=0;i<8;i++) {
        fault[i] = (ret & (0x1 << i)) ? false : true;

    }
    faulted = (ret & (0x1 << index)) ? false : true;
    return faulted;
}


quint32 OMS_MAXv::GetLimitStatus()
{
    return 0;
}

quint32 OMS_MAXv::GetHomeStatus()
{
    if (this->m_pVMEInterface == NULL)
        return 0xFFFFFFFF;

    unsigned int ret;
    m_pVMEInterface->Read32(m_BaseAddress+MAXV_HOME_STAT, &ret, 1);
    return ret;
}

quint32 OMS_MAXv::GetGPIOStatus()
{
    if (this->m_pVMEInterface == NULL)
        return 0xFFFFFFFF;

    unsigned int ret;
    m_pVMEInterface->Read32(m_BaseAddress+MAXV_GPIO_STAT, &ret, 1);
    return ret;
}

bool OMS_MAXv::ResetDoneFlag(int index)
{
    unsigned int val;

    val = 0x1 << index;
    if (m_pVMEInterface->Write32(m_BaseAddress+MAXV_STAT1_F,&val,1)!=1) return false;

    return true;
}

bool OMS_MAXv::ResetOvertravelFlag(int index)
{
    unsigned int val;

    val = 0x100 << index;
    if (m_pVMEInterface->Write32(m_BaseAddress+MAXV_STAT1_F,&val,1)!=1) return false;

    return true;
}

bool OMS_MAXv::ResetSlipFlag(int index)
{
    unsigned int val;

    val = 0x10000 << index;
    if (m_pVMEInterface->Write32(m_BaseAddress+MAXV_STAT1_F,&val,1)!=1) return false;

    return true;
}


bool OMS_MAXv::InitBoard()
{
    char responsebuf[128];
    int lresponse;
    if (m_pVMEInterface == NULL)
        return false;

    unsigned int val;
    bool bSuccess = true;
    char wy[3];
    char ioconf[10];
    strcpy(wy,"WY");
    strcpy(ioconf,"BDFF00; ");

    //qDebug() << "OMS_MAXv::InitBoard " + m_BaseAddressStr;
    val = 0x00000067;
    if (!m_pVMEInterface->Write32(m_BaseAddress + MAXV_IACK_IDV, &val, 1))
    {
        bSuccess = false;
        goto Mfailure;
    }

    val = 0xffffffff;
    if (!m_pVMEInterface->Write32(m_BaseAddress + MAXV_STAT2_F, &val, 1))
    {
        bSuccess = false;
        goto Mfailure;
    }
    val = 0x0;
    if (!m_pVMEInterface->Write32(m_BaseAddress + MAXV_STAT2_F, &val, 1))
    {
        bSuccess = false;
        goto Mfailure;
    }
    val = 0x0;
    if (!m_pVMEInterface->Write32(m_BaseAddress + MAXV_STAT1_IER, &val, 1))
    {
        bSuccess = false;
        goto Mfailure;
    }

    val = 0x0;
    if (!m_pVMEInterface->Write32(m_BaseAddress + MAXV_STAT2_IER, &val, 1))
    {
        bSuccess = false;
        goto Mfailure;
    }
    val = 0x0;
    if(!m_pVMEInterface->Write32(m_BaseAddress + MAXV_MSG_SEM, &val, 1))
    {
        bSuccess = false;
        goto Mfailure;
    }

    val = 0xffffffff;
    if (!m_pVMEInterface->Write32(m_BaseAddress + MAXV_STAT1_F, &val, 1))
    {
        bSuccess = false;
        goto Mfailure;
    }

Mfailure:
    if(!bSuccess)
    {
        QString msg = "MAXv Initialization failed! " + m_ModuleNumber + "(" + m_BaseAddressStr + ")";
        //Log::getInstance().VLog(&msg);
        //pMainWindow->SetStatusMessage(&msg);
        qDebug() << msg;
        m_InitFailedFlag = true;
        return false;
    }

    // read
    m_pVMEInterface->Read32(m_BaseAddress + MAXV_FIRM_STAT, &val, 1);
    if ((val & 0x00000007) != 0x00000004) {
        QString msg = "MAXv Application program not running! " + m_ModuleNumber + " (" + m_BaseAddressStr + ")";
        //Log::getInstance().VLog(&msg);
        //pMainWindow->SetStatusMessage(&msg);
        qDebug() << msg;
        m_InitFailedFlag = true;
        return false;
    }

    lresponse = sizeof(responsebuf);
    SendAndGetString(wy,responsebuf,lresponse);

    SendCommand(ioconf);

    return true;
}

char OMS_MAXv::AxisChar(int index)
{
    switch(index) {
    case 0: return 'X'; break;
    case 1: return 'Y'; break;
    case 2: return 'Z'; break;
    case 3: return 'T'; break;
    case 4: return 'U'; break;
    case 5: return 'V'; break;
    case 6: return 'R'; break;
    case 7: return 'S'; break;
    default:
        return 'A'; // All
    }
}



bool OMS_MAXv::Configure(int index, MotorParameters * mparm)
{
    char cmd[80];
    char tmpbuf[80];
    char ax;
    int res;
    unsigned int status;
    int MotorRawPosition, EncoderRawPosition;
    bool result;

    if ((index < 0) || (index > 7)) return false;
    ax = AxisChar(index);

    GetRawPositions(index,&MotorRawPosition, &EncoderRawPosition);

    dscale[index] = mparm->dscale;
    use_encoder[index]  = mparm->use_encoder;
    topvelocity[index]  = mparm->vscale * mparm->MotorStepsPerRevolution;
    basevelocity[index] = mparm->bscale * mparm->MotorStepsPerRevolution;
    deadband[index]     = mparm->deadband;
    MotorStepsPerRevolution[index] = mparm->MotorStepsPerRevolution;
    EncoderStepsPerRevolution[index] = mparm->EncoderStepsPerRevolution;
    limits_enabled[index] = mparm->limits_enabled;
    axis_enabled[index] = mparm->axis_enabled;

    if (mparm->is_servo) {
        sprintf(cmd,"A%c; PSM; SVB1; LP%d; KP%.1f; KI%.1f; KD%.1f; KA%.1f; CL1; ",
                ax,
                mparm->EncoderRawPosition,
                mparm->kp,mparm->ki,
                mparm->kd,mparm->ka);
        res = mparm->EncoderStepsPerRevolution;
    } else {
        if (mparm->use_encoder) {
            sprintf(cmd,"A%c; PSE; ER%d,%d; LP%d; ",
                    ax,
                    mparm->EncoderStepsPerRevolution,
                    mparm->MotorStepsPerRevolution,
                    mparm->MotorRawPosition); //<----------------------------RawPosition
            res = mparm->EncoderStepsPerRevolution;
        } else {
            sprintf(cmd,"A%c; PSO; LP%d; ",
                    ax,
                    mparm->MotorRawPosition);
            res = mparm->MotorStepsPerRevolution;
        }
    }

    // Set limit polarity
    if (mparm->limit_high) {
        sprintf(tmpbuf,"LTH; ");
    } else {
        sprintf(tmpbuf, "LTL; ");
    }

    strcat(cmd,tmpbuf);
    SendCommand(cmd);

    mparm->acceleration = mparm->ascale * res;
    mparm->basevelocity = mparm->bscale * res;
    mparm->topvelocity  = mparm->vscale * res;
    if (mparm->basevelocity >= mparm->topvelocity)
        mparm->basevelocity = mparm->topvelocity-1;



    // Set slip tolerance
    if (mparm->stalldetection) {
        sprintf(cmd,"A%c; ES%d; TN; ",ax,mparm->deadband);
    } else {
        sprintf(cmd,"A%c; TF; ",ax);
    }
    //SendCommand(cmd);
    //QThread::usleep(50);

    // Dump acceleration and velocity
    sprintf(cmd,"A%c; AC%d; VL%d; VB%d; ",
                ax,
                mparm->acceleration,
                mparm->topvelocity,
                mparm->basevelocity);
    SendCommand(cmd);
    //QThread::usleep(50);

    // Limits
    if (mparm->limits_enabled) {
        sprintf(cmd,"A%c; LMH; ",ax);
    } else {
        sprintf(cmd,"A%c; LMF; ",ax);
    }
    SendCommand(cmd);
    //QThread::usleep(50);

    // Position Maintenance
    if (mparm->use_encoder) {
        if (mparm->positionmaintenance) {
            int basespeed = (mparm->basevelocity) ? mparm->basevelocity : 2000;
            sprintf(cmd,"A%c; HV%d HD%d HG%d CL1; ",ax,basespeed,
                    mparm->deadband,2*basespeed);
        } else {
            sprintf(cmd,"A%c; CL0; ",ax);
        }
        SendCommand(cmd);
    }
    //QThread::usleep(50);

    res = 8 + index;
    if (!(mparm->axis_enabled ^ mparm->enable_high)) {
        sprintf(cmd,"BH%d; ", res);
    } else {
        sprintf(cmd,"BL%d; ", res);
    }
    SendCommand(cmd);
    //QThread::usleep(50);

    m_pVMEInterface->Read32(m_BaseAddress+MAXV_STAT1_F,&status,1);
    if (status & MAXV_COMMAND_ERROR) {
        qDebug() << "MAXv Command Error";

        // reset the error
        status = MAXV_COMMAND_ERROR;
        m_pVMEInterface->Write32(m_BaseAddress+MAXV_STAT1_F,&status,1);
        // Eat it and "fail" silently
    }

    return true;
}

bool OMS_MAXv::GetRawPositions(int index, int *MotorRawPosition, int *EncoderRawPosition)
{
    unsigned int addr;
    unsigned int dpos, epos;

    if ((index < 0) || (index > 7)) return false;

    addr = this->m_BaseAddress + CMD_POS_UPD + index*AXIS_OFFSET;
    m_pVMEInterface->Read32(addr,&dpos,1);

    addr = this->m_BaseAddress + ENC_POS_UPD + index*AXIS_OFFSET;
    m_pVMEInterface->Read32(addr,&epos,1);

    //qDebug() << "dpos[" << index << "] = " << dpos << " epos = " << epos;

    *MotorRawPosition = dpos;
    *EncoderRawPosition = epos;
    return true;
}

bool OMS_MAXv::GetPosition(int index, double * position)
{
    int MotorRawPosition, EncoderRawPosition;
    if ((index <0) || (index > 7)) return false;
    if (GetRawPositions(index,&MotorRawPosition,&EncoderRawPosition)) {
        if(use_encoder[index]) {
            *position = (((double) EncoderRawPosition)/
                         EncoderStepsPerRevolution[index]) *
                    dscale[index];
        } else {
            *position = (((double) MotorRawPosition)/
                         MotorStepsPerRevolution[index]) *
                    dscale[index];
        }
        return true; // Success
    }
    return false;
}

bool OMS_MAXv::SetPosition(int index, double position) {
    char ax;
    int pulses;
    char cmd[80];
    bool is_servo;

    is_servo = false; // Short circuit for now

    if ((index <0) || (index > 7)) return false;
    ax = AxisChar(index);

    if (is_servo) {
        pulses = (int) ((position/dscale[index]) *
                        EncoderStepsPerRevolution[index]);
        sprintf(cmd,"A%c; LP%d; CL1; ",ax,pulses);
    } else if (use_encoder[index]) {
        pulses = (int) ((position/dscale[index]) *
                        EncoderStepsPerRevolution[index]);
        sprintf(cmd,"A%c; ER%d,%d; LP%d; ",ax,
                EncoderStepsPerRevolution[index],
                MotorStepsPerRevolution[index],
                pulses);
    } else {
        pulses = (int) ((position/dscale[index]) *
                        MotorStepsPerRevolution[index]);
        sprintf(cmd,"A%c; LP%d; ",ax,pulses);
    }
    SendCommand(cmd);

    return true;
}

bool OMS_MAXv::EnableAxis(int index, bool enable)
{
    char cmd[80];
    int res;
    if ((index < 0) || (index > 7)) return false;
    bool enable_high = true; // Short circuit for now

    res = 8 + index;
    if (!(enable ^ enable_high)) {
        sprintf(cmd,"BH%d; ",res);
    } else {
        sprintf(cmd,"BL%d; ",res);
    }
    if (SendCommand(cmd)) {
        axis_enabled[index] = enable;
        return true;
    }
    return false;
}

bool OMS_MAXv::EnableLimits(int index, bool enable)
{
    char cmd[80];
    char ax;
    if ((index < 0) || (index > 7)) return false;
    ax = AxisChar(index);

    if (enable) {
        sprintf(cmd,"A%c; LMN; ",ax);
    } else {
        sprintf(cmd,"A%c; LMF; ",ax);
    }

    if (SendCommand(cmd)) {
        limits_enabled[index] = enable;
        return true;
    }
    return false;
}

/*
 * Motion status - we have to maintain our own motion status
 *                 1) Set a moving[] flag if we initiate motion
 *                 2) Check the DONE register periodically to see when to clear
 */
bool OMS_MAXv::IsMoving(int index) {
    unsigned int ival;
    int i, done[8], overtravel[8], slip[8];

    m_pVMEInterface->Read32(m_BaseAddress+MAXV_STAT1_F,&ival,1);
    //pbin(dbgbuf,ival,32);
    //qDebug() << "OMS_MAXv::IsMoving MAXV_STAT1_F = " << QString(dbgbuf);
    for (i=0;i<8;i++) {
        done[i]       = (ival & (0x000001 << i)) ? 1 : 0;
        overtravel[i] = (ival & (0x000100 << i)) ? 1 : 0;
        slip[i]       = (ival & (0x010000 << i)) ? 1 : 0;

        if (moving[i]) {
            if (done[i]) {
                moving[i] = false;
            } else if (overtravel[i]) {
                // "DONE NOT SET; OVERTRAVEL SET [%d]",i
                moving[i] = false;
            } else if (slip[i]) {
                // "DONE NOT SET; SLIP SET [%d]",i
                // could we keep moving in spite of our directive to stop motion?
                moving[i] = false;
            }
        }
    }
    if ((index < 0) || (index > 7)) return false;
    return moving[index];
}

bool OMS_MAXv::Stop(int index)
{
    char cmd[80];
    char ax;

    if ((index < 0) || (index > 7)) {
        strcpy(cmd,"AA; SD; ID; ");
    } else {
        ax = AxisChar(index);
        sprintf(cmd,"A%c; SD; ID; ",ax);
    }
    SendCommand(cmd);
    return true;
}

bool OMS_MAXv::GoToDestination(int index, double destination)
{
    char cmd[80], tmp[80];
    char ax;
    int  res, pulses, relpulses,CharsSent, cmdlen;
    int MotorRawPosition, EncoderRawPosition;
    bool is_servo;
    int direction;
    bool stalldetection, positionmaintenance;

    is_servo = false; // Short circuit for now
    stalldetection = false;
    positionmaintenance = false;

    if ((index < 0) || (index > 7)) return false;
    ax = AxisChar(index);

    res = (use_encoder[index] || is_servo) ?
                EncoderStepsPerRevolution[index] :
                MotorStepsPerRevolution[index];
    pulses = (int)((destination * res)/dscale[index]);

    GetRawPositions(index,&MotorRawPosition, &EncoderRawPosition);
    relpulses = (use_encoder[index]) ?
                (pulses - EncoderRawPosition) :
                (pulses - MotorRawPosition);

    direction = (relpulses > 0) ? 1 : 0;

    sprintf(cmd,"A%c; IC; VL%d; ",ax,topvelocity[index]);

    if (is_servo) {
        strcpy(tmp,"CL1; ");
    } else if (use_encoder[index]) {
        int basevel = (basevelocity[index]) ? basevelocity[index] : 2000;
        if (positionmaintenance) {
            sprintf(tmp,"HV%d; HD%d; HG%d; CL1; ",
                    basevel,
                    deadband[index],
                    2*basevel);
            strcat(cmd,tmp);
        } else {
            strcat(cmd,"CL0; ");
        }

        if (stalldetection) {
            // Set slip tolerance (ES), interrupt on stall, enable stall detection
            sprintf(tmp,"ES%d; TN; ",deadband[index]);
            strcat(cmd,tmp);
        }
    }
    ResetDoneFlag(index);
    ResetOvertravelFlag(index);
    ResetSlipFlag(index);
    sprintf(tmp,"MA%d; GN; ",pulses);
    strcat(cmd,tmp);
    cmdlen = strlen(cmd);

    CharsSent = SendCommand(cmd);
    if (CharsSent < cmdlen) return false; // Need to check for success of command
    /*
    m_pVMEInterface->Read32(m_BaseAddress+MAXV_STAT1_F,&status,1);
    if (status & MAXV_COMMAND_ERROR) {
        qDebug() << "MAXv Command Error: offending command = " + QString(cmd);

        // reset the error
        status = MAXV_COMMAND_ERROR;
        m_pVMEInterface->Write32(m_BaseAddress+MAXV_STAT1_F,&status,1);
        return false;
    }
    */

    moving[index] = true;
    return true;
}

bool OMS_MAXv::GoHome(int index, bool movepositive)
{
    return false;
}

bool OMS_MAXv::Oscillate(int index, double center, double halfamp)
{
    char cmd[120];
    char ax;
    double lo, hi;
    int res, lopulse, hipulse, speed, cmdlen, CharsSent;

    if ((index < 0) || (index > 7)) return false;
    ax = AxisChar(index);

    res = (use_encoder[index]) ? EncoderStepsPerRevolution[index] : MotorStepsPerRevolution[index];

    //speed = MotorParms[index]->topvelocity;
    speed = MotorStepsPerRevolution[index]; // This is wrong but we'll go with it for now
    lo = center - halfamp;
    lopulse = (int) ((lo * res)/dscale[index]);
    hi = center + halfamp;
    hipulse = (int) ((hi * res)/dscale[index]);

    sprintf(cmd,"A%c; IC; VL%d; WH; MA%d; GO; MA%d; GO; WG; ",
            ax, speed,lopulse,hipulse);
    cmdlen = strlen(cmd);
    ResetDoneFlag(index);
    ResetOvertravelFlag(index);
    ResetSlipFlag(index);

    CharsSent = SendCommand(cmd);
    if (CharsSent < cmdlen) return false;
    moving[index] = true;

    return true;
}

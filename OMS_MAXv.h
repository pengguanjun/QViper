#ifndef OMS_MAXV_H
#define OMS_MAXV_H

#include "VMEMotor.h"
#include "MotorParameters.h"


class OMS_MAXv : public VMEMotor
{
public:
    OMS_MAXv();
    virtual ~OMS_MAXv();

    //volatile MotorParameters * MotorParms[8];

    bool InitBoard();


    bool Configure(int index, MotorParameters * mparm);
    bool GetPosition(int index, double * position);
    bool SetPosition(int index, double position);
    bool GoToDestination(int index, double destination);
    bool GoHome(int index, bool movepositive);
    bool Oscillate(int index, double center, double halfamp);
    bool EnableAxis(int index, bool enable);
    bool EnableLimits(int index, bool enable);
    bool IsMoving(int index);
    bool Stop(int index);
    bool GetAxisLimitStatus(int index, bool *onposlimit, bool *onneglimit);
    bool GetAxisEnableStatus(int index);
    bool GetAxisFaultStatus(int index);

    //virtual void TimerProcessFunc();

    //MAXVCommands *m_pCommandInterface;



    void ClearCommandError();
    void ClearTextResponseAvailable();
    void ClearRequestedDataAvailable();
    void ClearMessageSemaphore();

    quint32 GetStatusWord1();
    quint32 GetStatusWord2();
    quint32 GetLimitStatus();
    quint32 GetHomeStatus();
    quint32 GetGPIOStatus();
    bool SetDoneFlags();
    bool ResetDoneFlag(int index);
    bool ResetOvertravelFlag(int index);
    bool ResetSlipFlag(int index);

    bool m_CommandErrorFlag;
    bool m_TextResponseAvailableFlag;

    int m_UIUpdatePeriod;  // VMEStatusTimer::m_UIPeriod / VMEStatusTimer::m_TimerPeriod
    int m_CurrentUITime;

    char m_ResponseBuffer[1024];

private:
    QString *WYCommand(); // WHO ARE YOU
    virtual int SendCommand(char * command);
    virtual int SendAndGetString(char * command, char *pResponse, int maxLen);
    char AxisChar(int index);
    bool GetRawPositions(int index, int *MotorRawPosition, int *EncoderRawPosition);

    char dbgbuf[64];
    void pbin(char buf[],long val, int len);

    double dscale[8];
    int topvelocity[8];
    int basevelocity[8];
    int deadband[8];
    int MotorStepsPerRevolution[8];
    int EncoderStepsPerRevolution[8];
    bool moving[8];
    bool use_encoder[8];
    bool axis_enabled[8];
    bool limits_enabled[8];
    bool fault[8];

    quint32 m_StatusWord1Flag;
    quint32 m_StatusWord2Flag;
    quint32 m_LimitSwitchStatus;
    quint32 m_HomeSwitchStatus;
    quint32 m_GPIOStatus;       // fault signals

};

/* Data transfer buffers */
#define COMMAND_INSERT  0x00f0
#define COMMAND_PROC    0x00f4
#define RESPONSE_INSERT 0x00f8
#define RESPONSE_PROC   0x00fc

#define COMMAND_BUF     0x0100
#define RESPONSE_BUF    0x0500
#define UTILITY_BUF     0x0900

#define COMMAND_BUF_SIZE  1024
#define RESPONSE_BUF_SIZE 1024
#define RESPONSE_RETRY_LIM  100
#define WAITUSEC            30

/* Register offsets */
#define MAXV_LIMIT_STAT 0x0040
#define MAXV_HOME_STAT  0x0044
#define MAXV_FIRM_STAT  0x0048
#define MAXV_DC_MBOX    0x004c
#define MAXV_PC_MBOX    0x0050 /* Position request mailbox */
#define MAXV_MSG_SEM    0x0094 /* Message semaphore */
#define MAXV_GPIO_STAT  0x009c

#define MAXV_STAT1_F    0x0fc0
#define MAXV_STAT1_IER  0x0fc4
#define MAXV_STAT2_F    0x0fc8
#define MAXV_STAT2_IER  0x0fcc
#define MAXV_IACK_IDV   0x0fd0
#define MAXV_CFG_SWITCH 0x0fd4
#define MAXV_CFG_AM     0x0fd8
#define MAXV_FIFO_CSR   0x0ff8
#define MAXV_FIFO_DATA  0x0ffc

#define MAXV_COMMAND_ERROR      0x01000000
#define MAXV_RESPONSE_AVAILABLE 0x02000000

/*
 * Compute axis fast status position as X_OFFSET + CMD_POS_UPD
 */
/* Axis offsets */
#define X_OFFSET      0x0000
#define Y_OFFSET      0x0004
#define Z_OFFSET      0x0008
#define T_OFFSET      0x000C
#define U_OFFSET      0x0010
#define V_OFFSET      0x0014
#define R_OFFSET      0x0018
#define S_OFFSET      0x001C

/* Value offsets */
#define CMD_POS_UPD   0x0000 /* Command position available each update cycle*/
#define ENC_POS_UPD   0x0020 /* Encoder position available each update cycle*/
#define CMD_POS_REQ   0x0054 /* Command position available on request */
#define ENC_POS_REQ   0x0074 /* Encoder position available on request */

#define MAXV_DC_MBOX_ID_QUERY 1
#define MAXV_DC_MBOX_KILLMOVE 2 /* Kill all moves    */
#define MAXV_DC_MBOX_RESET    3 /* Reset controller  */
#define MAXV_DC_MBOX_REBOOT   4 /* Reboot controller */

#define MAXV_NAXIS            8


#endif // OMS_MAXV_H

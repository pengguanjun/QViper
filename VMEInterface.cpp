#include "VMEInterface.h"
#include <time.h>

 VMEInterface::VMEInterface(ADDRESSMODIFIERCODE vmeAddressModifier) : m_AM(vmeAddressModifier)
{

}

VMEInterface::~VMEInterface()
{

}

int VMEInterface::vsleep(unsigned int usec)
{
    struct timespec tm,tr;

      tm.tv_sec = 0;
      tm.tv_nsec = usec * 1000; /* Turn into microseconds */
      return nanosleep(&tm,&tr);
}

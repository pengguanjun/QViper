#ifndef APPCONFIG_H
#define APPCONFIG_H

#include "MotorParameters.h"
#include "ModuleParameters.h"
#include "MotorAxis.h"

int MC_ConfigRead (const char * cfgfile, MotorParameters * mp[]);
int MC_ConfigWrite(const char * cfgfile, MotorParameters * mp[]);
int MC_PositionCache(const char * cachefile, MotorAxis * ma[]);
int MC_PositionRead(const char * cachefile, double * position);
int Mod_ConfigRead(const char * modfile, ModuleParameters * pm[]);

#endif // APPCONFIG_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "AppConfig.h"

// Read old-school VIPER configuration files
int MC_ConfigRead(const char * cfgfile, MotorParameters * mp[])
{
    FILE * cfgfp;

    char instring[256], label[40], *ptr, *ptr2, ch;
    int num, motmod, motax, motorstepsperrevolution;
    int encmod, encax,encoderstepsperrevolution,encoderzero,encoderdirection;
    int deadband;
    int enabled, use_encoder, stalldetection, positionmaintenance, limit_enable;
    double dscale, vscale, bscale, ascale;
    int nmots;

    if ((cfgfp = fopen(cfgfile,"r")) == NULL) return -1;

    nmots = 0;
    while((ptr=fgets(instring,sizeof(instring)-1,cfgfp))!= NULL) {
        ch = instring[0];
        if (ch == '#') continue;
        ptr = &instring[0];
        num = strtol(ptr,&ptr2,0);

        ptr = strstr(ptr2,"\"");
        ptr++; //
        ptr2 = strstr(ptr,"\"");
        *ptr2='\0';
        strncpy(label,ptr,sizeof(label)-1);
        ptr = ptr2 + 1;
        ptr2++;

        motmod = strtol(ptr,&ptr2,0);
        ptr = ptr2;

        motax = strtol(ptr, &ptr2, 0);
        ptr = ptr2;

        motorstepsperrevolution = strtol(ptr,&ptr2, 0);
        ptr = ptr2;

        encmod = strtol(ptr,&ptr2,0);
        ptr = ptr2;

        encax = strtol(ptr, &ptr2, 0);
        ptr = ptr2;

        encoderstepsperrevolution = strtol(ptr,&ptr2, 0);
        ptr = ptr2;

        encoderzero = strtol(ptr,&ptr2,0);
        ptr = ptr2;

        encoderdirection = strtol(ptr,&ptr2,0);
        ptr = ptr2;

        deadband = strtol(ptr,&ptr2,0);
        ptr = ptr2;

        enabled = strtol(ptr,&ptr2,0);
        ptr = ptr2;

        use_encoder = strtol(ptr,&ptr2,0);
        ptr = ptr2;

        stalldetection = strtol(ptr,&ptr2,0);
        ptr = ptr2;

        positionmaintenance = strtol(ptr,&ptr2,0);
        ptr = ptr2;

        limit_enable = strtol(ptr,&ptr2,0);
        ptr = ptr2;

        dscale = strtod(ptr,&ptr2);
        ptr = ptr2;

        bscale = strtod(ptr,&ptr2);
        ptr = ptr2;

        vscale = strtod(ptr,&ptr2);
        ptr = ptr2;

        ascale = strtod(ptr,&ptr2);

        if ((num<=0) || (num >= NAXIS)) continue;

        mp[num]->defined = true;
        strncpy(mp[num]->label,label,40);

        mp[num]->motmod = motmod;
        mp[num]->motax  = motax;
        mp[num]->MotorStepsPerRevolution = motorstepsperrevolution;
        mp[num]->encmod = encmod;
        mp[num]->encax  = encax;
        mp[num]->EncoderStepsPerRevolution = encoderstepsperrevolution;
        mp[num]->EncoderZero = encoderzero;
        mp[num]->EncoderDirection = encoderdirection;
        mp[num]->deadband = deadband;
        mp[num]->axis_enabled = enabled;
        mp[num]->use_encoder = use_encoder;
        mp[num]->stalldetection = stalldetection;
        mp[num]->positionmaintenance = positionmaintenance;
        mp[num]->limits_enabled = limit_enable;
        mp[num]->dscale = dscale;
        mp[num]->vscale = vscale;
        mp[num]->bscale = bscale;
        mp[num]->ascale = ascale;

        // Safe state variables
        mp[num]->target = 0;
        mp[num]->increment = 1;

    }

    fclose(cfgfp);
    return nmots;
}

// Write configuration
int MC_ConfigWrite(const char * cfgfile, MotorParameters * mp[])
{
    FILE * cfgfp;

    int encoderdirection, deadband;
    int enabled, use_encoder, stalldetection, positionmaintenance, limit_enable;
    int nmots;
    int i;

    if ((cfgfp = fopen(cfgfile,"w")) == NULL) return -1;
    nmots = 0;
    for (i=0;i<NAXIS;i++) {
        if (mp[i]->defined) {
            fprintf(cfgfp,"%2d \"%15s\" %2d %2d %5d %2d %2d %5d %9d ",
                    i,mp[i]->label,
                    mp[i]->motmod,mp[i]->motax,mp[i]->MotorStepsPerRevolution,
                    mp[i]->encmod,mp[i]->encax,mp[i]->EncoderStepsPerRevolution,
                    mp[i]->EncoderZero);
            encoderdirection = mp[i]->EncoderDirection;
            deadband = mp[i]->deadband;
            enabled = (mp[i]->axis_enabled) ? 1 : 0;
            use_encoder = (mp[i]->use_encoder) ? 1 : 0;
            stalldetection = (mp[i]->stalldetection) ? 1 : 0;
            positionmaintenance = (mp[i]->positionmaintenance) ? 1 : 0;
            limit_enable = (mp[i]->limits_enabled) ? 1 : 0;
            fprintf(cfgfp,"%2d %3d %2d %2d %d %d %d ",
                    encoderdirection,deadband,enabled,use_encoder,stalldetection,
                    positionmaintenance,limit_enable);
            fprintf(cfgfp,"%9.5f %7.4f %7.4f %7.4f\r\n",
                    mp[i]->dscale,mp[i]->bscale,mp[i]->vscale,mp[i]->ascale);
        }
    }

    return nmots;
}

// Return number of axes cached
int MC_PositionCache(const char * cachefile, MotorAxis * ma[])
{
    FILE *fp;
    char outstring[256];
    int i,naxis;

    if ((fp = fopen(cachefile,"w")) == NULL) return -1;

    naxis=0;
    for(i=0;i<NAXIS;i++) {
        if (ma[i]->defined) {
            sprintf(outstring,"%d\t%+9.4f\n",i,ma[i]->position);
            fputs(outstring,fp);
            naxis++;
        }
    }
    fclose(fp);

    return naxis;
}

// Return number of axes restored
// Read back into a floating point array
int MC_PositionRead(const char * cachefile, double * position)
{
    FILE *fp;
    char instring[256], *ptr, *ptr2;
    int index, naxis;
    double pos;

    if ((fp = fopen(cachefile,"r")) == NULL) return -1;
    naxis = 0;
    while ((ptr=fgets(instring,sizeof(instring)-1,fp)) != NULL) {
        // Parse the file and store positions as floats
        index = (int) strtol(ptr,&ptr2,0);
        ptr = ptr2++;
        pos = strtod(ptr,&ptr2);

        if ((index < 0) || (index > NAXIS)) continue;
        *(position + index) = pos;
        naxis++;
    }
    fclose(fp);
    return naxis;
}

int Mod_ConfigRead(const char * modfile,  ModuleParameters * pm[])
{
    FILE * fp;
    char instring[256], modclass[40], model[40], *ptr, *ptr2, ch;
    int i, nmods;
    unsigned int base;

    if ((fp = fopen(modfile,"r")) == NULL) return -1;

    nmods = 0;
    while((ptr=fgets(instring,sizeof(instring)-1,fp))!= NULL) {
        memset(modclass,0,sizeof(modclass));
        memset(model,0,sizeof(model));

        ch = instring[0];
        if (ch == '#') continue;
        ptr = &instring[0];

        ptr2 = strchr(ptr,' ');
        if (ptr2 == NULL) continue; // Should complain too

        *ptr2=0;
        qstrncpy(modclass,ptr,sizeof(modclass)-1);
        ptr = ptr2 + 1;

        while((ch = *ptr)==' ') ptr++;
        ptr2 = strchr(ptr,' ');
        if (ptr2 == NULL) continue; // Should complain too
        *ptr2=0;
        qstrncpy(model,ptr,sizeof(model)-1);
        ptr = ptr2 + 1;

        base = strtoul(ptr,&ptr2,0);

        qstrncpy(pm[nmods]->m_Class, modclass, sizeof(pm[nmods]->m_Class)-1);
        qstrncpy(pm[nmods]->m_Model,model,sizeof(pm[nmods]->m_Model)-1);
        pm[nmods]->m_BaseAddress = base;

        nmods++;

    }

    fclose(fp);
    return nmods;
}

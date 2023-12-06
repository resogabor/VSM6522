#pragma once
#include "StdAfx.h"
#include "sdk/vsm.hpp"
#define CHIPS_IMPL
#include "m6522.h"


//#define InfoLog(__s__) sprintf_s(LogLineT, "%05d: ", LogLine++); myPopup->print(LogLineT); myPopup->print(__s__); myPopup->print("\n")
#define InfoLog(__s__) myPopup->print(__s__); myPopup->print("\n")
//#define DEBUGCALLS

#define SetHigh drivestate(time, SHI)
#define SetLow drivestate(time, SLO)
#define SetFloat drivestate(time, FLT)



class DsimModel : public IDSIMMODEL
{
public:
	INT isdigital (CHAR *pinname);
	VOID setup (IINSTANCE *inst, IDSIMCKT *dsim);
	VOID runctrl (RUNMODES mode);
	VOID actuate (REALTIME time, ACTIVESTATE newstate);
	BOOL indicate (REALTIME time, ACTIVEDATA *data);
	VOID clockstep(ABSTIME time, DSIMMODES mode);
	VOID simulate(ABSTIME time, DSIMMODES mode);
	VOID callback (ABSTIME time, EVENTID eventid);
	
private:
	VOID SetAddr(uint16_t val, ABSTIME time);
	VOID SetData(uint8_t val, ABSTIME time);
	uint8_t GetData(void);
	void HIZAddr(ABSTIME time);
	void HIZData(ABSTIME time);
	void UpdateData(ABSTIME time);
	void ReadControlPins();
	void SetControlPins(ABSTIME time);
	void DebugLog();
	

	IINSTANCE *inst;
	IDSIMCKT *ckt;
	IDSIMPIN* pin_CA1, * pin_CA2;
	IDSIMPIN* pin_CB1, * pin_CB2;
	IDSIMPIN* pin_CS1, * pin_CS2;
	IDSIMPIN* pin_RS0, * pin_RS1, * pin_RS2, * pin_RS3;
	IDSIMPIN* pin_IRQ;
	IDSIMPIN* pin_RW;
	IDSIMPIN* pin_RES;
	IDSIMPIN* pin_CLK;
	IDSIMPIN* pin_PA[8];
	IDSIMPIN* pin_PB[8];
	IDSIMPIN* pin_D[8];

	IDEBUGPOPUP *myPopup;

	// Global variables
	UINT8 cycle = 0;		// Current cycle of the state machine
	

	int LogLine = 1;
	char LogLineT[10];
	char LogMessage[256];
	char DebugMessage[256];

	m6522_t via;
	uint64_t pins;
	


};

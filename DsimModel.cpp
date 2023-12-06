#include "StdAfx.h"
#include "DsimModel.h"


//#define DEBUG
//static z80_t cpu;
//static uint64_t pins;
//
const STATE pLOHI[] = { SLO,FLT,FLT,FLT,SLO,SHI };
const STATE dLOHI[] = { SLO,SHI,FLT,FLT };


void DsimModel::HIZAddr(ABSTIME time) {						// Sets the address bus to HIZ
	int i;

	for (i = 0; i < 16; i++) {
		pin_A[i]->SetFloat;
	}
}

void DsimModel::HIZData(ABSTIME time) {						// Sets the data bus to HIZ
	int i;

	for (i = 0; i < 8; i++) {
		pin_D[i]->SetFloat;
	}
}

void DsimModel::SetAddr(uint16_t val, ABSTIME time) {			// Sets an address onto the address bus
	int i, j;

	for (i = 0; i < 16; i++) {
		j = (val >> i) & 0x01;
		if (j) {
			pin_A[i]->SetHigh;
		} else {
			pin_A[i]->SetLow;
		}
	}
}

void DsimModel::SetData(uint8_t val, ABSTIME time) {			// Sets a value onto the data bus
	int i, j;

	for (i = 0; i < 8; i++) {
		j = (val >> i) & 0x01;
		if (j) {
			pin_D[i]->SetHigh;
		} else {
			pin_D[i]->SetLow;
		}
	}
}

uint8_t DsimModel::GetData(void) {							// Reads a value from the data bus
	int i;
	uint8_t val = 0;

	for (i = 0; i < 8; i++) {
		if (ishigh(pin_D[i]->istate()))
			val |= (1 << i);
	}
	return(val);
}

void DsimModel::UpdateData(ABSTIME time)
{
	uint64_t val;
	if (pins & M6502_RW) {
		/* memory read */
		val = GetData();
		M6502_SET_DATA(pins, val);
	}
	else {
		/* memory write */
		val = M6502_GET_DATA(pins);
		SetData(val, time);
		HIZData(time + 20000);
	}
	sprintf_s(LogMessage, "     Address: 0x%X  R/W: %c  data: 0x%X   Status: 0x%llX", M6502_GET_ADDR(pins), (pins & M6502_RW)?'R':'W', M6502_GET_DATA(pins), pins);
	InfoLog(LogMessage);
}

void DsimModel::ReadControlPins()
{
	ishigh(pin_RS0->istate()) ? pins = pins | M6522_RS0 : pins = pins & ~M6522_RS0;
	ishigh(pin_RS1->istate()) ? pins = pins | M6522_RS1 : pins = pins & ~M6522_RS1;
	ishigh(pin_RS2->istate()) ? pins = pins | M6522_RS2 : pins = pins & ~M6522_RS2;
	ishigh(pin_RS3->istate()) ? pins = pins | M6522_RS3 : pins = pins & ~M6522_RS3;
	ishigh(pin_CS1->istate()) ? pins = pins | M6522_CS1 : pins = pins & ~M6522_CS1;
	ishigh(pin_CA1->istate()) ? pins = pins | M6522_CA1 : pins = pins & ~M6522_CA1;
	//negált
	ishigh(pin_CS2->istate()) ? pins = pins & ~M6522_CS2 : pins = pins | M6522_CS2;	
	ishigh(pin_RW->istate()) ? pins = pins & ~M6522_RW : pins = pins | M6522_RW;

}

void DsimModel::SetControlPins(ABSTIME time)
{
	(pins & M6502_RW) ? pin_RW->SetHigh : pin_RW->SetLow;
	(pins & M6502_SYNC) ? pin_SYNC->SetHigh : pin_SYNC->SetLow;
}

void DsimModel::DebugLog()
{
	int i;
	
	uint8_t val = GetData();
	uint16_t addr = 0;

	for (i = 0; i < 16; i++) {
		if (ishigh(pin_A[i]->istate()))
			addr |= (1 << i);
	}
	/*
	for (i = 0; i < 8; i++) {
		if (ishigh(pin_D[i]->istate()))
			val |= (1 << i);
	}*/
	sprintf_s(DebugMessage, "Pin Status  Addr: 0x%X   data: 0x%X  R/W: %c", addr, val, ishigh(pin_RW->istate()) ? 'R' : 'W');
	InfoLog(DebugMessage);
}

INT DsimModel::isdigital(CHAR *pinname) {
	return TRUE;											// Indicates all the pins are digital
}

VOID DsimModel::setup(IINSTANCE* instance, IDSIMCKT* dsimckt) {

	int n;
	char s[8];

	inst = instance;
	ckt = dsimckt;



	CREATEPOPUPSTRUCT* cps = new CREATEPOPUPSTRUCT;
	cps->caption = "M6502 Simulator Debugger Log";			// WIN Header
	cps->flags = PWF_VISIBLE | PWF_SIZEABLE;				// Show + Size
	cps->type = PWT_DEBUG;									// WIN DEBUG
	cps->height = 500;
	cps->width = 400;
	cps->id = 123;

	myPopup = (IDEBUGPOPUP*)instance->createpopup(cps);

	InfoLog("Connecting control pins...");

	pin_CA1 = inst->getdsimpin("CA1", true);
	pin_CA2 = inst->getdsimpin("CA2", true);
	pin_CB1 = inst->getdsimpin("CB1", true);
	pin_CB2 = inst->getdsimpin("CB2", true);
	pin_CS1 = inst->getdsimpin("CS1", true);
	pin_CS2 = inst->getdsimpin("CS2", true);
	pin_RS0 = inst->getdsimpin("RS0", true);
	pin_RS1 = inst->getdsimpin("RS1", true);
	pin_RS2 = inst->getdsimpin("RS2", true);
	pin_RS3 = inst->getdsimpin("RS3", true);
	pin_IRQ = inst->getdsimpin("$IRQ$", true);
	pin_RW = inst->getdsimpin("R/$W$", true);
	pin_RES = inst->getdsimpin("$RES$", true);
	pin_CLK = inst->getdsimpin("CLK", true);

	InfoLog("Connecting data pins...");
	for (n = 0; n < 8; n++) {
		s[0] = 'D';
		_itoa_s(n, &s[1], 7, 10);
		pin_D[n] = inst->getdsimpin(s, true);
	}

	InfoLog("Connecting PA pins...");
	for (n = 0; n < 16; n++) {
		s[0] = 'P'; s[1] = 'A';
		_itoa_s(n, &s[2], 7, 10);
		pin_PA[n] = inst->getdsimpin(s, true);
	}

	InfoLog("Connecting PB pins...");
	for (n = 0; n < 16; n++) {
		s[0] = 'P'; s[1] = 'B';
		_itoa_s(n, &s[2], 7, 10);
		pin_PB[n] = inst->getdsimpin(s, true);
	}
	LogLine = 1;
	m6522_init(&via);

	// Connects function to handle Clock steps (instead of using "simulate")
	pin_CLK->sethandler(this, (PINHANDLERFN)&DsimModel::clockstep);
}

VOID DsimModel::runctrl(RUNMODES mode) {
}

VOID DsimModel::actuate(REALTIME time, ACTIVESTATE newstate) {
}

BOOL DsimModel::indicate(REALTIME time, ACTIVEDATA *data) {
	return FALSE;
}

VOID DsimModel::clockstep(ABSTIME time, DSIMMODES mode) {
	if (pin_CLK->isnegedge()) {
		ReadInputControlPins();
		DebugLog();			
		pins = m6502_tick(&cpu, pins);
		const uint16_t addr = M6502_GET_ADDR(pins);
		SetAddr(addr, time);
		SetOutputControlPins(time);		
	}
	if (pin_CLK->isposedge()) {
		//SetOutputControlPins(time);
		ReadInputControlPins();
		UpdateData(time); 
		if (0 == (pins & M6502_SYNC)) {
			sprintf_s(LogMessage, "cyle number is: %d", cycle);
			InfoLog(LogMessage);
			cycle = 1;
		}
		else {
			cycle++;
		}
		DebugLog();
	}
}

VOID DsimModel::simulate(ABSTIME time, DSIMMODES mode) {
}

VOID DsimModel::callback(ABSTIME time, EVENTID eventid) {
}

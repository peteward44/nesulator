

#ifndef MODECONSTANTS_H
#define MODECONSTANTS_H

#include <string>




class IModeConstants
{
public:
	virtual ~IModeConstants() {}

	virtual bool IsNTSC() const = 0;
	virtual float RefreshRate() const = 0;
	virtual int MasterCyclesPerCPU() const = 0;
	virtual int VBlankScanlines() const = 0;
	virtual int FrameScanlines() const = 0;
	virtual std::wstring Name() const = 0;
	virtual TERRITORY_MODE Mode() const = 0;
};


class NTSCModeConstants : public IModeConstants
{
public:
	virtual bool IsNTSC() const { return true; }
	virtual float RefreshRate() const { return 60.1f; }
	virtual int MasterCyclesPerCPU() const { return 15; }
	virtual int VBlankScanlines() const { return 20; }
	virtual int FrameScanlines() const { return 262; }

	virtual std::wstring Name() const { return L"NTSC"; }
	virtual TERRITORY_MODE Mode() const { return TERRITORY_NTSC; }
};


class PALModeConstants : public IModeConstants
{
public:
	virtual bool IsNTSC() const { return false; }
	virtual float RefreshRate() const { return 50.0f; }
	virtual int MasterCyclesPerCPU() const { return 16; }
	virtual int VBlankScanlines() const { return 70; }
	virtual int FrameScanlines() const { return 312; }

	virtual std::wstring Name() const { return L"PAL"; }
	virtual TERRITORY_MODE Mode() const { return TERRITORY_PAL; }
};


#endif


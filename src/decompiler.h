

#ifndef DECOMPILER_H
#define DECOMPILER_H

#include "main.h"
#include <fstream>
#include <vector>
#include <map>
#include "cartridge.h"

class Decompiler
{
private:
	std::wofstream out;
	int labelnum;

	typedef std::map<UInt16_t, std::wstring> LabelMap_t;
	typedef std::map< UInt16_t, std::wstring > CodeMap_t;

	LabelMap_t labels;
	CodeMap_t code;

	std::wstring GenerateLabelName();

	void DecompileFunction( CartridgePtr_t cart, const std::wstring& label, UInt16_t address );

public:
	Decompiler();

	void Save( const std::wstring& romFile, const std::wstring& outputFile );

};


#endif

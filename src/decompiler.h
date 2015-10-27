

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
	std::ofstream out;
	int labelnum;

	typedef std::map<UInt16_t, std::string> LabelMap_t;
	typedef std::map< UInt16_t, std::string > CodeMap_t;

	LabelMap_t labels;
	CodeMap_t code;

	std::string GenerateLabelName();

	void DecompileFunction( CartridgePtr_t cart, const std::string& label, UInt16_t address );

public:
	Decompiler();

	void Save( const std::string& romFile, const std::string& outputFile );

};


#endif

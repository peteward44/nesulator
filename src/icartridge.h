
#ifndef ICARTRIDGE_H
#define ICARTRIDGE_H


class ICartridge
{
public:
	virtual ~ICartridge() {}

	virtual unsigned int GetCRC32() const = 0 {}
	virtual std::wstring GetName() const = 0 {}
	
};


#endif


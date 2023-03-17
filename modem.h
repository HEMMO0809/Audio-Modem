#pragma once
#include <vector>

constexpr double pi = 3.14159265359;

class modem
{
protected:
	bool is_receiving;
	int counter;

public:
	modem() : is_receiving(false), counter(0) {};

	virtual void modulate(std::vector<int16_t>& dst, void* src, int bytes) = 0;
	virtual int demodulate(std::vector<int16_t>& src, std::vector<int8_t>& dst) = 0;
	virtual int sync(std::vector<int16_t>& dst) = 0;
};


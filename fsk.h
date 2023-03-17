#pragma once
#include "modem.h"
#include <vector>

constexpr int16_t max_volume = 32767;

enum class write_mode {
	hi = 0, lo = 1, delay = 2 
};

class fsk : public modem
{
private:
	int sample_rate;
	int baud_rate;
	int samples_per_baud;
	int max_transpose_byte;

	std::vector<double> hi_cos, hi_sin;
	std::vector<double> lo_cos, lo_sin;
	std::vector<int16_t> hi, lo;

	void write(std::vector<int16_t>& dst, write_mode mode, int num = 1);
	void write_handshake(std::vector<int16_t>& dst);

	void fft(std::vector<int16_t>& src, int offset, double& hi, double& lo);
	void phase(std::vector<int16_t>& src, int offset, double& cos, double& sin);

public:
	fsk(int sample_rate, int baud_rate);
	
	void modulate(std::vector<int16_t>& dst, void* src, int bytes);
	int sync(std::vector<int16_t>& dst);
	int demodulate(std::vector<int16_t>& src, std::vector<int8_t>& dst);
};


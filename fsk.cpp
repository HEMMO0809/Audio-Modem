#include "fsk.h"
#include <cmath>
#include <bitset>

fsk::fsk(int sample_rate, int baud_rate) : modem() {
	this->sample_rate = sample_rate;
	this->baud_rate = baud_rate;
	samples_per_baud = sample_rate / baud_rate;
	max_transpose_byte = 512;

	hi_cos.assign(samples_per_baud, 0);
	lo_cos.assign(samples_per_baud, 0);
	hi_sin.assign(samples_per_baud, 0);
	lo_sin.assign(samples_per_baud, 0);

	hi.assign(samples_per_baud, 0);
	lo.assign(samples_per_baud, 0);
	
	for (int i = 0; i < samples_per_baud; ++i) {
		double x = 2 * pi * i / samples_per_baud;

		hi_cos[i] = std::cos(2 * x);
		hi_sin[i] = std::sin(2 * x);
		lo_cos[i] = std::cos(x);
		lo_sin[i] = std::sin(x);

		hi[i] = hi_cos[i] * max_volume;
		lo[i] = lo_cos[i] * max_volume;
	}
}

void fsk::write(std::vector<int16_t>& dst, write_mode mode, int num) {
	if (mode == write_mode::delay) dst.insert(dst.end(), num, 0);

	else {
		for (int i = 0; i < num; ++i) {
			if (mode == write_mode::hi) dst.insert(dst.end(), hi.begin(), hi.end());
			else dst.insert(dst.end(), lo.begin(), lo.end());
		}
	}
}

void fsk::write_handshake(std::vector<int16_t>& dst) {
	write(dst, write_mode::lo, 5);
	write(dst, write_mode::hi);
}

void fsk::modulate(std::vector<int16_t>& dst, void* src, int bytes) {
	write(dst, write_mode::delay, 1000);

	for (int i = 0; i < bytes; ++i) {
		if (i % max_transpose_byte == 0) write_handshake(dst);

		std::bitset<8> bits(((char*)src)[i]);

		for (int k = 7; k >= 0; --k) {
			int bit = bits[k];

			if (bit) write(dst, write_mode::hi);
			else write(dst, write_mode::lo);
		}
	}
}

void fsk::fft(std::vector<int16_t>& src, int offset, double& hi, double& lo) {
	double hc = 0, hs = 0, lc = 0, ls = 0;

	for (int i = 0; i < samples_per_baud; ++i) {
		double x = src[offset + i] / max_volume;

		hc += hi_cos[i] * x;
		hs += hi_sin[i] * x;
		lc += lo_cos[i] * x;
		ls += lo_sin[i] * x;
	}

	hc /= samples_per_baud;
	hs /= samples_per_baud;
	lc /= samples_per_baud;
	ls /= samples_per_baud;

	hi = std::sqrt(hc * hc + hs * hs);
	lo = std::sqrt(lc * lc + ls * ls);
}

void fsk::phase(std::vector<int16_t>& src, int offset, double& cos, double& sin) {
	cos = 0;
	sin = 0;

	for (int i = 0; i < samples_per_baud; ++i) {
		double x = src[offset + i] / max_volume;
		cos += lo_cos[i] * x;
		sin += lo_sin[i] * x;
	}

	cos /= samples_per_baud;
	sin /= samples_per_baud;
}

int fsk::sync(std::vector<int16_t>& dst) {
	if (dst.size() < 10 * samples_per_baud) return -1;

	double hi, lo, cos, sin;
	bool signal_found = false;
	int index = 0;

	for (; index + (7 * samples_per_baud) <= dst.size(); index += samples_per_baud) {
		fft(dst, index, hi, lo);
		if (lo > 0.7) {
			break;
			signal_found = true;
		}
	}

	if (!signal_found) return -1;

	double max_score = -123, score;
	int max_index;
	index += samples_per_baud; 


	for (; index + samples_per_baud <= dst.size(); index++) {
		phase(dst, index, cos, sin);

		double score = cos - std::abs(sin);
		if (score > max_score) {
			max_score = score;
			max_index = index;
		}
	}

	return max_index;
}

int fsk::demodulate(std::vector<int16_t>& src, std::vector<int8_t>& dst) {
	double hi, lo;
	int index = 0;
	std::string buffer = "";

	for (; index + 5 * samples_per_baud <= dst.size(); index++) {
		fft(src, index, hi, lo);
		if (counter == 512 || std::max(hi, lo) < 0.5) {
			counter = 0;
			return index + samples_per_baud;
		}

		if (!is_receiving) {
			if (hi > lo) is_receiving = true;
			continue;
		}

		buffer += hi > lo ? '1' : '0';

		if (buffer.size() == 8) {
			dst.push_back(std::bitset<8>{buffer}.to_ulong());
			buffer = "";
			counter += 1;
		}
	}

	return index;
}
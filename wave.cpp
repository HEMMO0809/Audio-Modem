#include "wave.h"
#include <fstream>
#include <iostream>

wave::wave(int sample_rate, int bytes_per_sample, int n_channels) {
    std::memcpy(header.riff_header, "RIFF", 4);
    header.wav_size = 0;
    std::memcpy(header.wave_header, "WAVE", 4);

    std::memcpy(header.fmt_header, "fmt ", 4);
    header.fmt_chunk_size = 16;
    header.audio_format = 1;
    header.num_channels = 1;
    header.sample_rate = sample_rate;
    header.byte_rate = sample_rate * n_channels * bytes_per_sample; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    header.sample_alignment = n_channels * bytes_per_sample; // num_channels * Bytes Per Sample
    header.bit_depth = 8 * bytes_per_sample; // Number of bits per sample

    std::memcpy(header.data_header, "data", 4);
    header.data_bytes = 0;
}

void wave::open_file(const std::string& file_name) {
	std::fstream stream;
	stream.open(file_name, std::ios::binary || std::ios::in);

	if (!stream.is_open()) return;

	stream.seekg(0, std::ios::end);
	size_t data_size = (size_t)stream.tellg() - wave_header_size;
	stream.seekg(0, std::ios::beg);

	data.assign(data_size, 0);
	stream.read((char*)&header, wave_header_size);
	stream.read((char*)data.data(), data_size);

    stream.close();
}

wave::wave(const std::string& file_name) {
    open_file(file_name);
}

wave::wave(const wave& w) {
    std::memcpy(&header, &w.header, wave_header_size);
    data = w.data;
}

void wave::write_file(const std::string& file_name) {
    std::fstream stream;
    stream.open(file_name, std::ios::out | std::ios::binary);

    if (!stream.is_open()) return;

    header.data_bytes = data.size();
    header.wav_size = header.data_bytes + (wave_header_size - 8);

    stream.write((char*)&header, wave_header_size);
    stream.write((char*)data.data(), data.size());

    stream.close();
}

void wave::write(void* src, int size) {
    data.insert(data.end(), (char*)src, (char*)src + size);
}
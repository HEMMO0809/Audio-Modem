#pragma once
#include <string>
#include <vector>

#pragma pack(push, 1)
typedef struct wave_header {
    int8_t riff_header[4]; // Contains "RIFF"
    int32_t wav_size; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    int8_t wave_header[4]; // Contains "WAVE"

    int8_t fmt_header[4]; // Contains "fmt " (includes trailing space)
    int32_t fmt_chunk_size; // Should be 16 for PCM
    int16_t audio_format; // Should be 1 for PCM. 3 for IEEE Float
    int16_t num_channels;
    int32_t sample_rate;
    int32_t byte_rate; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    int16_t sample_alignment; // num_channels * Bytes Per Sample
    int16_t bit_depth; // Number of bits per sample

    int8_t data_header[4]; // Contains "data"
    int32_t data_bytes; // Number of bytes in data. Number of samples * num_channels * sample byte size
} wave_header;
#pragma pack(pop)

constexpr size_t wave_header_size = sizeof(wave_header);

class wave
{
private:
    using container_type = std::vector<int8_t>;

    wave_header header;
    container_type data;
public:
    wave(int sample_rate = 44100, int bytes_per_sample = 2, int n_channels = 1);
    wave(const wave& w);
    wave(const std::string& file_name);

    void write(void* src, int bytes);

    void open_file(const std::string& file_name);
    void write_file(const std::string& file_name);

    container_type& get_data() { return data; }
    wave_header& get_header() { return header; }
};


#include <iostream>
#include <fstream>
#include "fsk.h"
#include "wave.h"
#include <vector>

int main() {
	wave w;
	modem* modem_device = new fsk(44100, 400);
	std::fstream stream("a.txt", std::ios::in | std::ios::binary);

	int size = 0;
	stream.seekg(0, std::ios::end);
	size = stream.tellg();
	stream.seekg(0, std::ios::beg);

	std::vector<int8_t> data(size);
	std::vector<int16_t> modulated;
	stream.read((char*)data.data(), size);
	stream.close();

	modem_device->modulate(modulated, data.data(), data.size());
	w.write(modulated.data(), modulated.size() * 2);
	w.write_file("modulated.wav");
	

	return 0;
}

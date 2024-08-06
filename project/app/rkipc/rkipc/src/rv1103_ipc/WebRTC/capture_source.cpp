#include <fstream>

#include "capture_source.hpp"

#include "app_dbg.h"
#include <cstring>
#include <algorithm> 
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <cstddef>
using namespace std;


CaptureSource::CaptureSource(uint8_t fps) {
    this->loop = loop;
    this->sampleDuration_us = 1000 * 1000 / fps;
}

CaptureSource::~CaptureSource() {
	stop();
}

void CaptureSource::start() {
    sampleTime_us = std::numeric_limits<uint64_t>::max() - sampleDuration_us + 1;
    // loadNextSample();
}

void CaptureSource::stop() {
    sample = {};
    sampleTime_us = 0;
    counter = -1;
}


void dump_video(const char* file, unsigned char *data, int data_len)
{
    FILE *fp = fopen(file, "wb");

    if(fp) 
    {
        for (int i = 0; i < data_len; i++) {
            if (i + 1 % 16 == 0) {
                fprintf(fp, "\n");
            }
			fprintf(fp, "%02X ", data[i]);
        }
        fclose(fp);
    }
	
}

void CaptureSource::loadNextSample(){

}

void CaptureSource::insertDataToSample(uint8_t *data, uint32_t size) {
	uint32_t length = (uint32_t) size;
    for (int i = 3; i >= 0; i--) {
        sample.push_back(static_cast<std::byte>(length >> (i * 8) & 0xFF));
    }
    for(int i = 0; i < size; i++) {
        sample.push_back(static_cast<std::byte> (data[i]));
    }
}
/*
*
* Cam H264: 0x00 0x00 0x00 0x01          - data - 0x00 0x00 0x00 0x01          - data ...
* Convert : 0x00 0x00 0x00 (size data) - data - 0x00 0x00 0x00 (size data) - data ...
*/


void CaptureSource::loadNextSample(uint8_t *data, int size) {
	
	// APP_DBG("Load Next Sample\r\n");
    if(!sample.empty()) {
        sample.clear();
        sample.shrink_to_fit();
    }
#define ZERO_BIT_SIZE   (4)
    vector<uint8_t> forbidden_zero_bit = {0x00, 0x00, 0x00, 0x01};
    std::vector<uint8_t> _data(data, data + size);
	// dump_video("/mnt/video_original.txt", (unsigned char *)_data.data(), size);
    uint32_t index_zero_bit;
	// APP_DBG("forbidden_zero_bit size :%d \r\n", forbidden_zero_bit.size());
    auto found_zero_bit = std::search(_data.begin(), _data.end(), forbidden_zero_bit.begin(), forbidden_zero_bit.end());
    uint8_t q = 0;
	while (found_zero_bit != _data.end()) {
		_data.erase(_data.begin(), _data.begin() + 4);
		// string a = "/mnt/video_original_" + std::to_string(q) + ".txt";
		// dump_video(a.data(), _data.data(), _data.size());
		found_zero_bit = std::search(_data.begin(), _data.end(), forbidden_zero_bit.begin(), forbidden_zero_bit.end());
		
		if(!(found_zero_bit != _data.end())) {
			break;
		}
		index_zero_bit = std::distance(_data.begin(), found_zero_bit);
		// APP_DBG("index_zero_bit : %d\r\n", index_zero_bit);
		insertDataToSample((uint8_t*)_data.data(), index_zero_bit);
		_data.erase(_data.begin(), _data.begin() + index_zero_bit);
		q++;
    }
	if(!_data.empty()) {
		insertDataToSample((uint8_t*)_data.data(), _data.size());
		_data.clear();
		_data.shrink_to_fit();
	}
	
	// dump_video("/mnt/video_out.txt", (unsigned char*)sample.data(), sample.size());
    size_t i = 0;
	while (i < sample.size()) {
		assert(i + 4 < sample.size());
		auto lengthPtr		= (uint32_t *)(sample.data() + i);
		uint32_t size		= ntohl(*lengthPtr);
		auto naluStartIndex = i + 4;
		auto naluEndIndex	= naluStartIndex + size;
		assert(naluEndIndex <= sample.size());
		auto header = reinterpret_cast<rtc::NalUnitHeader *>(sample.data() + naluStartIndex);
		auto type	= header->unitType();

		switch (type) {
		case 7:
			previousUnitType7 = {sample.begin() + i, sample.begin() + naluEndIndex};
		break;
		case 8:
			previousUnitType8 = {sample.begin() + i, sample.begin() + naluEndIndex};
		break;
		case 5:
			previousUnitType5 = {sample.begin() + i, sample.begin() + naluEndIndex};
		break;
		}
		i = naluEndIndex;
	}
}


rtc::binary CaptureSource::getSample() {
	return sample;
}

uint64_t CaptureSource::getSampleTime_us() {
	return sampleTime_us;
}

uint64_t CaptureSource::getSampleDuration_us() {
	return sampleDuration_us;
}

void CaptureSource::loadNextTime(){
    // APP_DBG("Load Next Time\r\n");
    sampleTime_us += sampleDuration_us;
    // cout << "Sample Time :" << sampleTime_us << endl;
}

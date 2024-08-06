#ifndef _CAPTURE_SOURCE_H_
#define _CAPTURE_SOURCE_H_

#include <string>
#include <vector>
#include "stream.hpp"
#include "rtc/rtc.hpp"

class CaptureSource : public StreamSource {
    uint64_t sampleDuration_us = 1000 * 1000 / 10;
    uint64_t sampleTime_us = 0;
    uint32_t counter = -1;
    bool loop;
    uint64_t loopTimestampOffset = 0;

	std::optional<std::vector<std::byte>> previousUnitType5 = std::nullopt;
    std::optional<std::vector<std::byte>> previousUnitType7 = std::nullopt;
    std::optional<std::vector<std::byte>> previousUnitType8 = std::nullopt;
protected:
    rtc::binary sample = {};
	void insertDataToSample(uint8_t *, uint32_t );
    void insertDataToSample(std::vector<uint8_t> data, uint32_t size);
public:
    CaptureSource(uint8_t);
    virtual ~CaptureSource();
    virtual void start() override;
    virtual void stop() override;
	virtual void loadNextSample() override;
    virtual void loadNextSample(uint8_t *, int ) override;
	virtual void loadNextTime() override;

    rtc::binary getSample() override;
    uint64_t getSampleTime_us() override;
    uint64_t getSampleDuration_us() override;
	
};

#endif //_CAPTURE_SOURCE_H_
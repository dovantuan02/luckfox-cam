/**
 * libdatachannel streamer example
 * Copyright (c) 2020 Filip Klembara (in2core)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef opusfileparser_hpp
#define opusfileparser_hpp

// #include "fileparser.hpp"
#include "stream.hpp"

class OPUSFileParser: public StreamSource {
    static const uint32_t defaultSamplesPerSecond = 50;

public:
    OPUSFileParser(std::string directory, bool loop, uint32_t samplesPerSecond = OPUSFileParser::defaultSamplesPerSecond);
    virtual void start() override;
    virtual void stop() override;
	virtual void loadNextSample() override;
    virtual void loadNextSample(uint8_t *, int ) override;
	virtual void loadNextTime() override;

    rtc::binary getSample() override;
    uint64_t getSampleTime_us() override;
    uint64_t getSampleDuration_us() override;
};


#endif /* opusfileparser_hpp */

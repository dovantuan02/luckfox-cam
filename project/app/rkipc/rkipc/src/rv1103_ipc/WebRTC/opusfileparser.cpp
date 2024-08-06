/**
 * libdatachannel streamer example
 * Copyright (c) 2020 Filip Klembara (in2core)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "opusfileparser.hpp"

using namespace std;

OPUSFileParser::OPUSFileParser(string directory, bool loop,
                               uint32_t samplesPerSecond){}
void OPUSFileParser::start(){}
void OPUSFileParser::stop(){}
void OPUSFileParser::loadNextSample(){}
void OPUSFileParser::loadNextSample(uint8_t *, int ){}
void OPUSFileParser::loadNextTime(){}

rtc::binary OPUSFileParser::getSample(){}
uint64_t OPUSFileParser::getSampleTime_us(){}
uint64_t OPUSFileParser::getSampleDuration_us(){}

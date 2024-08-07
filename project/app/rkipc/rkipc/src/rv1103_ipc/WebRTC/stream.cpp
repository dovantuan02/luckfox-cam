/**
 * libdatachannel streamer example
 * Copyright (c) 2020 Filip Klembara (in2core)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "stream.hpp"
#include "helpers.hpp"

#include "app_dbg.h"
#include "task_webrtc.h"
#ifdef _WIN32
// taken from https://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
#include <windows.h>

void usleep(__int64 usec)
{
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}
#else
#include <unistd.h>
#endif

// std::mutex mtx_videoGetStream; 
std::mutex mtx_sendSample; 
Stream::Stream(std::shared_ptr<StreamSource> video, std::shared_ptr<StreamSource> audio):
	std::enable_shared_from_this<Stream>(), video(video), audio(audio) { }

Stream::Stream() {
	_isRunning = false;
}
Stream::~Stream() {
	stop();
}

std::pair<std::shared_ptr<StreamSource>, Stream::StreamSourceType> Stream::unsafePrepareForSample() {
	std::shared_ptr<StreamSource> ss;
	StreamSourceType sst;
	uint64_t nextTime;
	if (audio->getSampleTime_us() < video->getSampleTime_us()) {
		ss = audio;
		sst = StreamSourceType::Audio;
		nextTime = audio->getSampleTime_us();
	} else {
		ss = video;
		sst = StreamSourceType::Video;
		nextTime = video->getSampleTime_us();
	}

	auto currentTime = currentTimeInMicroSeconds();

	auto elapsed = currentTime - startTime;
	if (nextTime > elapsed) {
		auto waitTime = nextTime - elapsed;
		mutex.unlock();
		usleep(waitTime);
		mutex.lock();
	}
	return {ss, sst};
}

void Stream::sendSample() {
	std::lock_guard<std::mutex> lock(mtx_sendSample);
	if (!isRunning) {
		return;
	}
	// APP_DBG("Send Sample : Start\n");
	auto ssSST = unsafePrepareForSample();
	auto ss = ssSST.first;
	auto sst = ssSST.second;
	auto sample = ss->getSample();
	// <StreamSourceType, uint64_t, rtc::binary> 
	sampleHandler(sst, ss->getSampleTime_us(), sample);  
}

void Stream::onSample(std::function<void (StreamSourceType, uint64_t, rtc::binary)> handler) {
	sampleHandler = handler;
}

void Stream::stream_video(uint8_t *data, int length) {
	// APP_DBG("Video Get Stream\r\n");
	if (avStream == nullopt) {
		(void)data;
		// APP_DBG("[avStream] is nullopt\r\n");
		return;
	}

	if(clients.empty()) {
		// APP_DBG("No client found !\r\n");
		return;
	}
	auto videoStream = avStream.value();
	videoStream->video->loadNextSample(data, length);
	auto sample_capture = (videoStream->video->getSample());
	videoStream->video->loadNextTime();
	
	
	for(auto id_client: clients) {
		auto id = id_client.first;
		auto client = id_client.second;
		auto optTrackData = client->video;
		if (client->getState() == Client::State::Ready && optTrackData.has_value()) {
			auto trackData = optTrackData.value();
			auto rtpConfig = trackData->sender->rtpConfig;

			// sample time is in us, we need to convert it to seconds
			auto elapsedSeconds = double(videoStream->video->getSampleTime_us()) / (1000 * 1000);
			// get elapsed time in clock rate
			uint32_t elapsedTimestamp = rtpConfig->secondsToTimestamp(elapsedSeconds);
			// set new timestamp
			rtpConfig->timestamp = rtpConfig->startTimestamp + elapsedTimestamp;

			// get elapsed time in clock rate from last RTCP sender report
			auto reportElapsedTimestamp = rtpConfig->timestamp - trackData->sender->lastReportedTimestamp();
			// check if last report was at least 1 second ago
			if (rtpConfig->timestampToSeconds(reportElapsedTimestamp) > 1) {
				trackData->sender->setNeedsToReport();
			}

			// APP_DBG("Sending Video sample with size: %d : success\r\n " ,sample_capture.size());
			try {
				// send sample
				trackData->track->send(sample_capture);
			} catch (const std::exception &e) {
				// APP_DBG("Unable send Video sample : failed\r\n");
			}
		}
	}
	sample_capture.clear();
}

void Stream::start() {
	std::lock_guard lock(mutex);
	if (isRunning) {
		return;
	}
	_isRunning = true;
	startTime = currentTimeInMicroSeconds();
	audio->start();
	video->start();
}

void Stream::stop() {
	std::lock_guard lock(mutex);
	if (!isRunning) {
		return;
	}
	_isRunning = false;
	audio->stop();
	video->stop();
};

void Stream::start_stream(Stream& instance) {
	instance.start();
}

void Stream::stop_stream(Stream& instance) {
	instance.stop();
}

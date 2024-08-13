#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iomanip>
#include <cstdio>
#include <vector>
#include <variant>

#include <algorithm>
#include <future>
#include <iostream>
#include <memory>
// #include <random>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <chrono>
#include <pthread.h>
#include <assert.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <atomic>

// LIBDATACHANNEL
#include "rtc/rtc.hpp"

// AK
#include "app.h" 
#include "app_data.h"
#include "app_config.h"
#include "app_dbg.h"
#include "task_list.h"
#include "json.hpp"
#include "task_webrtc.h"

// WEBRTC
#include "capture_source.hpp"

using namespace rtc;
using namespace std;
using namespace chrono_literals;
using namespace chrono;
using json = nlohmann::json;
using OptionalPtr = variant<weak_ptr<WebSocket>, weak_ptr<MQTT>>;
/*		VARIABBLE		*/
template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }


unordered_map<string, shared_ptr<Client>> clients{};

/// Audio and video stream
optional<shared_ptr<Stream>> avStream = nullopt;

#define H264_DIRECTORY	"/mnt"
#define OPUS_DIRECTORY	"/tmp/audio"

q_msg_t gw_task_webrtc_mailbox;

/*		FUNCTION		*/

static void websocket_connect(shared_ptr<WebSocket> ws, string url);

shared_ptr<Client> createPeerConnection(const Configuration &config,
										OptionalPtr optionalPtr, 
										string id);

shared_ptr<ClientTrackData> addVideo(const shared_ptr<PeerConnection> pc,
									const uint8_t payloadType,
									const uint32_t ssrc, const string cname,
									const string msid,
									const function<void(void)> onOpen);

shared_ptr<ClientTrackData> addAudio(const shared_ptr<PeerConnection> pc,
									const uint8_t payloadType,
									const uint32_t ssrc, const string cname,
									const string msid,
									const function<void(void)> onOpen);

shared_ptr<Stream> createStream(const unsigned fps,
								const string opusSamples);

void startStream();

void addToStream(shared_ptr<Client> client, bool isAddingVideo);

void *gw_task_webrtc_entry(void *) {
	ak_msg_t *msg = AK_MSG_NULL;

	wait_all_tasks_started();

	string cam_local_id = "server";
	uint16_t cam_port = 8089;
	const string sturn_server = "stun:stun.l.google.com:19302";

	APP_DBG("[STARTED] gw_task_webrtc_entry\n");
	InitLogger(LogLevel::None);

	// Sturn server config
	Configuration config;
	APP_DBG("STUN SERVER : %s\n", sturn_server.c_str());
	config.iceServers.emplace_back(sturn_server);  // config sturn server
	config.disableAutoNegotiation = true;

	auto ws = std::make_shared<rtc::WebSocket>();
	string ws_url;
	while (1) {
		/* get messge */
		msg = ak_msg_rev(GW_TASK_WEBRTC_ID);

		switch (msg->header->sig) {
			case GW_WEBRTC_WEBSOCKET_REQ: {
				ws_url = std::string((char*)msg->header->payload, msg->header->len);
				APP_DBG("Url Websockets : %s\n", ws_url.c_str());
				websocket_connect(ws, ws_url);
			} break;

			case GW_WEBRTC_RECONNECT_WEBSOCKET_REG: {
				if (!ws->isOpen()) {
					APP_DBG_SIG("GW_WEBRTC_RECONNECT_WEBSOCKET_REG\r\n");
					ws->open(ws_url);
					APP_DBG_SIG("WEB SOCKET RECONNECT !\n");
					timer_set(GW_TASK_WEBRTC_ID,\
							GW_WEBRTC_RECONNECT_WEBSOCKET_REG,\
							GW_WEBRTC_RECONNECT_WEBSOCKET_INTERVAL, \
							TIMER_ONE_SHOT);
				}
			} break;

			case GW_WEBRTC_SET_SIGNALING_WEBSOCKET_REG: {
				APP_DBG_SIG("GW_WEBRTC_SET_SIGNALING_WEBSOCKET_REG\n");
				json data = json::parse(string((char *)msg->header->payload, msg->header->len));
				// APP_DBG("Websockets Send Message : %s\n", data.dump().c_str());
				ws->send(data.dump());
			} break;

			case GW_WEBRTC_GET_SIGNALING_WEBSOCKET_REG: {
				APP_DBG_SIG("GW_WEBRTC_GET_SIGNALING_WEBSOCKET_REG\n");
				std::string data((char*) msg->header->payload,msg->header->len);
				// get_data_dynamic_msg(msg, (uint8_t*)&data, msg->header->len);
				APP_DBG("WebSockets Receive Message : %s\n", data.c_str());
				
				json message = json::parse(data);

				auto it =  message.find("id");
				if(it == message.end()) {
					break;
				}
				string id = it->get<string>();
				APP_DBG("id : %s\n", id.c_str());

				it = message.find("type");
				if(it == message.end()) {
					break;
				}
				string type = it->get<string>();
				APP_DBG("type : %s\n", type.c_str());
				if(type == "request") {
					APP_DBG("Receive Request !\r\n");
					clients.emplace(id, createPeerConnection(config, make_weak_ptr(ws), id));
					
				} 
				else if (type == "answer") {
					if (auto jt = clients.find(id); jt != clients.end()) {
						auto pc = jt->second->peerConnection;
						auto sdp = message["sdp"].get<string>();
						auto description = Description(sdp, type);
						pc->setRemoteDescription(description);
						APP_DBG("Video Start \r\n");
#if AV_ENABLE == 1
						task_post_pure_msg(GW_TASK_AV_ID, GW_AV_START_REQ);
#endif // AV_ENABLE
					}
				}
				
			} break;
			
			default:
				break;
		}

		/* free message */
		ak_msg_free(msg);
	}

	return (void *)0;
}

void websocket_connect(shared_ptr<WebSocket> ws, string url) {
	/* set callback WebSocket	*/
	ws->onOpen([](){
		APP_DBG("Connected WebSocket !!\n");
#if AV_ENABLE == 1
		task_post_pure_msg(GW_TASK_AV_ID, GW_AV_INIT_REQ);
#endif
	});

	ws->onClosed([](){
		APP_DBG("Disconnected WebSocket !!\n");
	});

	ws->onError([](const string &error){
		APP_DBG("Connect WebSocket Error [%s]!!\n", error.c_str());
	});

	ws->onMessage([&](variant<binary, string> data) {
		if(!holds_alternative<string> (data)) {
			return;
		}
		string data_on_message = get<string>(data);
		APP_DBG("WebSockets On Message : %s\n", data_on_message.c_str());

		// task_post_pure_msg(GW_TASK_WEBRTC_ID, GW_WEBRTC_RECONNECT_WEBSOCKET_REG);
		task_post_dynamic_msg(GW_TASK_WEBRTC_ID, \
								GW_WEBRTC_GET_SIGNALING_WEBSOCKET_REG, \
								(uint8_t*)data_on_message.data(), \
								data_on_message.size());

		APP_DBG("WebSockets End Message !\n");
	});

	ws->open(url);

	if(!ws->isOpen()) {
		APP_DBG("WEB SOCKET RECONNECT !\n");
		timer_set(GW_TASK_WEBRTC_ID, \
					GW_WEBRTC_RECONNECT_WEBSOCKET_REG, \
					GW_WEBRTC_RECONNECT_WEBSOCKET_INTERVAL, \
					TIMER_ONE_SHOT);
	}
}

shared_ptr<Client> createPeerConnection(const Configuration &config,
										OptionalPtr optionalPtr, 
										string id) {

	auto pc = make_shared<PeerConnection>(config);
	auto client = make_shared<Client>(pc);

	pc->onStateChange([id](PeerConnection::State state) {
		/*
			0 : NEW
			1 : CONNECTING
			2 : CONNECTED
			3 : DISCONNECT
			4 : FAILED 
			5 : CLOSE
		*/
		cout << "State Peer :" << state << endl;
		if (state == PeerConnection::State::Disconnected ||
			state == PeerConnection::State::Failed ||
			state == PeerConnection::State::Closed) {
			// remove disconnected client
			clients.erase(id);
#if AV_ENABLE == 1
			task_post_pure_msg(GW_TASK_AV_ID, GW_AV_STOP_REQ);
#endif
		}
	});

	pc->onGatheringStateChange(
		[wpc = make_weak_ptr(pc), id, optionalPtr](PeerConnection::GatheringState state) {
		cout << "Gathering State: " << state << endl;
		if (state == PeerConnection::GatheringState::Complete) {
			if(auto pc = wpc.lock()) {
				auto description = pc->localDescription();
				json message = {
					{"id", id},
					{"type", description->typeString()},
					{"sdp", string(description.value())}
				};
				// Gathering complete, send answer
				if (holds_alternative<weak_ptr<WebSocket>>(optionalPtr)) {
					auto websocket_ptr = get<weak_ptr<WebSocket>>(optionalPtr).lock();
					if (websocket_ptr) {			// pointer websocket is available 
						// ws->send(message.dump());
						task_post_dynamic_msg(GW_TASK_WEBRTC_ID,\
												GW_WEBRTC_SET_SIGNALING_WEBSOCKET_REG,\
												(uint8_t*)to_string(message).data(),\
												to_string(message).size());
					}
					else {
						APP_DBG("websocket_ptr expired !\r\n");
					}
				}
			}
		}
	});

	client->video = addVideo(pc, 102, 1, "video-stream", "stream1", [id, wc = make_weak_ptr(client)]() {
		if (auto c = wc.lock()) {
				addToStream(c, true);
		};
		cout << "Video from " << id << " opened" << endl;
	});

	client->audio = addAudio(pc, 111, 2, "audio-stream", "stream1", [id, wc = make_weak_ptr(client)]() {
		if (auto c = wc.lock()) {
			addToStream(c, false);
		}
		cout << "Audio from " << id << " opened" << endl;
	});

	APP_DBG("DataChannel Create !!\n");
	auto dc = pc->createDataChannel("ping-pong");
	dc->onOpen([id, wdc = make_weak_ptr(dc)]() {
		APP_DBG("DataChannel Open !!\n");
		if (auto dc = wdc.lock()) {
			dc->send("Ping");
		}
	});

	dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg) {
		// APP_DBG("DataChannel On Message : [%s] : Received [%s]\n", id.c_str(), msg.c_str());
		if (auto dc = wdc.lock()) {
			dc->send("Ping");
		}
	});
	
	client->dataChannel = dc;

	pc->setLocalDescription();

	return  client;
}

shared_ptr<ClientTrackData> addVideo(const shared_ptr<PeerConnection> pc, const uint8_t payloadType, const uint32_t ssrc, const string cname, const string msid, const function<void (void)> onOpen) {
	auto video = Description::Video(cname);
	video.addH264Codec(payloadType);
	video.addSSRC(ssrc, cname, msid, cname);
	auto track = pc->addTrack(video);
	// create RTP configuration
	auto rtpConfig = make_shared<RtpPacketizationConfig>(ssrc, cname, payloadType, H264RtpPacketizer::defaultClockRate);
	// create packetizer
	auto packetizer = make_shared<H264RtpPacketizer>(NalUnit::Separator::Length, rtpConfig);
	// add RTCP SR handler
	auto srReporter = make_shared<RtcpSrReporter>(rtpConfig);
	packetizer->addToChain(srReporter);
	// add RTCP NACK handler
	auto nackResponder = make_shared<RtcpNackResponder>();
	packetizer->addToChain(nackResponder);
	// set handler
	track->setMediaHandler(packetizer);
	track->onOpen(onOpen);
	auto trackData = make_shared<ClientTrackData>(track, srReporter);
	return trackData;
}

shared_ptr<ClientTrackData> addAudio(const shared_ptr<PeerConnection> pc, const uint8_t payloadType, const uint32_t ssrc, const string cname, const string msid, const function<void (void)> onOpen) {
	auto audio = Description::Audio(cname);
	audio.addOpusCodec(payloadType);
	audio.addSSRC(ssrc, cname, msid, cname);
	auto track = pc->addTrack(audio);
	// create RTP configuration
	auto rtpConfig = make_shared<RtpPacketizationConfig>(ssrc, cname, payloadType, OpusRtpPacketizer::DefaultClockRate);
	// create packetizer
	auto packetizer = make_shared<OpusRtpPacketizer>(rtpConfig);
	// add RTCP SR handler
	auto srReporter = make_shared<RtcpSrReporter>(rtpConfig);
	packetizer->addToChain(srReporter);
	// add RTCP NACK handler
	auto nackResponder = make_shared<RtcpNackResponder>();
	packetizer->addToChain(nackResponder);
	// set handler
	track->setMediaHandler(packetizer);
	track->onOpen(onOpen);
	auto trackData = make_shared<ClientTrackData>(track, srReporter);
	return trackData;
}

/// Create stream
shared_ptr<Stream> createStream(const unsigned fps, const string opusSamples) {
	// video source
	auto video = make_shared<CaptureSource>(fps);
	// audio source
	auto audio = make_shared<OPUSFileParser>(opusSamples, true);

	auto stream = make_shared<Stream>(video, audio);
	// set callback responsible for sample sending
	/*
	stream->onSample([ws = make_weak_ptr(stream)](Stream::StreamSourceType type,
												uint64_t sampleTime,
												rtc::binary sample) {
		
		string streamType = type == Stream::StreamSourceType::Video ? "video" : "audio";
		APP_DBG("Stream Type : %s\r\n", streamType);
		// get all clients with Ready state
		for(auto id_client: clients) {
			auto id = id_client.first;
			auto client = id_client.second;
			auto optTrackData = (type == Stream::StreamSourceType::Video ? client->video : client->audio);
			if (client->getState() == Client::State::Ready && optTrackData.has_value()) {
				auto trackData = optTrackData.value();
				auto rtpConfig = trackData->sender->rtpConfig;

				// sample time is in us, we need to convert it to seconds
				auto elapsedSeconds = double(sampleTime) / (1000 * 1000);
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

				cout << "Sending " << streamType << " sample with size: " <<(sample.size()) << " to " << client << endl;
				try {
					// send sample
					trackData->track->send(sample);
				} catch (const std::exception &e) {
					cerr << "Unable to send "<< streamType << " packet: " << e.what() << endl;
				}
			}
		}
		if (clients.empty()) {
			// we have no clients, stop the stream
			if (auto stream = ws.lock()) {
				stream->stop();
			}
		}
	});
	*/
	return stream;
}

/// Start stream
void startStream() {
	shared_ptr<Stream> stream;
	APP_DBG("-----------%s---------\r\n", __func__);
	if (avStream.has_value()) {
		APP_DBG("Audio and Video have stream !\n");
		stream = avStream.value();
		if (stream->isRunning) {
			// stream is already running
			return;
		}
	} else {
		APP_DBG("create stream !\r\n");
		stream = createStream(30, OPUS_DIRECTORY);
		avStream = stream;
	}
	stream->start();
}

void addToStream(shared_ptr<Client> client, bool isAddingVideo) {
	APP_DBG("---------%s---------\r\n", __func__);
	if (client->getState() == Client::State::Waiting) {
		client->setState(isAddingVideo ? Client::State::WaitingForAudio : Client::State::WaitingForVideo);
	} else if ((client->getState() == Client::State::WaitingForAudio && !isAddingVideo)
			|| (client->getState() == Client::State::WaitingForVideo && isAddingVideo)) {

		// Audio and video tracks are collected now
		assert(client->video.has_value() && client->audio.has_value());
		auto video = client->video.value();

		if (avStream.has_value()) {
			// sendInitialNalus(avStream.value(), video);
		}

		client->setState(Client::State::Ready);
	}
	if (client->getState() == Client::State::Ready) {
		APP_DBG("START STREAM !\n");
		startStream();
	}
}
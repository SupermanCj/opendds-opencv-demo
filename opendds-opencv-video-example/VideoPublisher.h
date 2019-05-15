#pragma once

#include "VideoTypeSupportImpl.h"

#include <dds/DdsDcpsInfrastructureC.h>

#include <atomic>
#include <thread>

class VideoPublisher {
public:
	VideoPublisher(DDS::DomainParticipant_var p);
	VideoPublisher(DDS::DomainParticipant_var p, const char * topicName);
	VideoPublisher(DDS::DomainParticipant_var p, int videoCapture);
	VideoPublisher(DDS::DomainParticipant_var p, const char * topicName, int videoCapture);
	VideoPublisher(DDS::DomainParticipant_var p, int _from, const char * topicName, int videoCapture);
	~VideoPublisher();
	void playVideo();
	void stop();
	void _sendFrames();

private:
	
	void _frameRegister(const char * topicName);
	DDS::DataWriterQos _reliableQos();

	std::atomic_bool playing;
	std::thread t;
	int capId;
	DDS::Topic_var topic;
	DDS::DomainParticipant_var participant;
	DDS::Publisher_var publisher;
	Video::FrameDataWriter_var writer;

	int from;
};


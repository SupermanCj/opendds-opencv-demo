#pragma once

#include "VideoTypeSupportImpl.h"
#include "FrameReaderListenerImpl.h"

#include <dds/DdsDcpsInfrastructureC.h>
#include <ace/Global_Macros.h>
#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DCPS/LocalObject.h>
#include <dds/DCPS/Definitions.h>


class VideoSubscriber {
public:
	VideoSubscriber(DDS::DomainParticipant_var p, DDS::DataReaderListener_var& listener);
	VideoSubscriber(DDS::DomainParticipant_var p, DDS::DataReaderListener_var& listener, const char* topicName);
	void clear();

private:

	void _frameRegister(const char* topicName);
	DDS::DataReaderQos _reliableQos();

	DDS::DataReaderListener_var listener;
	DDS::Topic_var topic;
	DDS::DomainParticipant_var participant;
	DDS::Subscriber_var subscriber;
	Video::FrameDataReader_var reader;

};

#include "VideoSubscriber.h"
#include "utils.h"
#include "VideoTypeSupportImpl.h"

#include <ace/Log_Msg.h>
#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsPublicationC.h>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include "dds/DCPS/StaticIncludes.h"

using namespace DDS;
using namespace OpenDDS::DCPS;
using namespace Video;
using namespace std;

const char* DEFAULT_VIDEO_TOPIC = "Video";

VideoSubscriber::VideoSubscriber(DDS::DomainParticipant_var p, DDS::DataReaderListener_var & listener)
	:VideoSubscriber(p, listener, DEFAULT_VIDEO_TOPIC)
{
}

VideoSubscriber::VideoSubscriber(DDS::DomainParticipant_var p, DDS::DataReaderListener_var & _listener, const char * topicName)
	: listener(_listener)
{
	//	listener = _listener;
	participant = p;

	_frameRegister(topicName);

	subscriber = participant->create_subscriber(
		SUBSCRIBER_QOS_DEFAULT, 0, DEFAULT_STATUS_MASK
	);
	if (!subscriber) {
		throw string(" create_subscriber failed!\n");
	}

	DataReader_var _reader = subscriber->create_datareader(
		topic, DATAREADER_QOS_DEFAULT,
		listener, DEFAULT_STATUS_MASK
	);
	if (!_reader) {
		throw string(" create_datareader failed!\n");
	}
	reader = FrameDataReader::_narrow(_reader);
	if (!reader) {
		throw string(" _narrow failed!\n");
	}
}

void VideoSubscriber::clear()
{
	subscriber->delete_contained_entities();
	participant->delete_subscriber(subscriber.in());
}

void VideoSubscriber::_frameRegister(const char * topicName)
{
	FrameTypeSupport_var ts = new FrameTypeSupportImpl();
	if (ts->register_type(participant, "") != RETCODE_OK) {
		throw std::string("register_type failed!\n");
	}

	CORBA::String_var type_name = ts->get_type_name();
	topic =
		participant->create_topic(topicName,
			type_name,
			TOPIC_QOS_DEFAULT,
			0,
			DEFAULT_STATUS_MASK);
	if (!topic) {
		throw std::string("create topic fail!");
	}
}

DDS::DataReaderQos VideoSubscriber::_reliableQos()
{
	DataReaderQos dr_qos;
	subscriber->get_default_datareader_qos(dr_qos);
	dr_qos.reliability.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
	//dr_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
	dr_qos.reliability.max_blocking_time.sec = 1;
	dr_qos.reliability.max_blocking_time.nanosec = 1000 * 1000;
	dr_qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
	dr_qos.history.depth = 50;
	dr_qos.resource_limits.max_samples = 100;
	dr_qos.resource_limits.max_samples_per_instance = 50;
	return dr_qos;
}

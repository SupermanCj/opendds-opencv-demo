#include "VideoPublisher.h"
#include "utils.h"
#include "VideoTypeSupportImpl.h"

#include <ace/Log_Msg.h>
#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsPublicationC.h>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include "dds/DCPS/StaticIncludes.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include <atomic>
#include <iostream>
#include <thread>
#include <time.h>

using namespace DDS;
using namespace OpenDDS::DCPS;
using namespace Video;
using namespace std;


const char* DEFAULT_TOPIC_NAME = "Video";
const int DEFAULT_CAMERA = 0;
const int DEFAULT_FROM = 0XFFFFFFF;
const int DEFAULT_SEELP_MILLISECOND = 100;

VideoPublisher::VideoPublisher(DDS::DomainParticipant_var p)
	:VideoPublisher(p, DEFAULT_TOPIC_NAME, DEFAULT_CAMERA){}

VideoPublisher::VideoPublisher(DDS::DomainParticipant_var p, const char * topicName)
	: VideoPublisher(p, topicName, DEFAULT_CAMERA) {}

VideoPublisher::VideoPublisher(DDS::DomainParticipant_var p, int videoCapture)
	: VideoPublisher(p, DEFAULT_TOPIC_NAME, videoCapture) {}

VideoPublisher::VideoPublisher(DDS::DomainParticipant_var p, const char * topicName, int videoCapture)
	:VideoPublisher(p, DEFAULT_FROM, topicName, videoCapture){}

VideoPublisher::VideoPublisher(DDS::DomainParticipant_var p, int _from, const char * topicName, int videoCapture)
{
	cout << "start create p\n";
	participant = p;
	_frameRegister(topicName);
	
	capId = videoCapture;

	publisher =
		participant->create_publisher(PUBLISHER_QOS_DEFAULT, 0, DEFAULT_STATUS_MASK);
	if (!publisher) {
		throw std::string(" create_publisher failed!\n");
	}

	DDS::DataWriter_var _writer =
		publisher->create_datawriter(topic,
			_reliableQos(),
			0,
			OpenDDS::DCPS::DEFAULT_STATUS_MASK);

	if (!_writer) {
		throw std::string(" create_datawriter failed!\n");
	}
	
	writer = FrameDataWriter::_narrow(_writer);
	if (!writer) {
		throw string(" _narrow failed!\n");
	}
	
	if (_from == DEFAULT_FROM) {
		srand(unsigned(time(NULL)));
		from = rand() % 10000;
	}
	else {
		from = _from;
	}

	playing.store(false);
	cout << "stop create\n";
}

VideoPublisher::~VideoPublisher()
{
	if (playing) {
		stop();
	}
	cout << "destory\n";
}

void VideoPublisher::playVideo()
{
	if (playing.exchange(true)) {
		return;
	}
	std::cout << "play!" << endl;
	t = thread(&VideoPublisher::_sendFrames, this);
	t.detach();
}

void VideoPublisher::stop()
{
	playing.store(false);
	publisher->delete_contained_entities();
	participant->delete_publisher(publisher.in());
}

void VideoPublisher::_sendFrames()
{
	VideoCapture cap(capId);
	if (!cap.isOpened()) {
		throw string("fail to open VideoCapture!");
	}
	Frame data;
	data.video_id = from;
	InstanceHandle_t handle = writer->register_instance(data);
	char *temp = new char[10];
	sprintf(temp, "%d", from);
	data.from = temp;
	data.frame_id = 0;

	Mat frame;

	int count = 0;
	cout << "send from:"<< from << endl;
	while (playing) {
		cap >> frame;
		data.frame_id += 1;
		setMatToFrame(frame, data);

		ReturnCode_t error = writer->write(data, handle);
		if (error != RETCODE_OK) {
			cout << "writer return :" << error << endl;
				/*			
				ACE_ERROR((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" write returned %d!\n"), error));
				*/
		}
		count++;
		std::cout << "send:" << count << " size:" << data.cols*data.rows*data.channels << std::endl;
		msleep(DEFAULT_SEELP_MILLISECOND);
	}
	std::cout << "number of message sent :" << count << std::endl;
}

void VideoPublisher::_frameRegister(const char * topicName)
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

DataWriterQos VideoPublisher::_reliableQos() {
	DataWriterQos dw_qos;
	publisher->get_default_datawriter_qos(dw_qos);
	dw_qos.reliability.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
	//dw_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
	dw_qos.reliability.max_blocking_time.sec = 1;
	dw_qos.reliability.max_blocking_time.nanosec = 1000 * 1000;
	dw_qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
	dw_qos.history.depth = 50;
	dw_qos.resource_limits.max_samples = 100;
	dw_qos.resource_limits.max_samples_per_instance = 50;
	return dw_qos;
}

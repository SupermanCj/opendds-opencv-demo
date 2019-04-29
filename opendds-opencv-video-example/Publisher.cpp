
#include <ace/Log_Msg.h>
#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsPublicationC.h>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include "dds/DCPS/StaticIncludes.h"

#include <iostream>
#include "utils.h"
#include <thread>
#include <atomic>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include "OperationReaderListenerImpl.h"
#include "VideoTypeSupportImpl.h"



using namespace DDS;
using namespace Video;
using namespace OpenDDS::DCPS;
using namespace cv;

inline int OperationSubscribe(DomainParticipant_var &dp);
DomainParticipant_var create_participant(DomainParticipantFactory_var dpf, int domain_id);
Topic_var create_frame_topic(DomainParticipant_var p);
DataWriterQos getDWReliableQos(Publisher_var publisher);
void notifyInput(std::atomic_bool* );
void checkInput(std::atomic_bool* exist);

int main(int argc, char *argv[]) {
	try {
		DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);

		DomainParticipant_var participant = create_participant(dpf, 79);

		VideoCapture cap(0);
		if (!cap.isOpened())
		{
			std::cout << "Read video Failed !" << std::endl;
			return 1;
		}

		Topic_var topic = create_frame_topic(participant);

		//订阅操作
		OperationSubscribe(participant);
		

		Publisher_var publisher =
			participant->create_publisher(PUBLISHER_QOS_DEFAULT, 0, DEFAULT_STATUS_MASK);
		if (!publisher) {
			throw std::string(" create_publisher failed!\n");
		}

		DataWriter_var writer =
			publisher->create_datawriter(topic,
				getDWReliableQos(publisher),
				0,
				OpenDDS::DCPS::DEFAULT_STATUS_MASK);

			if (!writer) {
			throw std::string(" create_datawriter failed!\n");
		}


		FrameDataWriter_var frame_writer = FrameDataWriter::_narrow(writer);
		if (!frame_writer) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" _narrow failed!\n")),
				-1);
		}
		Frame frame;
		frame.video_id = 1;
		InstanceHandle_t handle = frame_writer->register_instance(frame);

		frame.from = "test";
		frame.frame_id = 1;
		Mat frame_;

		std::atomic_bool stop{ false };
		int count = 0;
		std::cout << "send!\n";
		notifyInput(&stop);
		while (!stop) {
			cap >> frame_;
			frame.frame_id += 1;
			setMatToFrame(frame_, frame);
			
			ReturnCode_t error = frame_writer->write(frame, handle);
			if (error != RETCODE_OK) {
				ACE_ERROR((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" write returned %d!\n"), error));
			}
			count++;
			std::cout<<"send:"<<count<<" size:"<<frame.cols*frame.rows*frame.channels<<std::endl;
			msleep(20);
		}
		std::cout << "number of message sent :" << count << std::endl;

		participant->delete_contained_entities();
		dpf->delete_participant(participant);

		TheServiceParticipant->shutdown();
	}
	catch (const CORBA::Exception& e) {
		e._tao_print_exception("Exception caught in main():");
		return -1;
	}
	return 0;
}

int OperationSubscribe(DomainParticipant_var &participant) {
	
	OperationTypeSupport_var ots = new OperationTypeSupportImpl();
	if (ots->register_type(participant, "") != RETCODE_OK) {
		ACE_ERROR_RETURN((LM_ERROR,
			ACE_TEXT("ERROR: %N:%l: main() -")
			ACE_TEXT(" register_type failed!\n")),
			-1);
	}

	CORBA::String_var type_name = ots->get_type_name();
	Topic_var otopic =
		participant->create_topic("Operation",
			type_name,
			TOPIC_QOS_DEFAULT,
			0,
			DEFAULT_STATUS_MASK);
	if (!otopic) {
		ACE_ERROR_RETURN((LM_ERROR,
			ACE_TEXT("ERROR: %N:%l: main() -")
			ACE_TEXT(" create_topic failed!\n")),
			-1);
	}

	Subscriber_var subscriber = participant->create_subscriber(
		SUBSCRIBER_QOS_DEFAULT, 0, DEFAULT_STATUS_MASK
	);
	if (!subscriber) {
		ACE_ERROR_RETURN((LM_ERROR,
			ACE_TEXT("ERROR: %N:%l: main() -")
			ACE_TEXT(" create_subscriber failed!\n")), -1);
	};
	std::cout << "create reader!\n";
	DDS::DataReaderListener_var listener(new OperationReaderListenerImpl);
	DataReader_var reader = subscriber->create_datareader(
		otopic, DATAREADER_QOS_DEFAULT,
		listener, DEFAULT_STATUS_MASK
	);
	if (!reader) {
		ACE_ERROR_RETURN((LM_ERROR,
			ACE_TEXT("ERROR: %N:%l: main() -")
			ACE_TEXT(" create_datareader failed!\n")), -1);
	}
	/*
	OperationDataReader_var reader_i = OperationDataReader::_narrow(reader);
	if (!reader_i) {
		ACE_ERROR_RETURN((LM_ERROR,
			ACE_TEXT("ERROR: %N:%l: main() -")
			ACE_TEXT(" _narrow failed!\n")),
			-1);
	}
	*/
	return 200;
};

DomainParticipant_var create_participant(DomainParticipantFactory_var dpf, int domain_id) {
	DomainParticipant_var participant = dpf->create_participant(
		domain_id, PARTICIPANT_QOS_DEFAULT, 0, DEFAULT_STATUS_MASK);
	if (!participant) {
		throw std::string("failed to create domain participant");
	}
	return participant;
}

Topic_var create_frame_topic(DomainParticipant_var p) {
	Topic_var topic;
	FrameTypeSupport_var ts = new FrameTypeSupportImpl();
	if (ts->register_type(p, "") != RETCODE_OK) {
		throw std::string("register_type failed!\n");
	}

	CORBA::String_var type_name = ts->get_type_name();
	topic =
		p->create_topic("Video",
			type_name,
			TOPIC_QOS_DEFAULT,
			0,
			DEFAULT_STATUS_MASK);
	if (!topic) {
		throw std::string("create topic fail!");
	}
	return topic;
}

DataWriterQos getDWReliableQos(Publisher_var publisher) {
	DataWriterQos dw_qos;
	publisher->get_default_datawriter_qos(dw_qos);
	dw_qos.reliability.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
	//dw_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
	dw_qos.reliability.max_blocking_time.sec = 1;
	dw_qos.reliability.max_blocking_time.nanosec = 1000*1000;
	dw_qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
	dw_qos.history.depth = 50;
	dw_qos.resource_limits.max_samples = 100;
	dw_qos.resource_limits.max_samples_per_instance = 50;
	return dw_qos;
}

void notifyInput(std::atomic_bool* exist) {
	std::thread t(checkInput, exist);
	t.detach();
}

void checkInput(std::atomic_bool* exist) {
	// std::cin.seekg(std::cin.end);
	std::cin.peek();
	std::cout<<std::cin.peek()<<std::endl;
	exist->store(true);
}

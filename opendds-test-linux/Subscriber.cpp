#include <ace/Log_Msg.h>

#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsSubscriptionC.h>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include <iostream>
#include "dds/DCPS/StaticIncludes.h"

#include "FrameReaderListenerImpl.h"
#include "VideoTypeSupportImpl.h"

using namespace DDS;
using namespace OpenDDS::DCPS;
using namespace Video;

DomainParticipant_var create_participant(DomainParticipantFactory_var dpf,int domain_id);
Topic_var create_frame_topic(DomainParticipant_var p);
DataReaderQos getDRReliableQos(Subscriber_var subscriber);
Topic_var create_operation_topic(DomainParticipant_var p);

int main(int argc, char *argv[]) {
	try {
		DomainParticipantFactory_var dpf =
			TheParticipantFactoryWithArgs(argc, argv);

		DomainParticipant_var participant = create_participant(dpf, 79);


		Topic_var frame_topic = create_frame_topic(participant);
		

		Subscriber_var subscriber = participant->create_subscriber(
			SUBSCRIBER_QOS_DEFAULT, 0, DEFAULT_STATUS_MASK
		);
		if (!subscriber) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" create_subscriber failed!\n")), -1);
		};
		std::cout << "create reader!\n";
		DDS::DataReaderListener_var listener(new FrameReaderListenerImpl);
		DataReader_var reader = subscriber->create_datareader(
			frame_topic, getDRReliableQos(subscriber),
			listener, DEFAULT_STATUS_MASK
		);
		if (!reader) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" create_datareader failed!\n")), -1);
		}

		FrameDataReader_var reader_i = FrameDataReader::_narrow(reader);
		if (!reader_i) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" _narrow failed!\n")),
				-1);
		}
		std::cout << "create reader end!\n";


		Topic_var operation_topic = create_operation_topic(participant);


		Publisher_var publisher =
			participant->create_publisher(PUBLISHER_QOS_DEFAULT, 0, DEFAULT_STATUS_MASK);
		if (!publisher) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" create_publisher failed!\n")),
				-1);
		}
		DataWriter_var writer =
			publisher->create_datawriter(operation_topic,
				DATAWRITER_QOS_DEFAULT,
				0,
				OpenDDS::DCPS::DEFAULT_STATUS_MASK);

		if (!writer) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" create_datawriter failed!\n")),
				-1);
		}
		OperationDataWriter_var operation_writer = OperationDataWriter::_narrow(writer);
		if (!operation_writer) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" _narrow failed!\n")),
				-1);
		}

		Operation operation;
		operation.operation_id = 111;
		operation.opera_seq_num = 1;
		operation.action = OperationAction::BACK;

		for (int i = 0; i < 100; i++) {
			sleep(1);
			operation.opera_seq_num = i;
			operation_writer->write(operation, 0);
			
		}


		std::cin.get();
		
		
		participant->delete_contained_entities();
		dpf->delete_participant(participant);

		TheServiceParticipant->shutdown();
	}catch (const CORBA::Exception& e) {
		e._tao_print_exception("Exception caught in main():");
		return -1;
	}
	return 0;
}

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
Topic_var create_operation_topic(DomainParticipant_var p) {
	Topic_var topic;
	OperationTypeSupport_var ts = new OperationTypeSupportImpl();
	if (ts->register_type(p, "") != RETCODE_OK) {
		throw std::string(" register_type failed!\n");
	}
	CORBA::String_var type_name = ts->get_type_name();
	topic =
		p->create_topic("Operation",
			type_name,
			TOPIC_QOS_DEFAULT,
			0,
			DEFAULT_STATUS_MASK);
	if (!topic) {
		throw std::string(" create_topic failed!\n");
	}
	return topic;
}

DataReaderQos getDRReliableQos(Subscriber_var subscriber) {
	DataReaderQos dr_qos;
	subscriber->get_default_datareader_qos(dr_qos);
	dr_qos.reliability.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
	//dr_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
	dr_qos.reliability.max_blocking_time.sec = 0;
	dr_qos.reliability.max_blocking_time.nanosec = 1000*1000;
	dr_qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
	dr_qos.history.depth = 100;
	dr_qos.resource_limits.max_samples = 200;
	dr_qos.resource_limits.max_samples_per_instance = 100;
	return dr_qos;
}


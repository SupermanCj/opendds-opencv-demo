#include "OperationSubscriber.h"
#include "utils.h"
#include "VideoTypeSupportImpl.h"

#include <ace/Log_Msg.h>
#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsPublicationC.h>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include "dds/DCPS/StaticIncludes.h"

#include <iostream>

using namespace DDS;
using namespace OpenDDS::DCPS;
using namespace Video;
using namespace std;

const char* DEFAULT_OPERATION_TOPIC = "Operation";

void DEFAULT_HANDLE(Video::Operation operation, DDS::SampleInfo info)
{
	cout << "id:" << operation.operations_id << endl
		<< "seq:" << operation.opera_seq_num << endl
		<< "action:" << operation.action << endl;
}

OperationSubscriber::OperationSubscriber(DDS::DomainParticipant_var p, int _OperationsId)
	:OperationSubscriber(p, _OperationsId, DEFAULT_HANDLE, DEFAULT_OPERATION_TOPIC){}

OperationSubscriber::OperationSubscriber(DDS::DomainParticipant_var p, int _OperationsId, const char * topicName)
	:OperationSubscriber(p, _OperationsId, DEFAULT_HANDLE, topicName){}

OperationSubscriber::OperationSubscriber(DDS::DomainParticipant_var p, int _OperationsId, Handle _handle, const char* topicName)
{
	participant = p;
	OperationsId = _OperationsId;
	handle = _handle;
	listener = new OperationReaderListenerImpl(handle);

	_operationRegister(topicName);
	subscriber = participant->create_subscriber(
		SUBSCRIBER_QOS_DEFAULT, 0, DEFAULT_STATUS_MASK
	);
	if (!subscriber) {
		throw string(" create_subscriber failed!\n");
	};
	DDS::DataReaderListener_var _listener(listener);
	DataReader_var _reader = subscriber->create_datareader(
		topic, DATAREADER_QOS_DEFAULT,
		_listener, DEFAULT_STATUS_MASK
	);
	if (!_reader) {
		throw string(" create_datareader failed!\n");
	}
	reader = OperationDataReader::_narrow(_reader);
	if (!reader) {
		throw string(" _narrow failed!\n");
	}
}

void OperationSubscriber::clear()
{
	subscriber->delete_contained_entities();
	participant->delete_subscriber(subscriber.in());
}


void OperationSubscriber::_operationRegister(const char * topicName)
{
	OperationTypeSupport_var ts = new OperationTypeSupportImpl();
	if (ts->register_type(participant, "") != RETCODE_OK) {
		throw std::string("register type:Operation failed!\n");
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


OperationSubscriber::OperationReaderListenerImpl::OperationReaderListenerImpl(Handle _handle)
{
	handle = _handle;
}

void OperationSubscriber::OperationReaderListenerImpl::on_requested_deadline_missed(::DDS::DataReader_ptr reader, const::DDS::RequestedDeadlineMissedStatus & status)
{

}

void OperationSubscriber::OperationReaderListenerImpl::on_requested_incompatible_qos(::DDS::DataReader_ptr reader, const::DDS::RequestedIncompatibleQosStatus & status)
{
	std::cout << "incompatible_qos" << endl;
}

void OperationSubscriber::OperationReaderListenerImpl::on_sample_rejected(::DDS::DataReader_ptr reader, const::DDS::SampleRejectedStatus & status)
{
	std::cout << "rejected" << endl;
}

void OperationSubscriber::OperationReaderListenerImpl::on_liveliness_changed(::DDS::DataReader_ptr reader, const::DDS::LivelinessChangedStatus & status)
{

}

void OperationSubscriber::OperationReaderListenerImpl::on_data_available(::DDS::DataReader_ptr reader)
{

	Video::OperationDataReader_var reader_v = Video::OperationDataReader::_narrow(reader);
	cout << "recv!\n";
	if (!reader_v) {
		ACE_ERROR((LM_ERROR,
			ACE_TEXT("ERROR: %N:%l: on_data_available() -")
			ACE_TEXT(" _narrow failed!\n")));
		ACE_OS::exit(-1);
	}

	Video::Operation operation;
	DDS::SampleInfo info;

	DDS::ReturnCode_t error = reader_v->take_next_sample(operation, info);
	if (error == DDS::RETCODE_OK) {
		handle(operation, info);
	}
	else {
		ACE_ERROR((LM_ERROR,
			ACE_TEXT("ERROR: %N:%l: on_data_available() -")
			ACE_TEXT(" take_next_sample failed!\n")));
	}
}

void OperationSubscriber::OperationReaderListenerImpl::on_subscription_matched(::DDS::DataReader_ptr reader, const::DDS::SubscriptionMatchedStatus & status)
{
	cout << "match or dismatch:"<<status.last_publication_handle << endl;
}

void OperationSubscriber::OperationReaderListenerImpl::on_sample_lost(::DDS::DataReader_ptr reader, const::DDS::SampleLostStatus & status)
{
	std::cout << "lost from last status change:" << status.total_count_change << " lost total:" << status.total_count << std::endl;
}


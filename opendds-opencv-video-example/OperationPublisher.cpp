#include "OperationPublisher.h"
#include "VideoTypeSupportImpl.h"

#include <ace/Log_Msg.h>
#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsPublicationC.h>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include "dds/DCPS/StaticIncludes.h"

#include <iostream>

using namespace Video;
using namespace DDS;
using namespace OpenDDS::DCPS;
using namespace std;

const char* DEFAULT_OPERATION_TOPIC = "Operation";

OperationPublisher::OperationPublisher(DDS::DomainParticipant_var p)
	:OperationPublisher(p, DEFAULT_OPERATION_TOPIC)
{
}

OperationPublisher::OperationPublisher(DDS::DomainParticipant_var p, const char * topicName)
{
	participant = p;

	_operationRegister(topicName);

	publisher =
		participant->create_publisher(PUBLISHER_QOS_DEFAULT, 0, DEFAULT_STATUS_MASK);
	if (!publisher) {
		throw std::string(" create_publisher failed!\n");
	}

	DDS::DataWriter_var _writer =
		publisher->create_datawriter(topic,
			DATAWRITER_QOS_DEFAULT,
			0,
			OpenDDS::DCPS::DEFAULT_STATUS_MASK);
	if (!_writer) {
		throw std::string(" create_datawriter failed!\n");
	}

	writer = OperationDataWriter::_narrow(_writer);
	if (!writer) {
		throw string(" _narrow failed!\n");
	}

}

bool OperationPublisher::publishOperation(Video::Operation operation)
{
	DDS::ReturnCode_t r = writer->write(operation, 0);
	if (r != RETCODE_OK) {
		return false;
	}
	return true;
}

void OperationPublisher::clear()
{
	publisher->delete_contained_entities();
	participant->delete_publisher(publisher.in());
}

void OperationPublisher::_operationRegister(const char * topicName)
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


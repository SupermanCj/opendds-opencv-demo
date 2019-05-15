#pragma once

#include "VideoTypeSupportImpl.h"

#include <dds/DdsDcpsInfrastructureC.h>

class OperationPublisher {
public:

	OperationPublisher(DDS::DomainParticipant_var p);
	OperationPublisher(DDS::DomainParticipant_var p, const char* topicName);

	bool publishOperation(Video::Operation operation);
	void clear();
private:

	void _operationRegister(const char * topicName);

	DDS::Topic_var topic;
	DDS::DomainParticipant_var participant;
	DDS::Publisher_var publisher;
	Video::OperationDataWriter_var writer;

};

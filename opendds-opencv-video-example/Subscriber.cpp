#include <ace/Log_Msg.h>

#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DCPS/Definitions.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include <iostream>
#include "dds/DCPS/StaticIncludes.h"

#include "FrameReaderListenerImpl.h"
#include "VideoSubscriber.h"
#include "VideoTypeSupportImpl.h"
#include "utils.h"

using namespace DDS;
using namespace OpenDDS::DCPS;
using namespace Video;

DomainParticipant_var create_participant(DomainParticipantFactory_var dpf,int domain_id);

int main(int argc, char *argv[]) {
	try {
		DomainParticipantFactory_var dpf =
			TheParticipantFactoryWithArgs(argc, argv);

		DomainParticipant_var participant = create_participant(dpf, 79);

		OperationPublisher operation(participant);
		FrameReaderListenerImpl listener(&operation);
		
		VideoSubscriber subscriber(participant, DDS::DataReaderListener_var(&listener));
		
		std::cin.get();
		std::cin.get();

		subscriber.clear();

		//participant->delete_contained_entities();
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





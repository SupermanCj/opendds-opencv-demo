
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

#include "VideoTypeSupportImpl.h"
#include "VideoPublisher.h"
#include "OperationSubscriber.h"


using namespace DDS;
using namespace Video;
using namespace OpenDDS::DCPS;

DomainParticipant_var create_participant(DomainParticipantFactory_var dpf, int domain_id);

int main(int argc, char *argv[]) {
	try {
		DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);

		DomainParticipant_var participant = create_participant(dpf, 79);
		VideoPublisher* publisher = new VideoPublisher(participant, 0);
		OperationSubscriber subscriber (participant, 1);
		publisher->playVideo();

		std::cin.get();
		std::cin.get();

		delete publisher;
		subscriber.clear();
		dpf->delete_participant(participant);

		TheServiceParticipant->shutdown();
		system("pause");
	}
	catch (const CORBA::Exception& e) {
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

#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdlib.h>

#include "OperationReaderListenerImpl.h"
#include "VideoTypeSupportC.h"
#include "VideoTypeSupportImpl.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include "utils.h"
using namespace cv;
using namespace std;


void OperationReaderListenerImpl::on_requested_deadline_missed(::DDS::DataReader_ptr reader, const::DDS::RequestedDeadlineMissedStatus & status)
{

}

void OperationReaderListenerImpl::on_requested_incompatible_qos(::DDS::DataReader_ptr reader, const::DDS::RequestedIncompatibleQosStatus & status)
{
	std::cout << "incompatible_qos" << endl;
}

void OperationReaderListenerImpl::on_sample_rejected(::DDS::DataReader_ptr reader, const::DDS::SampleRejectedStatus & status)
{
	std::cout << "rejected" << endl;
}

void OperationReaderListenerImpl::on_liveliness_changed(::DDS::DataReader_ptr reader, const::DDS::LivelinessChangedStatus & status)
{

}

void OperationReaderListenerImpl::on_data_available(::DDS::DataReader_ptr reader)
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
		cout << "id:" << operation.operation_id << endl
			<< "seq:" << operation.opera_seq_num << endl
			<< "action:" << operation.action << endl;
		
	}
	else {
		ACE_ERROR((LM_ERROR,
			ACE_TEXT("ERROR: %N:%l: on_data_available() -")
			ACE_TEXT(" take_next_sample failed!\n")));
	}
}

void OperationReaderListenerImpl::on_subscription_matched(::DDS::DataReader_ptr reader, const::DDS::SubscriptionMatchedStatus & status)
{
	std::cout << "match!" << endl;
}

void OperationReaderListenerImpl::on_sample_lost(::DDS::DataReader_ptr reader, const::DDS::SampleLostStatus & status)
{
	std::cout << "lost from last status change:" << status.total_count_change << " lost total:" << status.total_count << std::endl;
}

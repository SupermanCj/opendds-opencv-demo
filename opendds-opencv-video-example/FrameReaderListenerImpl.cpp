#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdlib.h>

#include "FrameReaderListenerImpl.h"
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
using namespace Video;

FrameReaderListenerImpl::FrameReaderListenerImpl(OperationPublisher* _writer)
{
	writer = _writer;
	round_count.store(0);
	cur_count.store(0);
}

FrameReaderListenerImpl::~FrameReaderListenerImpl() {
	writer->clear();
}

void FrameReaderListenerImpl::on_requested_deadline_missed(::DDS::DataReader_ptr reader, const::DDS::RequestedDeadlineMissedStatus & status)
{
	std::cout << "requested_deadline_missed" << endl;
}

void FrameReaderListenerImpl::on_requested_incompatible_qos(::DDS::DataReader_ptr reader, const::DDS::RequestedIncompatibleQosStatus & status)
{
	std::cout << "incompatible_qos" << endl;
}

void FrameReaderListenerImpl::on_sample_rejected(::DDS::DataReader_ptr reader, const::DDS::SampleRejectedStatus & status)
{
	std::cout << "rejected" << endl;
}

void FrameReaderListenerImpl::on_liveliness_changed(::DDS::DataReader_ptr reader, const::DDS::LivelinessChangedStatus & status)
{
	
}

void FrameReaderListenerImpl::on_data_available(::DDS::DataReader_ptr reader)
{
	Video::FrameDataReader_var reader_v = Video::FrameDataReader::_narrow(reader);
	if (!reader_v) {
		ACE_ERROR((LM_ERROR,
			ACE_TEXT("ERROR: %N:%l: on_data_available() -")
			ACE_TEXT(" _narrow failed!\n")));
		ACE_OS::exit(-1);
	}

	Video::Frame frame;
	DDS::SampleInfo info;

	DDS::ReturnCode_t error = reader_v->take_next_sample(frame, info);
	if (error == DDS::RETCODE_OK) {
		cur_count.fetch_add(1);
		std::cout << "SampleInfo.sample_rank = " << info.sample_rank << std::endl;
		std::cout << "SampleInfo.instance_state = " << info.instance_state << std::endl;
		std::cout << "instance_handle = " << info.instance_handle << std::endl;
		if (info.valid_data) {
			std::cout << "frame_id		= " << frame.frame_id << std::endl
				<< "from			= " << frame.from << std::endl;
			imshow(frame.from.in(), getMatFromFrame(frame));
			waitKey(5);
		}
		if (cur_count == 10) {
			round_count.fetch_add(1);
			cur_count.store(0);
			Operation operation;
			operation.operations_id = frame.video_id;
			operation.opera_seq_num = round_count * 10 + cur_count;
			operation.action = rand();
			writer->publishOperation(operation);
		}
	}else {
		ACE_ERROR((LM_ERROR,
			ACE_TEXT("ERROR: %N:%l: on_data_available() -")
			ACE_TEXT(" take_next_sample failed!\n")));
	}
}

void FrameReaderListenerImpl::on_subscription_matched(::DDS::DataReader_ptr reader, const::DDS::SubscriptionMatchedStatus & status)
{
	std::cout << "match or dismatch!" << endl;
}

void FrameReaderListenerImpl::on_sample_lost(::DDS::DataReader_ptr reader, const::DDS::SampleLostStatus & status)
{
	std::cout << "lost from last status change:" << status.total_count_change << " lost total:" << status.total_count << std::endl;
}

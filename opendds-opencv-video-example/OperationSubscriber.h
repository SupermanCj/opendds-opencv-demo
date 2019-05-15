#pragma once

#include "VideoTypeSupportImpl.h"

#include <dds/DdsDcpsInfrastructureC.h>
#include <ace/Global_Macros.h>
#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DCPS/LocalObject.h>
#include <dds/DCPS/Definitions.h>

typedef void(*Handle)(Video::Operation operation, DDS::SampleInfo);
void DEFAULT_HANDLE(Video::Operation operation, DDS::SampleInfo info);

class OperationSubscriber {
public:
	OperationSubscriber(DDS::DomainParticipant_var p, int _OperationsId);
	OperationSubscriber(DDS::DomainParticipant_var p, int _OperationsId, const char* topicName);
	OperationSubscriber(DDS::DomainParticipant_var p, int _OperationsId, Handle _handle, const char* topicName);
	void clear();

	class OperationReaderListenerImpl
		: public virtual OpenDDS::DCPS::LocalObject<DDS::DataReaderListener> {
	public:
		OperationReaderListenerImpl(Handle _handle);
		virtual void on_requested_deadline_missed(::DDS::DataReader_ptr reader, const::DDS::RequestedDeadlineMissedStatus & status) override;

		virtual void on_requested_incompatible_qos(::DDS::DataReader_ptr reader, const::DDS::RequestedIncompatibleQosStatus & status) override;

		virtual void on_sample_rejected(::DDS::DataReader_ptr reader, const::DDS::SampleRejectedStatus & status) override;

		virtual void on_liveliness_changed(::DDS::DataReader_ptr reader, const::DDS::LivelinessChangedStatus & status) override;

		virtual void on_data_available(::DDS::DataReader_ptr reader) override;

		virtual void on_subscription_matched(::DDS::DataReader_ptr reader, const::DDS::SubscriptionMatchedStatus & status) override;

		virtual void on_sample_lost(::DDS::DataReader_ptr reader, const::DDS::SampleLostStatus & status) override;
	private:
		Handle handle;
	};

private:

	void _operationRegister(const char* topicName);

	Handle handle;
	int OperationsId;
	OperationReaderListenerImpl* listener;
	DDS::Topic_var topic;
	DDS::DomainParticipant_var participant;
	DDS::Subscriber_var subscriber;
	Video::OperationDataReader_var reader;

};




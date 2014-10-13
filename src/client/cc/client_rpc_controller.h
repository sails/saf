#ifndef _CLIENT_RPC_CONTROLLER_H_
#define _CLIENT_RPC_CONTROLLER_H_

#include <iostream>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>


namespace sails {

class RpcControllerImp : public google::protobuf::RpcController
{
	void SetFailed(const std::string & reason);
	bool IsCanceled() const;
	void NotifyOnCancel(google::protobuf::Closure * callback);
	void Reset();
	bool Failed() const;
	std::string ErrorText() const;
	void StartCancel();
};


} // namespace sails



#endif /* _CLIENT_RPC_CONTROLLER_H_ */

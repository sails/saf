#include "client_rpc_controller.h"
#include <iostream>
#include <google/protobuf/descriptor.h>


using namespace std;
using namespace google::protobuf;


namespace sails {


void RpcControllerImp::SetFailed(const string &reason) {
		
}

bool RpcControllerImp::IsCanceled() const {
	return false;
}

void RpcControllerImp::NotifyOnCancel(google::protobuf::Closure *callback) {
	
}

void RpcControllerImp::Reset() {
	
}

bool RpcControllerImp::Failed() const {
	
}

string RpcControllerImp::ErrorText() const {
	return "error";
}

void RpcControllerImp::StartCancel() {
	
}


} // namespace sails

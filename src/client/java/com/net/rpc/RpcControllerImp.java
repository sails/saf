package com.net.rpc;

import com.google.protobuf.RpcCallback;
import com.google.protobuf.RpcController;

public class RpcControllerImp implements RpcController {

	@Override
	public String errorText() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean failed() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean isCanceled() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void notifyOnCancel(RpcCallback<Object> arg0) {
		// TODO Auto-generated method stub

	}

	@Override
	public void reset() {
		// TODO Auto-generated method stub

	}

	@Override
	public void setFailed(String arg0) {
		// TODO Auto-generated method stub

	}

	@Override
	public void startCancel() {
		// TODO Auto-generated method stub

	}

}

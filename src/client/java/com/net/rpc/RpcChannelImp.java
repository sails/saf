package com.net.rpc;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;

import com.google.protobuf.BlockingRpcChannel;
import com.google.protobuf.Descriptors.MethodDescriptor;
import com.google.protobuf.Message;
import com.google.protobuf.RpcController;

public class RpcChannelImp implements BlockingRpcChannel {
	
	public boolean connect(String ip, int port) {
		try {
			socket = new Socket();
			SocketAddress socketAddress = new InetSocketAddress(ip, port);
			socket.connect(socketAddress, 3000); // 连接超时3s
			inputStream = socket.getInputStream();
			outputStream = socket.getOutputStream();
			if (inputStream != null && outputStream != null) {
				socket.setSoTimeout(5000); // 5s read timeout
				return true;
			}else {
				return false;
			}
			
		}catch(Exception e) {
			e.printStackTrace();
		}
		return false;
	}
	
	@Override
	public Message callBlockingMethod(MethodDescriptor method, RpcController controller,
			Message request, Message response) {
		// TODO Auto-generated method stub
		response = this.syncCall(method, controller, request, response);
		return response;
	}
	
	Message syncCall(MethodDescriptor method, RpcController controller,
			Message request, Message response) {
		
		try {
			String service_name = method.getService().getName();
			String method_name = method.getName();
			byte[] content = request.toByteArray();
			
			// set packet
			RpcPackets packet = new RpcPackets();
			
			packet.setType(RpcPackets.PACKET_PROTOBUF_CALL);
			packet.setService_name(service_name);
			packet.setMethod_name(method_name);
			packet.setMethod_index(method.getIndex());
			packet.setData(content);
			
			byte[] data = new byte[packet.getPacketSize()];
			packet.toByteArray(data, 0);
			
			try {
				outputStream.write(data,0, data.length);
				
				// read data
				byte[] readBuf = new byte[2048];
				int off = 0;
				int readlen = 0;
				boolean isRun = true;
				while (isRun) {
					try {
						readlen = inputStream.read(readBuf, off, 1024);
					}catch(SocketTimeoutException e) { // ���ö���ʱ��ֹ�̲߳�����ֹ
						e.printStackTrace();
						return null;
					}catch(Exception e) {
						e.printStackTrace();
						return null;
					}
					if (readlen == 0) {
						continue;
					}else if(readlen == -1) {
						break;
					}
					
					
					if(readBuf[0] == RpcPackets.PACKET_PROTOBUF_RET) {
						off+=readlen;
						int len = ByteConvertUtil.byteArray2int(readBuf, 1);
						if (len > off) {
							continue;
						}
						// else data recv complete
						isRun = false;
						RpcPackets result = RpcPackets.parser(readBuf, 0);
						if (result != null) {
							byte[] body = result.getData();
							if(body != null && body.length > 0) {
								response = response.getParserForType().parseFrom(body);
								return response;
							}
						}else {
							return null;
						}
					}else {
						off = 0;
					}
				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
		}catch(Exception e) {
			e.printStackTrace();
		}
		
		
		return null;
	}
	
	public void close() {
		try {
			this.socket.close();
			inputStream.close();
			outputStream.close();
			
			this.socket = null;
			inputStream = null;
			outputStream = null;
		}catch(Exception e) {
			e.printStackTrace();
		}
	}
	private Socket socket = null;
	private InputStream inputStream = null;
	private OutputStream outputStream = null;
}

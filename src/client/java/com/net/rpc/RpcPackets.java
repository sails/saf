package com.net.rpc;


public class RpcPackets {
	public static byte PACKET_HEARTBEAT = 1;                 // heartbeat
	public static byte PACKET_EXCEPTION = 2;                 // exception
	public static byte PACKET_PROTOBUF_CALL = 3;             // protobuf
	public static byte PACKET_PROTOBUF_RET = 4;              // protobuf
	public static byte PACKET_MAX = 5; 
	
	
	private byte type;
	private int len;
	private String service_name = null;
	private String method_name = null;
	private int method_index;
	private byte[] data = null;
	
	public byte getType() {
		return type;
	}
	public void setType(byte type) {
		this.type = type;
	}
	
	public int getPacketSize() {
		return 1+4+50+50+4+data.length;
	}
	public String getService_name() {
		return service_name;
	}
	public void setService_name(String service_name) {
		this.service_name = service_name;
	}
	public String getMethod_name() {
		return method_name;
	}
	public void setMethod_name(String method_name) {
		this.method_name = method_name;
	}
	public int getMethod_index() {
		return method_index;
	}
	public void setMethod_index(int method_index) {
		this.method_index = method_index;
	}
	public byte[] getData() {
		return data;
	}
	public void setData(byte[] data) {
		this.data = data;
	}
	
	public int toByteArray(byte[] bytes, int offset) {
		try {
			int dataLen = 1+4+50+50+4+data.length;
			this.len = dataLen;
			
			if (bytes.length - offset < dataLen) {
				return 0;
			}
			
			bytes[offset] = this.type;
			offset += 1;
			
			ByteConvertUtil.int2byteArray(this.len, bytes, offset);
			offset += 4;
			
			if(service_name.length() > 0) {
				byte[] service_str = new byte[this.service_name.length()];
				ByteConvertUtil.String2byteArray(this.service_name, service_str);
				for(int i = 0; i < service_str.length; i++) {
					bytes[offset+i] = service_str[i];
				}
			}
			
			offset += 50;
			
			if (this.method_name.length() > 0) {
				byte[] method_str = new byte[this.method_name.length()];
				ByteConvertUtil.String2byteArray(this.method_name, method_str);
				for(int i = 0; i < method_str.length; i++) {
					bytes[offset+i] = method_str[i];
				}
			}
			offset += 50;
			
			ByteConvertUtil.int2byteArray(this.method_index, bytes, offset);
			offset += 4;
			
			for(int i = 0; i < data.length; i++) {
				bytes[offset+i] = data[i];
			}
			return dataLen;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return 0;
	}
	
	public static RpcPackets parser(byte[] bytes, int offset) {
		try {
			byte type = bytes[offset];
			offset += 1;
			if (type == PACKET_PROTOBUF_RET) {
				int len = ByteConvertUtil.byteArray2int(bytes, offset);
				if (len > bytes.length - (offset-1)) {
					return null;
				}
				offset += 4;
				RpcPackets packet = new RpcPackets();
				packet.type = type;
				packet.len = len;
				
				packet.service_name = ByteConvertUtil.byteArray2String(bytes, offset, 50);
				offset += 50;
				
				packet.method_name = ByteConvertUtil.byteArray2String(bytes, offset, 50);
				offset += 50;
				
				packet.method_index = ByteConvertUtil.byteArray2int(bytes, offset);
				offset += 4;
				
				int datalen = len-1-4-50-50-4;
				packet.data = new byte[datalen];
				for(int i = 0; i < datalen; i++) {
					packet.data[i] = bytes[offset+i];
				}
				offset += datalen;
				
				return packet;
			}
		}catch(Exception e) {
			e.printStackTrace();
		}
		return null;
	}
	
}

#include "server.h"
#include "handle_rpc.h"

namespace sails {

sails::log::Logger serverlog(log::Logger::LOG_LEVEL_DEBUG,
				  "./log/server.log", log::Logger::SPLIT_DAY);

Server::Server(int netThreadNum) :
    sails::net::EpollServer<net::PacketCommon>(netThreadNum){
    
    // 得到配置的模块
    config.get_modules(&modules_name);
    // 注册模块
    std::map<std::string, std::string>::iterator iter;
    for(iter = modules_name.begin(); iter != modules_name.end()
	    ; iter++) {
	if(!iter->second.empty()) {
	    moduleLoad.load(iter->second);
	}
    }     

}


net::PacketCommon* Server::parse(
    std::shared_ptr<sails::net::Connector> connector) {

    if (connector->readable() < sizeof(net::PacketCommon)) {
	return NULL;
    }
    net::PacketCommon *packet = (net::PacketCommon*)connector->peek();
    if (packet->type.opcode >= net::PACKET_MAX
	|| packet->type.opcode <= net::PACKET_MIN) { // error, and empty all data
	connector->retrieve(connector->readable());
	return NULL;
    }
    if (packet != NULL) {
	int packetlen = packet->len;
	if (packetlen < sizeof(net::PacketCommon)) {
	    return NULL;
	}
	if (packetlen > PACKET_MAX_LEN) {
	    connector->retrieve(packetlen);
	    char errormsg[100] = {'\0'};
	    sprintf(errormsg, "receive a invalid packet len:%d", packetlen);
	    perror(errormsg);
	    
	}
	if(connector->readable() >= packetlen) {
	    net::PacketCommon *item = (net::PacketCommon*)malloc(packetlen);
	    if (item == NULL) {
		char errormsg[100] = {'\0'};
		sprintf(errormsg, "malloc failed due to copy receive data to a common packet len:%d", packetlen);
		perror(errormsg);
		return NULL;
	    }
	    memset(item, 0, packetlen);
	    memcpy(item, packet, packetlen);
	    connector->retrieve(packetlen);

	    return item;
	}
    }
    
    return NULL;
}

Server::~Server() {
    modules_name.clear();
    moduleLoad.unload();
}











HandleImpl::HandleImpl(
    sails::net::EpollServer<sails::net::PacketCommon>* server)
    :sails::net::HandleThread<sails::net::PacketCommon>(server) {
    
}


void HandleImpl::handle(
    const sails::net::TagRecvData<net::PacketCommon> &recvData) {
    
    
    net::PacketCommon *request = recvData.data;

    net::ResponseContent content;
    memset(&content, 0, sizeof(net::ResponseContent));

    base::HandleChain<net::PacketCommon*, 
		    net::ResponseContent*> handle_chain;
    HandleRPC proto_decode;
    handle_chain.add_handle(&proto_decode);

    handle_chain.do_handle(request, &content);

    if (content.len > 0 && content.data != NULL) {
	int response_len = sizeof(net::PacketRPC)+content.len-1;
	net::PacketRPC *response = (net::PacketRPC*)malloc(response_len);
	memset(response, 0, response_len);
	response->common.type.opcode = net::PACKET_PROTOBUF_RET;
	response->common.len = response_len;
	memcpy(response->data, content.data, content.len);
	
	std::string buffer = std::string((char *)response, response_len);
	server->send(buffer, recvData.ip, recvData.port, recvData.uid, recvData.fd);

	free(response);
    }

}



} // namespace sails

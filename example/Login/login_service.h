#ifndef _LOGIN_SERVICE_H_
#define _LOGIN_SERVICE_H_

#include "login.pb.h"

namespace sails {

class LoginServiceImpl : public LoginService {
    virtual void login(::google::protobuf::RpcController* controller,
                       const ::sails::LoginRequest* request,
                       ::sails::LoginResponse* response,
		       ::google::protobuf::Closure* done);
    virtual void logout(::google::protobuf::RpcController* controller,
			const ::sails::LogoutRequest* request,
			::sails::LogoutResponse* response,
			::google::protobuf::Closure* done);
};


struct ptr_string {
  char *ptr;
  size_t len;
};

size_t read_callback(void *buffer, size_t size, size_t nmemb, void *userp);

bool post_message(const char* url, const char* data, std::string &result);

bool login_check(const LoginRequest *request);

std::string get_session();

typedef struct RoomInfo {
    char ip[20];
    int port;
    int usercount;
} RoomInfo;

bool get_room_info(int roomid, RoomInfo *info);

bool report_session(std::string session, std::string user, RoomInfo* room);

bool delete_session(std::string session);


} // namespace

#endif /* _LOGIN_SERVICE_H_ */

#include "login_service.h"
#include <curl/curl.h>
#include <sails/base/time_t.h>
#include <sails/crypto/md5.h>
#include <sails/base/string.h>
#include <json/json.h>
#include <sails/log/logging.h>
#include <string>
#include "login_config.h"
#include "sails/base/util.h"

namespace sails {

log::Logger loginLog(log::Logger::LOG_LEVEL_DEBUG,
                     "./log/login.log", log::Logger::SPLIT_DAY);

LoginConfig config;



void LoginServiceImpl::login(::google::protobuf::RpcController* controller,
                             const ::sails::LoginRequest* request,
                             ::sails::LoginResponse* response,
                             ::google::protobuf::Closure* done) {
  if (request != NULL && response != NULL) {
    LoginResponse_ResultCode result_code =
        LoginResponse_ResultCode::LoginResponse_ResultCode_LOGIN_OTHER_ERR;
    if (login_check(request)) {  // check username and password
      // get room info
      int roomid = request->roomid();
      response->set_roomid(request->roomid());
      if (roomid > 0 && roomid < 1000) {
        RoomInfo room_info;
        memset(&room_info, 0, sizeof(RoomInfo));
        if ( get_room_info(roomid, &room_info) ) {
          response->set_ip(room_info.ip);
          response->set_port(room_info.port);
          if (room_info.usercount >= config.max_player()) {
            result_code = LoginResponse_ResultCode_LOGIN_ROOM_FULL;
          } else {
            // post session
            std::string session = get_session();
            response->set_session(session);

            if (report_session(session, request->username(), &room_info)) {
              result_code = LoginResponse_ResultCode_LOGIN_SUCCESS;
            } else {
              result_code = LoginResponse_ResultCode_LOGIN_ADD_SESSION_FAIL;
            }
          }
        } else {
          result_code = LoginResponse_ResultCode_LOGIN_GET_ROOM_FAIL;
        }
      } else {
        loginLog.error("roomid is %d and invalid", roomid);
      }
    } else {
      result_code = LoginResponse_ResultCode_LOGIN_CHECK_FAIL;
    }

    response->set_code(result_code);
  }
}





void LoginServiceImpl::logout(::google::protobuf::RpcController* controller,
                              const ::sails::LogoutRequest* request,
                              ::sails::LogoutResponse* response,
                              ::google::protobuf::Closure* done) {
  if (request != NULL && response != NULL) {
    std::string session = request->session();
    if (session.length() > 0) {
      if (delete_session(session)) {
        response->set_code(LogoutResponse_ResultCode_LOGOUT_SUCCESS);
        return;
      }
    }
  }
  response->set_code(LogoutResponse_ResultCode_LOGOUT_ERR);
}




size_t read_callback(void *buffer, size_t size, size_t nmemb, void *userp) {
  struct ptr_string *data = (struct ptr_string*)userp;
  size_t new_len = data->len + size*nmemb;
  char *ptr = reinterpret_cast<char*>(malloc(new_len+1));
  memcpy(ptr, data->ptr, data->len);
  memcpy(ptr+data->len, buffer, size*nmemb);
  free(data->ptr);
  data->ptr = ptr;
  data->len = new_len;
  data->ptr[new_len] = '\0';
  return size*nmemb;
}

bool login_check(const LoginRequest *request) {
  char signstr[500] = {'\0'};
  std::string strsignMd5;
  char time_str[100] = {'\0'};
  memset(time_str, 0, 100);
  base::TimeT::time_str(time_str, 100);
  snprintf(signstr, sizeof(signstr), "%s%s%s%s",
          request->username().c_str(), request->ticket().c_str(),
           time_str, config.get_login_key().c_str());
  strsignMd5 = crypto::MD5(signstr).toString();

  std::string password(request->ticket());
  char password_encoding[1000];
  memset(password_encoding, '\0', 1000);

  std::string url;
  if (config.is_test()) {
    url = config.get_login_test_url();
  } else {
    url = config.get_login_url();
  }
  char param[500] = {'\0'};
  if (config.is_test()) {
    snprintf(param, sizeof(param), "model=netgame&action=authuser&uid=%s&"
            "ticket=%s&time=%s&sign=%s&line=10",
            request->username().c_str(),
             base::url_encode(password.c_str(), password_encoding,
                              sizeof(password_encoding)),
            time_str, strsignMd5.c_str());

  } else {
    snprintf(param, sizeof(param), "model=netgame&action=authuser&uid=%s&"
             "ticket=%s&time=%s&sign=%s",
             request->username().c_str(),
             base::url_encode(password.c_str(), password_encoding,
                              sizeof(password_encoding)),
             time_str, strsignMd5.c_str());
  }

  std::string result;
  if ( post_message(url.c_str(), param, result) ) {
    // parser json data
    Json::Reader reader;
    Json::Value root;
    if (reader.parse(result, root)) {
      if ( !root["status"].empty() ) {
        if (root["status"].asInt() == 1) {
          return true;
        }
      }
    }
  }
  return false;
}

std::string get_session() {
  // gen session
  char temp_str[100] = {'\0'};
  snprintf(temp_str, sizeof(temp_str), "%llu", sails::base::GetUID());
  std::string session = crypto::MD5(temp_str).toString();

  return session;
}


bool get_room_info(int roomid, RoomInfo *info) {
  std::string url = config.get_store_api_url()+"/room";
  char param[40] = {'\0'};
  snprintf(param, sizeof(param), "method=CHECK&id=%d", roomid);
  std::string result;

  if ( post_message(url.c_str(), param, result) ) {
    loginLog.debug("get room infor:%s", result.c_str());
    Json::Reader reader;
    Json::Value root;
    if (reader.parse(result, root)) {
      int status = root["status"].asInt();
      if (status == 0) {  // call right

        if (!root["message"]["ip"].empty() &&
            !root["message"]["port"].empty() &&
            !root["message"]["usercount"].empty() ) {
          std::string ip = root["message"]["ip"].asString();
          strncpy(info->ip, ip.c_str(), ip.length());
          loginLog.debug("ip:%s", ip.c_str());

          int port = atoi(root["message"]["port"].asString().c_str());
          info->port = port;
          loginLog.debug("port:%d", port);

          int usercount = atoi(root["message"]["usercount"].asString().c_str());
          info->usercount = usercount;
          loginLog.debug("usercount:%d", usercount);

          return true;
        }
      }
    }
  }
  return false;
}

bool report_session(std::string session, std::string user, RoomInfo* room) {
  std::string url = config.get_store_api_url()+"/session";
  char param[200] = {'\0'};
  snprintf(param, sizeof(param),
           "method=ADD&ip=%s&port=%d&session=%s&username=%s",
          room->ip, room->port, session.c_str(), user.c_str());
  std::string result;

  if (post_message(url.c_str(), param, result)) {
    loginLog.debug("report session result:%s", result.c_str());
    Json::Reader reader;
    Json::Value root;
    if (reader.parse(result, root)) {
      int status = root["status"].asInt();
      if (status == 0) {  // call right
        return true;
      }
    }
  }
  return false;
}

bool delete_session(std::string session) {
  std::string url = config.get_store_api_url()+"/session";
  char param[200] = {'\0'};
  snprintf(param, sizeof(param), "method=REMOVE&session=%s", session.c_str());
  std::string result;

  if (post_message(url.c_str(), param, result)) {
    loginLog.debug("delete session result:%s", result.c_str());
    Json::Reader reader;
    Json::Value root;
    if (reader.parse(result, root)) {
      int status = root["status"].asInt();
      if (status == 0) {  // call right
        return true;
      }
    }
  }
  return false;
}



bool post_message(const char* url, const char* data, std::string &result) {
  bool ret = false;
  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();
  if (curl != NULL) {
    struct ptr_string login_result;
    login_result.len = 0;
    login_result.ptr = NULL;


    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &login_result);

    CURLcode res;
    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
      result += std::string(login_result.ptr, login_result.len);
      ret = true;
    } else {
      loginLog.error("post url:%s", url);
    }

    if (login_result.len > 0 && login_result.ptr != NULL) {
      login_result.len = 0;
      free(login_result.ptr);
      login_result.ptr = NULL;
    }
  }

  curl_easy_cleanup(curl);
  curl_global_cleanup();
  return ret;
}

}  // namespace sails

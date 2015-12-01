#ifndef _LOGIN_CONFIG_H_
#define _LOGIN_CONFIG_H_

#include <map>
#include <string>
#include "sails/base/json.hpp"

using json = nlohmann::json;

namespace sails {

class LoginConfig
{
 public:
  LoginConfig();
  std::string get_login_url();
  std::string get_login_test_url();
  bool is_test();
  std::string get_login_key();
  std::string get_store_api_url();
  int max_player();
 private:
  json root;
};


} // namespace sails
#endif /* _LOGIN_CONFIG_H_ */

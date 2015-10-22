<?php
/*测试，先编译php_safclient库，把它放到 `php-config --extension-dir`指向的目录中
 */
include("../../src/safclient.php");

$request_data = '{"person":[{"name":"xu","id":1,"email":"sailsxu@gmail.com"}]}';

/*
// 测试长连接速度，测试一个连接差不多可以达到1w每秒
$client = new RpcClient("127.0.0.1", 8000, true);
if ($client->init()){
  for ($i = 0; $i < 10000; $i++) {
    $response_data = $client->RawCallMethod("AddressBookService", "add", $request_data, 2);
    //echo $response_data;
  }
} else {
  echo "can't connect\n";
}
*/

// 测试短连接时客户端的速度，因为RpcClient在关闭时要进行资源和线程释放，所以需要一定时间
// 测试发现差不多1秒只能调用150次，所以对于大并发客户端，不要用这种方式，测试时注意最大
// fd的限制
for ($i = 0; $i < 200; $i++) {
  $client = new RpcClient("127.0.0.1", 8000, true);
  if ($client->init()) {
    $response_data = $client->RawCallMethod("AddressBookService", "add", $request_data, 2);
    // echo $response_data;
  } else {
    echo "can't connect\n";
  }
}
?>
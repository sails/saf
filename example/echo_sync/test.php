<?php
/*测试，先编译php_safclient库，把它放到 `php-config --extension-dir`指向的目录中
 */
include("../../src/safclient.php");

$client = new RpcClient("127.0.0.1", 8000, true);
$request_data = '{"person":[{"name":"xu","id":1,"email":"sailsxu@gmail.com"}]}';
//for ($i = 0; $i < 10000; $i++) {
  $response_data = $client->RawCallMethod("AddressBookService", "add", $request_data, 2);
  echo $response_data;
//}

?>
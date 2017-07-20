<?php

header("Content-type: text/html; charset=utf-8");
define("TOKEN", "guojing");//define("TOKEN", "enedunet");
define("APPID", "wx40a0a42fde395dae");//define("APPID", "wx3fa0f0bafeb6b40e");
define("APPSECRET", "ca9b24203c47aca662c0d7cf22dfca23");//define("APPSECRET", "f3be1537efa7603944ec267e038f3acb");

$appid = "wx40a0a42fde395dae";
$appsecret = "ca9b24203c47aca662c0d7cf22dfca23";
$url = "https://api.weixin.qq.com/cgi-bin/token?grant_type=client_credential&appid=$appid&secret=$appsecret";

$output = https_request($url);
$jsoninfo = json_decode($output, true);

$access_token = $jsoninfo["access_token"];

$jsonmenu = '{
	"button":[
	{
		"name":"雨水情",
		"sub_button":[
		{
			"type":"click",
			"name":"上报汛情",
			"key":"请输入你要上报的汛情！"
		},
		{
			"type":"click",
			"name":"查询上报",
			"key":"请输入你的汛情编号!"
		}]
	}]
}';

$url = "https://api.weixin.qq.com/cgi-bin/menu/create?access_token=".$access_token;

$result = https_request($url, $jsonmenu);
var_dump($result);

function https_request($url, $data=null){
	$curl = curl_init();
	curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, FALSE);
    curl_setopt($curl, CURLOPT_SSL_VERIFYHOST, FALSE);
    if (!empty($data)){
        curl_setopt($curl, CURLOPT_POST, 1);
        curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
    }
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    $output = curl_exec($curl);
    curl_close($curl);
    return $output;;
};
?>
<?php
	
	/**
	 * 企业的消息处理逻辑主要包含两部分：
	 * 1：接受企业微信推送过来的消息，解包进行验证并取出数据 
	 * 2：处理完业务逻辑后将消息封装为XML数据包推送到企业微信的接口然后push给用户	 	 
	 */
	require_once "../lib/app_api.php";
	
	$reqContent = $_GET["content"];
	$reqContent = "111";
	$touser = $_GET["username"];
	
	$text = array("content"=>$reqContent . "测试——上报汛情成功！");
	file_put_contents('content.txt', $text); //debug:查看smg 
	
	$touser = "GuoJing";
	
	/**
	 * 场景1、企业主动向用户推送消息  
     * 支持文本消息、图片消息、语音消息、视频消息、文件消息、文本卡片消息、图文消息等消息类型	 	 
	 */	 
	$msg = array(
 		'touser'	=>	$touser, 
 		'toparty'	=>	'', 
 		'msgtype'	=>	'text',
 		'agentid'	=>	1000002,
 		'text'		=>	$text
 	);
	 		 	
	 $api = new APP_API(1000002);
	 	
	 $api->sendMsgToUser($msg);
?>


<?php
	/**
	 * 企业的消息处理逻辑主要包含两部分：
	 * 1：接受企业微信推送过来的消息，解包进行验证并取出数据 
	 * 2：处理完业务逻辑后将消息封装为XML数据包推送到企业微信的接口然后push给用户	 	 
	 */

	require_once "../lib/helper.php";	
	require_once "../lib/msgcrypt.php";
	require_once "../lib/app_api.php";
	
	//读取config文件里面的配置
	$appConfigs = loadConfig();
	$config = getConfigByAgentId(1000002);

	$token = $config->Token;
	$encodingAesKey = $config->EncodingAESKey;	
	$corpId = $appConfigs->CorpId;

	/**
	 * 场景2、企业接收用户在应用的聊天窗口输入后传递过来的数据
     * 支持文本消息、图片消息、语音消息、视频消息、文件消息、文本卡片消息、图文消息等消息类型	 	 
	 */	 

    //公众号服务器数据  
    $sReqMsgSig = $sVerifyMsgSig = $_GET['msg_signature'];  
    $sReqTimeStamp = $sVerifyTimeStamp = $_GET['timestamp'];  
    $sReqNonce = $sVerifyNonce = $_GET['nonce'];  
    $sReqData = file_get_contents("php://input");
	
	//parse_str($sReqData, $data);
	//log_request_info(); 
	//echo $sReqData;
	/*$sReqData = array(
		"ToUserName"	=>	"ww2bac3cf3a0aa33ea",
		"FromUserName"	=>	"GuoJing",
		"CreateTime"	=>	time(),
		"MsgType"		=>	"text",
		"Content"		=>	"报汛",
		"MsgId"			=>	"1391751380",
		"AgentID"		=>	"1000002"); */
    $sVerifyEchoStr = $_GET['echostr'];  
	
    $wxcpt = new MsgCrypt($token, $encodingAesKey, $corpId);   
    if($sVerifyEchoStr){  
		$sEchoStr = "";
		$errCode = $wxcpt->VerifyURL($sVerifyMsgSig, $sVerifyTimeStamp, $sVerifyNonce, $sVerifyEchoStr, $sEchoStr);  
		if ($errCode == 0) {  
			print($sEchoStr);   
		} else {  
			print($errCode . "\n\n");  
		}  
		exit;  
    }  
     //decrypt  
    $sMsg = "";  //解析之后的明文  
    $errCode = $wxcpt->DecryptMsg($sReqMsgSig, $sReqTimeStamp, $sReqNonce, $sReqData, $sMsg);  
	file_put_contents('log.txt', $sMsg); //debug:查看smg 
    if ($errCode == 0) {  
		$xml = new DOMDocument();  
		$xml->loadXML($sMsg);   
		$reqToUserName = $xml->getElementsByTagName('ToUserName')->item(0)->nodeValue;  
		$reqFromUserName = $xml->getElementsByTagName('FromUserName')->item(0)->nodeValue;  
		$reqCreateTime = $xml->getElementsByTagName('CreateTime')->item(0)->nodeValue;  
		$reqMsgType = $xml->getElementsByTagName('MsgType')->item(0)->nodeValue;  
		$reqContent = $xml->getElementsByTagName('Content')->item(0)->nodeValue;  
		$reqMsgId = $xml->getElementsByTagName('MsgId')->item(0)->nodeValue;  
		$reqAgentID = $xml->getElementsByTagName('AgentID')->item(0)->nodeValue;  
		switch($reqContent){  
			case "报汛":  
				$mycontent = "请输入标准格式的汛情信息！";
				break;  
			case "马化腾":  
				$mycontent="您好，马化腾！我知道创建了企鹅帝国！";  
				break;  
			case "史玉柱":  
				$mycontent="您好，史玉柱！我知道您创建了巨人网络！";  
				break;  
			default :   
				break;  
		}  
		$sRespData =   
			"<xml>  
				<ToUserName><![CDATA[" . $reqFromUserName . "]]></ToUserName>  
				<FromUserName><![CDATA[" . $corpId . "]]></FromUserName>  
				<CreateTime>" . $sReqTimeStamp . "</CreateTime>  
				<MsgType><![CDATA[text]]></MsgType>  
				<Content><![CDATA[" . $mycontent . "]]></Content>  
			</xml>";  
		$sEncryptMsg = ""; //xml格式的密文  
		$errCode = $wxcpt->EncryptMsg($sRespData, $sReqTimeStamp, $sReqNonce, $sEncryptMsg);  
		if($errCode == 0) {  
			file_put_contents('smg_response.txt', $sEncryptMsg); //debug:查看smg  
			print($sEncryptMsg);  
		} else {  
			print("ERR: " . $errCode . "\n\n");	
		}  
    } else {  
		print("ERR: " . $errCode . "\n\n");	
    }    
?>


<?php
	/**
	 * 企业的消息处理逻辑主要包含两部分：
	 * 1：接受企业微信推送过来的消息，解包进行验证并取出数据 
	 * 2：处理完业务逻辑后将消息封装为XML数据包推送到企业微信的接口然后push给用户	 	 
	 */

	require_once "../lib/helper.php";	
	require_once "../lib/msgcrypt.php";
	require_once "../lib/app_api.php";
	include "../checkBwByWx.php";
	
	//读取config文件里面的配置
	$appConfigs = loadConfig();
	$config = getConfigByAgentId(1000003);

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
	file_put_contents('log.txt', time() . $sReqData); //debug:查看smg 
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
    if ($errCode == 0) {  
		$xml = new DOMDocument();  
		$xml->loadXML($sMsg);   
		$reqToUserName = $xml->getElementsByTagName('ToUserName')->item(0)->nodeValue;  
		$reqFromUserName = $xml->getElementsByTagName('FromUserName')->item(0)->nodeValue;  
		$reqCreateTime = $xml->getElementsByTagName('CreateTime')->item(0)->nodeValue;  
		$reqMsgType = $xml->getElementsByTagName('MsgType')->item(0)->nodeValue; 
		
		$reporter = $reqFromUserName; //报汛人

		switch($reqMsgType){
			case "event": //消息类型，event 
				$reqEvent = $xml->getElementsByTagName("Event")->item(0)->nodeValue;
				if($reqEvent == "click"){//事件类型，click 
					$reqEventKey = $xml->getElementsByTagName("EventKey")->item(0)->nodeValue;
					switch($reqEventKey){//事件KEY值，与自定义菜单接口中KEY值对应 
						/* 水情报送 */
						case "Report-H":
							$mycontent = "请输入河道水情报文！\n\n编报8时河道水情的示例如下：\n\n某河道站55202，6月17日8时水位134.72m，水势涨，查线流量为1350m3/s，则可用A格式编码：\ns\nH┗┛55202┗┛06170800┗┛Z┗┛134.72┗┛ZS┗┛5┗┛Q┗┛1350┗┛QS┗┛1┗┛NN\n\n┗┛代表空格";
							break;
						case "Report-K":
							$mycontent = "请输入水库水情报文！";
							break;
						case "Report-M":
							$mycontent = "请输入土壤墒情报文！";
							break;
						/* 历史查阅 */
						case "History-H":
							$type = "H";
							$mycontent = getHistoryDatasByType($reporter, $type);
							break;
						case "History-K":
							$type = "K";
							$mycontent = getHistoryDatasByType($reporter, $type);
							break;
						case "History-M":
							$type = "M";
							$mycontent = getHistoryDatasByType($reporter, $type);
							break;
						/* 报文统计 */
						case "Statistics-H":
							$type = "H";
							$mycontent = getStatisticsByType($reporter, $type);
							break;
						case "Statistics-K":
							$type = "K";
							$mycontent = getStatisticsByType($reporter, $type);
							break;
						case "Statistics-M":
							$type = "M";
							$mycontent = getStatisticsByType($reporter, $type);
							break;
						default:
							break;
					}
				}
				break;
			case "text": //消息类型，text
				$reqContent = $xml->getElementsByTagName('Content')->item(0)->nodeValue;  
				$reqMsgId = $xml->getElementsByTagName('MsgId')->item(0)->nodeValue;  
				$reqAgentID = $xml->getElementsByTagName('AgentID')->item(0)->nodeValue;
				$str = "reqToUserName=" . $reqToUserName . "&reqFromUserName=" . $reqFromUserName . "&reqCreateTime=" . $reqCreateTime . "&reqMsgType=" . $reqMsgType . "&reqContent=" . $reqContent . "&reqMsgId=" . $reqMsgId . "&reqAgentID=" . $reqAgentID;
				$reporter = $reqFromUserName;
				$reporttype = 1;
				$baoxun = $reqContent;
				if($baoxun != ""){
					$res = checkbx($reporter, $reporttype, $baoxun);
				}else{
					$res = 4;
				}
				$bw = "返回值为：" . $res . "&reporter=" . $reporter . "&reporttype=" . $reporttype . "&baoxun=" . $baoxun . " ";
				switch($res){  
					case 1:  
						$mycontent = $str . $bw . "上报成功！";
						break;  
					case 2:  
						$mycontent = $str . $bw . "报文格式错误，请仔细检查！";  
						break;  
					case 3:  
						$mycontent = $str . $bw . "插入数据库失败！";  
						break;  
					default :   
						//$mycontent = $bw . "未知错误！";  
						break;  
				}
				break;
			default:
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
	
	//历史查阅——最新十条
	function getHistoryDatasByType($reporter, $type){
		$html = "";
		mysql_connect("122.114.144.156","enedu","enedu987");
		mysql_select_db("waterbureau");
		$sql = "select id, content, createtime from report where reporttype = 1 and reporter = '" . $reporter . "' and type = '" . $type . "' order by createtime desc limit 10";
		$res = mysql_query($sql);
		
		if(mysql_num_rows($res) > 0){
			$i = 0;
			while($row = mysql_fetch_array($res)){
				if($i == 0){
					$html .= "报文：" . $row["content"] . "\n发送时间：" . $row["createtime"];
				}else{
					$html .= "\n\n报文：" . $row["content"] . "\n发送时间：" . $row["createtime"];
				}
				$i++;
			}
		}else{
			$html = "暂无数据！";
		}

		return $html;
	}

	//报文统计
	function getStatisticsByType($reporter, $type){
		$html = "";
		mysql_connect("122.114.144.156","enedu","enedu987");
		mysql_select_db("waterbureau");
		$sql = "select id, content, createtime from report where reporttype = 1 and reporter = '" . $reporter . "' and type = '" . $type . "' order by createtime desc";
		$res = mysql_query($sql);
		
		$num = mysql_num_rows($res);

		if($type == "H"){
			$typeStr = "【河道站】";
		}else if($type == "K"){
			$typeStr = "【水库站】";
		}else if($type == "M"){
			$typeStr = "【墒情】";
		}else{
			$typeStr = "";
		}
		$html = "您上报的" . $typeStr . "报文条数为【" . $num . "】条";

		return $html;
	}
?>


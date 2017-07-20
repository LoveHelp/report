<?php
header("Content-type: text/html; charset=utf-8");
include "conn.php";

$errorMsg = "";
$keyword = $_POST['content'];
$reportType = $_POST['type'];
$reporter = $_POST['reporter'];
/*$reportType = 1;
$reporter = "guojing";*/

//$keyword = "K┗┛59804┗┛07040800┗┛ZU┗┛18.05┗┛ZS┗┛4┗┛W┗┛1500┗┛NN";
//$keyword = str_replace("┗┛", " ", $keyword);
$array = explode(" ", $keyword);

$type = $array[0];//汛情种类
$stcd = $array[1];//测站编码
$time = $array[2];
$tm = date("Y") . "-" . substr($time, 0 , 2) . "-" . substr($time, 2, 2) . " " . substr($time, 4, 2) . ":" . substr($time, 6, 2) . ":00";//时间

// H:河道 K:水库 M:墒情
$current_time = date('Y-m-d H:i:s');
//降水——P	河道——H 水库（湖泊）——K 闸坝——Z 泵站——D 潮汐——T 土壤墒情——M 地下水情——G 特殊水情——Y 水文预报——F
	
$sql = "select stcd from st_pptn_r where stcd = '" . $stcd . "' and tm = '" . $tm . "'";
$res = mysql_query($sql);
if(mysql_num_rows($res) < 1){
	$sql = "insert into report (type, reporter, reporttype, content, createtime) values ('" . $type . "', '" . $reporter . "'," . $reportType . ",'" . $keyword . "','" . $current_time . "')";
	$res2 = mysql_query($sql);
	if($res2){
		$reportid = mysql_insert_id();
		if($type == "P"){//降水量
			/*
				编报时段降水量的示例如下：
				某雨量站81012，采用三级标准编报，降水量的起报标准为有雨即报。6月18日8时至14时降水量为1.4mm，14时天气阴，则该时段降水量编码可采用A格式编写：
				P┗┛a 81012┗┛06181400┗┛P6┗┛1.4┗┛WS┗┛8┗┛NN 
				a：“┗┛”表示空格符。下同。
			*/
			$wth = $array[6];
			$interval = $array[3]; //时段
			//时段长
			if($interval == "PD"){
				$dyp = $array[4];
				$sql = "insert into st_pptn_r (stcd, tm, dyp, wth, reportid) values ('" . $stcd . "', '" . $tm . "', " . $dyp . "," . $wth . "," . $reportid . ")";
			}else{
				if($interval == "P1"){
				$intv = 1;
				}else if($interval == "P2"){
					$intv = 2;
				}else if($interval == "P3"){
					$intv = 3;
				}else if($interval == "P6"){
					$intv = 6;
				}else if($interval == "P12"){
					$intv = 12;
				}
				$drp = $array[4];
				$sql = "insert into st_pptn_r (stcd, tm, drp, intv, wth, reportid) values ('" . $stcd . "', '" . $tm . "', " . $drp . "," . $intv . "," . $wth . "," . $reportid . ")";
			}
			
		}else if($type == 'H'){//河道水情
			/*
				编报8时河道水情的示例如下：
				某河道站55202，6月17日8时水位134.72m，水势涨，查线流量为1350m3/s，则可用A格式编码：
				H┗┛55202┗┛06170800┗┛Z┗┛134.72┗┛ZS┗┛5┗┛Q┗┛1350┗┛QS┗┛1┗┛NN
			*/
			$z = $array[4];
			$wptn = $array[6];
			$q = $array[8];
			$sql = "insert into st_river_r (stcd, tm, z, q, wptn, reportid) values ('" . $stcd . "', '" . $tm . "', " . $z . "," . $q . "," . $wptn . "," . $reportid . ")";
		}else if($type == 'K'){//水库水情
			/*
				编报湖泊水情的示例如下：
				某湖泊站59804，7月4日8时，湖水位18.05m，水势落，蓄水量15.0亿m3。A格式编码为：
				K┗┛59804┗┛07040800┗┛ZU┗┛18.05┗┛ZS┗┛4┗┛W┗┛1500┗┛NN
			*/
			$rz = $array[4];//库水位
			$rwptn = $array[6];//库水水势
			$w = $array[8];//蓄水量
			$sql = "insert into st_rsvr_r (stcd, tm, rz, w, rwptn, reportid) values ('" . $stcd . "', '" . $tm . "', " . $rz . "," . $w . "," . $rwptn . "," . $reportid . ")";
		}

		$res3 = mysql_query($sql);
		if($res3){
			$errorMsg = "上报汛情成功！";
		}else{
			$sql = "delete from report where id = " . $reportid;
			mysql_query($sql);
			$errorMsg = "发生错误，上报汛情失败！";
		}
	}else{
		$errorMsg = "发生错误，上报汛情失败！";
	}	
}else{
	$errorMsg = "汛情已存在，上报汛情失败！";
}

echo $errorMsg;

mysql_free_result($res);
mysql_close();
?>
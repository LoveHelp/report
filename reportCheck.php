<?php
header("Content-type: text/html; charset=utf-8");
include "conn.php";

$keyword = $_POST['content'];
$errorMsg = "";
$reportType = 3;
// H:河道 K:水库 M:墒情
$codeArr = array("B2016010", "B2016020", "B2016030");
$type = substr($keyword, 0, 1);

if($type == 'H' || $type == "K" || $type == "M"){
	$stcd = substr($keyword, 2, 8);
	$time = substr($keyword, 11, 19);
	$data = substr($keyword, 31);
	$wptn = 6;
	$current_time = date('Y-m-d h:i:s');
	//$current_time = strtotime(date());
	//$report_time = strtotime($time);
	if(!in_array($stcd, $codeArr, true)){
		$errorMsg = '组织机构代码错误，上报汛情失败！';
	}else if($time == ""){
		$errorMsg = "缺少时间！";
	}else if($time > $current_time){
		$errorMsg = 'time:' . $time . ' current_time:' . $current_time . '时间大于当前时间，上报汛情失败！';
	}else if($data == ""){
		$errorMsg = "数据为空！";
	}else{
		$createtime = date('Y-m-d h:i:s', time());
		mysql_connect("122.114.144.156","enedu","enedu987");
		mysql_select_db("waterbureau");
		if($type == 'H'){//河道水情
			$sql = "insert into st_river_r (stcd, tm, z, wptn, type, createtime) values ('" . $stcd . "', '" . $time . "', " . $data . "," . $wptn . "," . $reportType . ",'" . $createtime . "')";
		}else if($type == "K"){//水库水情
			$sql = "insert into st_rsvr_r (stcd, tm, rz, rwptn, type, createtime) values ('" . $stcd . "', '" . $time . "', " . $data . "," . $wptn . "," . $reportType . ",'" . $createtime . "')";
		}else{//土壤墒情
			$sql = "insert into st_soil_r (stcd, tm, rz, rwptn, type, createtime) values ('" . $stcd . "', '" . $time . "', " . $data . "," . $wptn . "," . $reportType . ",'" . $createtime . "')";
		}
		$res = mysql_query($sql);
		
		if($res){
			$errorMsg = 'stcd:' . $stcd . ' time:' . $time . ' current_time:' . $current_time . '上报汛情成功！';
		}else{
			$errorMsg = $sql . $res . '添加汛情失败！';
		}

		mysql_close();
	}
}else{
	$errorMsg = "报讯格式错误！";
}

echo $errorMsg;

?>
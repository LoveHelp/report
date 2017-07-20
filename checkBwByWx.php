<?php
/*
检查报讯报文并将合法报文存入report表及标准库:
报文格式的合法性.报讯成功返回1,报讯信息格式错误返回2,报讯存储错误返回3;
使用POST请求接收报文.
创建人:闫学森 2017-06-14
*/
$APP_Path=$_SERVER['DOCUMENT_ROOT'];
include_once($APP_Path.'/wx/logmsg.php');
include_once($APP_Path.'/wx/codeInfo.php');
header('content-type:text/html;charset=utf-8');
$baoxun=$_POST['bx'];
$reporter=$_POST['reporter'];
$reporttype=$_POST['reporttype'];
$baotou=array(1=>'P',2=>'H',3=>'K',4=>'Z',5=>'M',6=>'RP',7=>'RH',8=>'RK',9=>'RZ',10=>'RM',11=>'DP',12=>'DH',13=>'DK',14=>'DZ',15=>'DM');
//$baoxun='K 75626 10010800 ZUX 90.54 QAX 62.0 QIM 158 ZUM 101.05 WM 78 QAM 108 TM 09231330 ZUXM 100.46 TM 09282000 ZUXN 82.32 TM 09261800 QAXM 87 TM 09220800 QAXN 54 TM 09071400 QIMM 425 DT 6 TM 09031700 ZUMM 108.42 TM 09120800 ZUMN 78.47 TM 09031700 WMM 318 TM 09120800 WMN 28 TM 09051640 QAMM 125 TM 09140600 QAMN 32 GS 4 GN 1 QZM 68 TM 09100820 QZMM 100 GT 3 GH 1.30 TM 09200810 QZMN 20  GT 1 GH 0.80 NN';
//$reporter='yan';
//$reporttype=1; 
$elements=array();
//checkbx($reporter,$reporttype,$baoxun);
//报讯合法性检查及存储
function checkbx($reporter,$reporttype,$bx)
{
	$sflag=array();//存储结果返回数组
	$TMflag=array(
	'ZDM'=>'HTZ',
	'ZXM'=>'HTZ',
	'ZMM'=>'HTZ',
	'ZYM'=>'HTZ',
	'ZDN'=>'LTZ',
	'ZXN'=>'LTZ',
	'ZMN'=>'LTZ',
	'ZYN'=>'LTZ',
	'QDM'=>'MXQ',
	'QXM'=>'MXQ',
	'QMM'=>'MXQ',
	'QYM'=>'MXQ',
	'QDN'=>'MNQ',
	'QXN'=>'MNQ',
	'QMN'=>'MNQ',
	'QYN'=>'MNQ',
	'ZUDM'=>'HTRZ',
	'ZUXM'=>'HTRZ',
	'ZUMM'=>'HTRZ',
	'ZUYM'=>'HTRZ',
	'ZUDN'=>'LTRZ',
	'ZUXN'=>'LTRZ',
	'ZUMN'=>'LTRZ',
	'ZUYN'=>'LTRZ',
	'WDM'=>'MXW',
	'WXM'=>'MXW',
	'WMM'=>'MXW',
	'WYM'=>'MXW',
	'WDN'=>'MNW',
	'WXN'=>'MNW',
	'WMN'=>'MNW',
	'WYN'=>'MNW',
	'QADM'=>'MXOTQ',
	'QAXM'=>'MXOTQ',
	'QAMM'=>'MXOTQ',
	'QAYM'=>'MXOTQ',
	'QADN'=>'MNOTQ',
	'QAXN'=>'MNOTQ',
	'QAMN'=>'MNOTQ',
	'QAYN'=>'MNOTQ',
	'ZBDM'=>'HTDWZ',
	'ZBXM'=>'HTDWZ',
	'ZBMM'=>'HTDWZ',
	'ZBYM'=>'HTDWZ',
	'ZBDN'=>'LTDWZ',
	'ZBXN'=>'LTDWZ',
	'ZBMN'=>'LTDWZ',
	'ZBYN'=>'LTDWZ',
	'QIDM'=>'MXINQ',
	'QIXM'=>'MXINQ',
	'QIMM'=>'MXINQ',
	'QIYM'=>'MXINQ',
	'QIDN'=>'MNINQ',
	'QIXN'=>'MNINQ',
	'QIMN'=>'MNINQ',
	'QIYN'=>'MNINQ'
	);
	$karr=array('GS','GN','GT','GH','QZ');
	$reporttypearr=array(1=>'微信',2=>'短信',3=>'网络');
	logmsg('收到'.$reporter.$reporttypearr[$reporttype].'报汛:'.$bx);
	$bxarry=validity($bx);
	if($bxarry){
		//将合法报讯信息存入report表
		$data=array('type'=>$bxarry[0],		
					'reporter'=>$reporter,
					'reporttype'=>$reporttype,
					'content'=>$bx,
					'createtime'=>Date('Y-m-d H:i:s'));
		
		if($reportid=store('report',$data)){$sflag[]=1;logmsg('操作成功');}else{return 3;logmsg('操作失败');exit;};
		//将合法报讯信息存入标准库相对应表
		global $elements;		
		$stcd=$bxarry[1];
		$tm=date('Y').'-'.substr($bxarry[2],0,2).'-'.substr($bxarry[2],2,2).' '.substr($bxarry[2],4,2).':'.substr($bxarry[2],6,2).':00';
		$type=$bxarry[0];
		if(strlen($type)>1){
			$ud=substr($type,0,1);
			$path=$_SERVER['DOCUMENT_ROOT'].'/wx/Element'.strtoupper(substr($type,1,1)).'.xml';
			if(file_exists($path)) $elements=readelement($path);
			if($ud=='d' || $ud=='D')
			{
			//删除报文
			$i=3;
			$j=0;			
			$tb=array();
			$data=array();	
			$old='';
			while($i<count($bxarry)){				
				foreach($elements as $e){
					if($bxarry[$i]==$e['flag']){
						if($e['tb']!='%'){
						 if(count($tb)>0){
							$f=false;
							for($k=0;$k<count($tb);$k++){
								
								if($e['tb']==$tb[$k]){
									$j=$k;
									$f=true;
									break;
								}
							}
							if(!$f){
								$j=count($tb);
								$tb[$j]=$e['tb'];
							}
						 }else{
							$tb[$j]=$e['tb'];
						 }
						}
					
					break;	
					}						
				}
			$i=$i+2;
			}
			
			//存入标准库
			//print_r($data);
			for($i=0;$i<count($tb);$i++)
			{
				$where='STCD="'.$stcd.'" and ';
				if(strtoupper($tb[$i])=='ST_PPTN_R' || strtoupper($tb[$i])=='ST_STORM_R' || strtoupper($tb[$i])=='ST_HAIL_R' || strtoupper($tb[$i])=='ST_SNOW_R' || strtoupper($tb[$i])=='ST_RIVER_R' || strtoupper($tb[$i])=='ST_RSVR_R' || strtoupper($tb[$i])=='ST_WAS_R' || strtoupper($tb[$i])=='ST_GATE_R'|| strtoupper($tb[$i])=='ST_SOIL_R'){
					$where.='TM="'.$tm.'"';
					}else{
						$where.='IDTM="'.$tm.'"';
						}
				if(store($tb[$i],'',$where,'delete')){$sflag[]=1;logmsg('操作成功');}else{$sflag[]=0;logmsg('操作失败');}
			}
			//删除结束			
			}
			elseif($ud=='r' || $ud=='R')
			{
			//修改报文
			$i=3;
			$j=0;			
			$tb=array();
			$data=array();	
			
			while($i<count($bxarry)-1){				
				foreach($elements as $e){
					if($bxarry[$i]==$e['flag']){
						if($e['tb']!='%'){
						 if(count($tb)>0){
							$f=false;
							for($k=0;$k<count($tb);$k++){								
								if($e['tb']==$tb[$k]){
									$j=$k;
								 //避免同类字段多次出现在同一数组中,新插入一条
								  if(in_array($bxarry[$i],$karr)){
								   if(!array_key_exists($e['filed'],$data[$j])){
								   	$f=true;
									break;
								   }
								 }else{
								   	$f=true;
									break;								 
								 }
								}
							}
							if(!$f){
								$j=count($tb);
								if(empty($e['tb'])) break;
								$tb[$j]=$e['tb'];
							}
						 }else{
							if(empty($e['tb']))  break;
								$tb[$j]=$e['tb'];
						 }
						}
						if(!empty($e['parentfield'])){$data[$j][$e['parentfield']]=$e['parentvalue'];}						
						if($e['filed']=='%'){							
							if(strtoupper($bxarry[$i])=='TM'){
							if(array_key_exists($bxarry[$i+2],$TMflag)) $tmf=$TMflag[$bxarry[$i+2]];
							if(!empty($tmf)) $data[$j][$tmf.'TM']=date('Y').'-'.substr($bxarry[$i+1],0,2).'-'.substr($bxarry[$i+1],2,2).' '.substr($bxarry[$i+1],4,2).':'.substr($bxarry[$i+1],6,2).':00';
							}elseif(strtoupper($bxarry[$i])=='ZS'){
							$tmf=$TMflag[$bxarry[$i-2]];
							if($tmf=='ZU') $data[$j]['SUPWPTN']=$bxarry[$i+1];
							if($tmf=='ZB') $data[$j]['SDWWPTN']=$bxarry[$i+1];
							}
						}else{
							if(strtoupper($bxarry[$i])=='DT'){
								if(strtoupper($tb[$j])=='ST_PPTN_R')
								{
									$data[$j]['PDR']=$bxarry[$i+1];
								}elseif(strtoupper($tb[$j])=='ST_STORM_R'){
									$data[$j]['STRMDR']=$bxarry[$i+1];
								}else{
									$data[$j][$e['filed']]=$bxarry[$i+1];
								}
							}
							else
							{
									$data[$j][$e['filed']]=$bxarry[$i+1];
							}
						}
					break;	
					}						
				}
			$i=$i+2;
			}
			
			//存入标准库
			if(count($data)==0){
				echo 2;
				store('report','','id='.$reportid,'delete');
				exit;
			}
			$exkey=65;
			for($i=0;$i<count($tb);$i++)
			{
				$where='STCD="'.$stcd.'" and ';
				if(strtoupper($tb[$i])=='ST_PPTN_R' || strtoupper($tb[$i])=='ST_STORM_R' || strtoupper($tb[$i])=='ST_HAIL_R' || strtoupper($tb[$i])=='ST_SNOW_R' || strtoupper($tb[$i])=='ST_RIVER_R' || strtoupper($tb[$i])=='ST_RSVR_R' || strtoupper($tb[$i])=='ST_WAS_R' || strtoupper($tb[$i])=='ST_GATE_R' || strtoupper($tb[$i])=='ST_SOIL_R'){
					$where.='TM="'.$tm.'"';
					}else{
						$where.='IDTM="'.$tm.'"';
				}								
				if(strtoupper($tb[$i])=='ST_GATE_R' || strtoupper($tb[$i])=='ST_SOIL_R'){
					$where.=' and EXKEY="'.chr($exkey).'"';
					$exkey=$exkey+1;
				}		
				if(store($tb[$i],$data[$i],$where,'update')){$sflag[]=1;logmsg('操作成功');}else{$sflag[]=0;logmsg('操作失败');}
			}
			//修改结束
				
			}else{
				return 2;exit;
			}
			
		}else{
			$path=$_SERVER['DOCUMENT_ROOT'].'/wx/Element'.strtoupper($type).'.xml';
			$elements=readelement($path);
			if(file_exists($path)) $elements=readelement($path);
			//逐要素读出,先判断所属表,后存入同序号字段数组
			$i=3;
			$j=0;
			$b=0;
			$tb=array();
			$data=array();	
			
			while($i<count($bxarry)-1){	
				$tmf='';
				foreach($elements as $e){
					if($bxarry[$i]==$e['flag']){
						if(empty($e['tb']))  break;
						if(empty($e['filed']))  break;
						if($e['tb']!='%'){
						 if(count($tb)>0){
							$f=false;
							for($k=0;$k<count($tb);$k++){							
							 if($e['tb']==$tb[$k]){
									$j=$k;
								//避免同类字段多次出现在同一数组中,新插入一条
							  if(in_array($bxarry[$i],$karr)){
								if(!array_key_exists($e['filed'],$data[$j])){
								   	$f=true;
									break;
									 }
							  }else{
								   	$f=true;
									break;								 
							  }
							 }
							}
							if(!$f){

								$j=count($tb);								
								$tb[$j]=$e['tb'];
							}
						 }else{							
							$tb[$j]=$e['tb'];
						 }	
						}
						
						if(!empty($e['parentfield'])){
						$data[$j][$e['parentfield']]=$e['parentvalue'];							
						}						
						if($e['filed']=='%'){							
							if(strtoupper($bxarry[$i])=='TM'){
							if(array_key_exists($bxarry[$i+2],$TMflag)) $tmf=$TMflag[$bxarry[$i+2]];							
							if(!empty($tmf)) $data[$j][$tmf.'TM']=date('Y').'-'.substr($bxarry[$i+1],0,2).'-'.substr($bxarry[$i+1],2,2).' '.substr($bxarry[$i+1],4,2).':'.substr($bxarry[$i+1],6,2).':00';
							}elseif(strtoupper($bxarry[$i])=='ZS'){
							$tmf=$TMflag[$bxarry[$i-2]];
							if($tmf=='ZU') $data[$j]['SUPWPTN']=$bxarry[$i+1];
							if($tmf=='ZB') $data[$j]['SDWWPTN']=$bxarry[$i+1];
							}
						}else{
							if(strtoupper($bxarry[$i])=='DT'){
								if(strtoupper($tb[$j])=='ST_PPTN_R')
								{
									$data[$j]['PDR']=$bxarry[$i+1];
								}elseif(strtoupper($tb[$j])=='ST_STORM_R'){
									$data[$j]['STRMDR']=$bxarry[$i+1];
								}else{
									$data[$j][$e['filed']]=$bxarry[$i+1];
								}
							}
							else
							{								
								$data[$j][$e['filed']]=$bxarry[$i+1];
								}
						}
					break;	
					}						
				}
			$i=$i+2;
			}
			
			//存入标准库
			if(count($data)==0){
				echo 2;
				store('report','','id='.$reportid,'delete');
				exit;
			}
			$exkey=65;
			for($i=0;$i<count($tb);$i++)
			{
				$data[$i]['reportid']=$reportid;
				$data[$i]['STCD']=$stcd;
				if(strtoupper($tb[$i])=='ST_PPTN_R' || strtoupper($tb[$i])=='ST_STORM_R' || strtoupper($tb[$i])=='ST_HAIL_R' || strtoupper($tb[$i])=='ST_SNOW_R' || strtoupper($tb[$i])=='ST_RIVER_R' || strtoupper($tb[$i])=='ST_RSVR_R' || strtoupper($tb[$i])=='ST_WAS_R' || strtoupper($tb[$i])=='ST_GATE_R' || strtoupper($tb[$i])=='ST_SOIL_R'){
					$data[$i]['TM']=$tm;
					}else{
						$data[$i]['IDTM']=$tm;
					}
				
				if(strtoupper($tb[$i])=='ST_GATE_R' || strtoupper($tb[$i])=='ST_SOIL_R'){
					$data[$i]['EXKEY']=chr($exkey);
					$exkey=$exkey+1;
				}
				if(store($tb[$i],$data[$i])>0){$sflag[]=1;logmsg('操作成功');}else{$sflag[]=0;logmsg('操作失败');}
			}
		}
		$cf=0;
		foreach($sflag as $sf){
					$cf+=$sf;
		}
		if($cf==count($sflag)){return 1;}else{
 			for($i=0;$i<count($tb);$i++){
				store($tb[$i],'','reportid='.$reportid,'delete');
			} 
			store('report','','id='.$reportid,'delete');
			return 3;exit;
			}
	}else{
		return 2;
	}
}

//报文格式合法性检查,如不合法,返回false;合法返回切割后数组
function validity($bw)
{	
	logmsg('head');
	global $codeInfo;
	logmsg('middle'.$codeInfo);
	logmsg('middle');
	$bw=trim($bw);
	if(strlen($bw)<14) return false;
	$bw=str_replace(array('　','	',"\r\n", "\r", "\n"),' ',$bw);		
	$bw=preg_replace('/ +/',' ',$bw);
	$explode_str=explode(' ',$bw);
	$bwlen=count($explode_str);		
	if($bwlen%2>0) return false;
	if($bwlen>4){
		if(strtoupper($explode_str[$bwlen-1])=='NN')
		{
			global $baotou;
			$key=0;
			foreach($baotou as $k=>$v){
				if(strtoupper($v)==$explode_str[0]){
					$key=$k;
					break;
				}
			}
 			for($i=3;$i<count($explode_str)-1;$i=$i+2){
				if(!in_array($explode_str[$i],$codeInfo)){
					logmsg('body');
					return false;
					break;
				}
			}  
 
			if($key>0){				
				return $explode_str;
			}else{
				return false;
			}
			
		}else{
			return false;
		}			
	}else{
		return false;
	}
}

//要素表读取
function readelement($path)
{
	$f=fopen($path,'r');
	$content=fread($f,filesize($path));
	fclose($f);
	$xmlarr=xml2array($content);
	$e=$xmlarr['item'];
	return $e;
}

//报讯数据标准库存储.$data为字段与数据键值对,$do为insert,update,delete
function store($tb,$data='',$where='',$do='insert')
{
	$config=array('host'=>'122.114.144.156','port'=>'3306','user'=>'enedu','pass'=>'enedu987','db'=>'waterbureau');
	$con=mysqli_connect($config['host'],$config['user'],$config['pass'],$config['db'],$config['port']);
	if(!$con) return false;
	
	$sql='';
	$res=null;
	$result=false;
	switch($do){
		case 'insert':
		if(count($data)<1) return false;
		$fields='';
		$values='';
		foreach($data as $k=>$v){
			$fields=$fields.$k.',';
			$values=$values.'"'.$v.'",';
		}
		$fields=substr($fields,0,strlen($fields)-1);
		$values=substr($values,0,strlen($values)-1);
		$sql='insert into '.$tb.'('.$fields.') values('.$values.')';
		logmsg($sql);
		$res=mysqli_query($con,$sql);		
		if($res){
			$result=mysqli_insert_id($con);
			if($result==0) $result=1;
				}		
		break;
		
		case 'update':
		if(count($data)<1) return false;
		$sets='';
		foreach($data as $k=>$v){
			$sets=$sets.$k.'="'.$v.'",';			
		}
		$sets=substr($sets,0,strlen($sets)-1);	
		$sql='update '.$tb.' set '.$sets.' where '.$where;
		logmsg($sql);
		$res=mysqli_query($con,$sql);
		if($res) $result=true;
		break;
		
		case 'delete':
		if(strlen($where)<1) return false;		
		$sql='delete from '.$tb.' where '.$where;
		logmsg($sql);
		$res=mysqli_query($con,$sql);
		if($res) $result=true;
		break;	
		default:
		break;
	}
	//mysqli_free_result($res);
	mysqli_close($con);
	return $result;
}
//xml文本转数组
function xml2array($xmlString = '')
 {
  $targetArray = array();
  $xmlObject = simplexml_load_string($xmlString);
  $mixArray = (array)$xmlObject;
  foreach($mixArray as $key => $value)
  {
   if(is_string($value))
   {
    $targetArray[$key] = $value;
   }
   if(is_object($value))
   {
    $targetArray[$key] = xml2array($value->asXML());
   }
   if(is_array($value))
   {
    foreach($value as $zkey => $zvalue)
    {
     if(is_numeric($zkey))
     {
      $targetArray[$key][] = xml2array($zvalue->asXML());
     }
     if(is_string($zkey))
     {
      $targetArray[$key][$zkey] = xml2array($zvalue->asXML());
     }
    }
   }
  }
  return $targetArray;

 }
<?php
header("Content-type:text/html;charset=utf-8");
$tb=array(1=>'P',2=>'H',3=>'K',4=>'Z',5=>'M');
foreach($tb as $k=>$v)
{
readexcel('yaosu.xls',$k,'Element'.$v.'.xml');
}
function readexcel($excelfilename,$sheetpage,$xmlname){
	include_once("Classes/PHPExcel/IOFactory.php");
	if(preg_match("/.xlsx$/", $excelfilename))
	{	
	$reader = PHPExcel_IOFactory::createReader('Excel2007'); 
	}
	else{	
	$reader = PHPExcel_IOFactory::createReader('Excel5'); 
	}
	$PHPExcel = $reader->load($excelfilename); // 文档名称
	$sheet = $PHPExcel->getSheet($sheetpage); // 读取第一个工作表(编号从 0 开始)
	$highestRow = $sheet->getHighestRow(); // 取得总行数
	$highestColumn = $sheet->getHighestColumn(); // 取得总列数

	$start=0;//数据起始行
	$end=$highestRow;//备注起始行
	$data=array();
	for($i=3;$i<=$end;$i++)
	{		
			$data[$i-3]['id']=trim($sheet->getCellByColumnAndRow(0, $i)->getValue());
			$data[$i-3]['element']=trim($sheet->getCellByColumnAndRow(1, $i)->getValue());
			$data[$i-3]['flag']=trim($sheet->getCellByColumnAndRow(2, $i)->getValue());
			$data[$i-3]['info']=trim($sheet->getCellByColumnAndRow(3, $i)->getValue());
			$data[$i-3]['tb']=trim($sheet->getCellByColumnAndRow(4, $i)->getValue());
			$data[$i-3]['parentfield']=trim($sheet->getCellByColumnAndRow(5, $i)->getValue());
			$data[$i-3]['parentvalue']=trim($sheet->getCellByColumnAndRow(6, $i)->getValue());
			$data[$i-3]['filed']=trim($sheet->getCellByColumnAndRow(7, $i)->getValue());			
	}

$string = <<<XML
<?xml version='1.0' encoding='utf-8'?>
<elements>
</elements>
XML;
 
$xml = simplexml_load_string($string); 
foreach ($data as $d) {
    $item = $xml->addChild('item');
    if (is_array($d)) {
        foreach ($d as $key => $row) {
          $item->addChild($key, $row); 
        }
    }
}
if($xml->asXML($xmlname)) echo "导出 $xmlname 成功";
}
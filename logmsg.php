<?php
/*
日志文件
创建人:闫学森 2017-06-14
*/

function logmsg($msg='')
{
	ini_set('date.timezone','Asia/Shanghai');
	$nowtime=date('Y-m-d H:i:s');
	$fd=date('Ymd');
	$fname=$_SERVER['DOCUMENT_ROOT'].'/wx/log/'.$fd.'.log';
	$f=fopen($fname,'a+');
	$s=$nowtime.'  '.$msg."\r\n";
	fwrite($f,$s);
	fclose($f);
}
<?php
$dbname = 'waterbureau';/*数据库名*/
/*填入数据库连接信息*/
$host = '122.114.144.156';//'192.168.1.120';//
$port = 3306;
$user = 'enedu';//'admin';//用户名(api key)
$pwd = 'enedu987';//'CrZQV7Q48aDuca4F';//密码(secret key)
 /*以上信息都可以在数据库详情页查找到*/

/*接着调用mysql_connect()连接服务器*/
$link = @mysql_connect("{$host}:{$port}",$user,$pwd,true);
if(!$link) {
    die("Connect Server Failed: " . mysql_error());
}
/*连接成功后立即调用mysql_select_db()选中需要连接的数据库*/
if(!mysql_select_db($dbname,$link)) {
    die("Select Database Failed: " . mysql_error($link));
}
mysql_query("set names utf8");
?>
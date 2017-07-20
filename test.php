<?php
$CORPID = "ww2bac3cf3a0aa33ea";
$REDIRECT_URI = urlencode("http://www.enedu.net/wx/report.html");
$SCOPE = "snsapi_privateinfo";
$AGENTID = "1000002";
$STATE = "dd";

//$url = "https://open.weixin.qq.com/connect/oauth2/authorize?appid=" . $CORPID . "&redirect_uri=" . $REDIRECT_URI . "&response_type=code&scope=" . $SCOPE . "&agentid=" . $AGENTID . "&state=" . $STATE . "#wechat_redirect";
$url = "https://qy.weixin.qq.com/cgi-bin/loginpage?corp_id=" . $CORPID . "&redirect_uri=" . $REDIRECT_URI . "&state=" . $STATE . "&usertype=member";
echo '<a href="' . $url . '">' . $url . '</a>';
?>
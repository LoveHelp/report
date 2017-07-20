<?php
	
	/*
	 * 通讯录管理 － 查询企业号某部门下所有用户
	 */

	require_once "../lib/txl_api.php";

	$api = new TXL_API();

	$id = isset($_GET["id"]) ? $_GET["id"] : 2;
	$simple = isset($_GET["simple"]) ? $_GET["simple"] : 1;
	$fetch = isset($_GET["fetch"]) ? $_GET["fetch"] : 1;

	$deptListJson = json_decode(queryDepartmentsById($api, $id), true);
	$deptList = $deptListJson["department"];
	
	$userList = Array();

	foreach($deptList as $dept){
		$deptid = $dept["id"];
		$userListJson = json_decode(queryUserByDepId($api, $deptid, $simple, $fetch), true);
		$userArray = $userListJson["userlist"];
		foreach($userArray as $user){
			$userList[] = array(
				"userid"	=>	$user["userid"],
				"name"		=>	$user["name"]);
		}
	}

	echo json_encode($userList);

	function queryDepartmentsById($instance, $id){
		return $instance->getDepartmentsById($id);
	}

	function queryUserByDepId($instance, $id, $simple, $fetch){
		return $instance->queryUsersByDepartmentId($id, $fetch, $simple);	
	}
?>


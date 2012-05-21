<?php
$sm_build = 18;

if( !isset($_SERVER['HTTP_X_SM_BUILD']) )
{
	header( "HTTP/1.1 400 Bad Request" );
	echo "HTTP/1.1 400 Bad Request";
}
else
	header( "X-SM-Build: " . number_format($sm_build, 0, '', '') );

?>

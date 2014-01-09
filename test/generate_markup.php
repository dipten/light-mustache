<?php
/*Include the libary*/ 
require('../src/lightmustache.inc');

/*Take the name of the template as input from command line if passed*/
$templateName = 'test_template1.mu';
if ($argc > 1) {
   $templateName = $argv[1];
}

$template = file_get_contents($templateName);
$phpStr = LightMustache::compile($template , Array('flags' => LightMustache::FLAG_THIS | LightMustache::FLAG_PARENT));

$php_inc = "/tmp/" . preg_replace('/mu/', 'php', $templateName);
file_put_contents($php_inc, $phpStr);

$renderer = include($php_inc);
$assoc_arr = array(
              array('var' => 'h'),
              array('var' => ' '),
              array('var' => 'e'),
              array('var' => ' '),
              array('var' => 'l')
              );

$data = array('name' => 'John', 'value' => 10000,
              'foo' => 'a non empty string',
              'var' => urlencode("http://www.yahoo.com"),
              'var1' => '1',
              'var2' =>  '0',
              'var3' => false,
              'var4' => '',
              'var5' => array('a', 'ab', 'c'),
              'var6' => array(),
              'arr' => array('e1' => 'heh', 'e2' => ''),
              'photosCount' => 10, 'helpLink' => 'http://yahoo.com', 'blogLink' => 'msn.com',
              'nonassoc_arr' => array('hello' , 'welcome', 'to', 'light mustache'),
              'assoc_arr' => $assoc_arr
             );
echo $renderer($data); 
?>

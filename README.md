light-mustache
==============

A PHP library to turbocharge Mustache (http://mustache.github.io/mustache.5.html) performance on server side. It runs as fast as pure PHP code and drastically cuts down (> 75%) the page stitch time.

Features
--------

* Supports both mustache ( http://mustache.github.com/ ) and handlebars ( http://handlebarsjs.com/ ) syntax.
* Decouples the template compilation phase from markup generation.
* Compiles template to <B>pure standalone php</B> code.
   * Examples:
      * templateA: https://github.com/dipten/test/test_template1.mu
      * compile as phpA: https://github.com/dipten/test/test_template1.php
      * templateB: https://github.com/dipten/test/test_template2.mu
      * compile as phpB: https://github.com/dipten/test/test_template2.php
* <B>Fast</B>!
   * runs 4~6 times faster than https://github.com/bobthecow/mustache.php
   * runs 4~10 times faster than https://github.com/dingram/mustache-php
   * runs 10~30 times faster than https://github.com/XaminProject/handlebars.php

* Standalone Template
   * The compiled php template can run without any php library.

Sample
------
<pre>
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
</pre>

Sample output
-------------
<pre>
Template is:
Test case 1:
Welcome {{name}} , You win \${{value}} dollars!!\n
Check for conditional expression {{#foo}}{{foo}}{{/foo}} world

Test case 2:
Variable with double brace {{var}}
Variable with triple brace {{{var}}}

Test case 3:
Check of conditional expression. Output should be if block
{{#var1}}if block{{/var1}}
{{^var1}}else block{{/var1}}

Test case 4:
Check of conditional expression. Output should be else block
{{#var2}}if block{{/var2}}
{{^var2}}else block{{/var2}}

Test case 5:
Check of conditional expression. Output should be else block
{{#var3}}if block{{/var3}}
{{^var3}}else block{{/var3}}

Test case 6:
Check of conditional expression. Output should be else block
{{#var4}}if block{{/var4}}
{{^var4}}else block{{/var4}}

Test case 7:
Check of conditional expression. Output should be if block 3 times
{{#var5}}if block{{/var5}}
{{^var5}}else block{{/var5}}

Test case 8:
Check of conditional expression. Output should be else block
{{#var6}}if block{{/var6}}
{{^var6}}else block{{/var6}}

Test case 9:
Check of conditional expression for variable of the form arr.e1. Output should be if block
{{#arr.e1}}if block{{/arr.e1}}
{{^arr.e1}}else block{{/arr.e1}} value is {{{arr.e1}}}


Rendered PHP code is:
<?php return function ($in) {
    $cx = Array(
        'flags' => Array(
            'jstrue' => false,
            'jsobj' => false,
        ),
        'scopes' => Array($in),
        'path' => Array(),

    );
    return 'Test case 1:
Welcome '.htmlentities(@$in['name'], ENT_QUOTES).' , You win \$'.htmlentities(@$in['value'], ENT_QUOTES).' dollars!!\n
Check for conditional expression '.LCRun::sec('foo', $cx, $in, false, function($cx, $in) {return ''.htmlentities(@$in['foo'], ENT_QUOTES).'';}).' world

Test case 2:
Variable with double brace '.htmlentities(@$in['var'], ENT_QUOTES).'
Variable with triple brace '.@$in['var'].'

Test case 3:
Check of conditional expression. Output should be if block
'.LCRun::sec('var1', $cx, $in, false, function($cx, $in) {return 'if block';}).'
'.((@$in['var1'] === '' || empty($in['var1']) || @$in['var1'] === false) ? 'else block' : '').'

Test case 4:
Check of conditional expression. Output should be else block
'.LCRun::sec('var2', $cx, $in, false, function($cx, $in) {return 'if block';}).'
'.((@$in['var2'] === '' || empty($in['var2']) || @$in['var2'] === false) ? 'else block' : '').'

Test case 5:
Check of conditional expression. Output should be else block
'.LCRun::sec('var3', $cx, $in, false, function($cx, $in) {return 'if block';}).'
'.((@$in['var3'] === '' || empty($in['var3']) || @$in['var3'] === false) ? 'else block' : '').'

Test case 6:
Check of conditional expression. Output should be else block
'.LCRun::sec('var4', $cx, $in, false, function($cx, $in) {return 'if block';}).'
'.((@$in['var4'] === '' || empty($in['var4']) || @$in['var4'] === false) ? 'else block' : '').'

Test case 7:
Check of conditional expression. Output should be if block 3 times
'.LCRun::sec('var5', $cx, $in, false, function($cx, $in) {return 'if block';}).'
'.((@$in['var5'] === '' || empty($in['var5']) || @$in['var5'] === false) ? 'else block' : '').'

Test case 8:
Check of conditional expression. Output should be else block
'.LCRun::sec('var6', $cx, $in, false, function($cx, $in) {return 'if block';}).'
'.((@$in['var6'] === '' || empty($in['var6']) || @$in['var6'] === false) ? 'else block' : '').'

Test case 9:
Check of conditional expression for variable of the form arr.e1. Output should be if block
'.LCRun::sec('arr.e1', $cx, $in, false, function($cx, $in) {return 'if block';}).'
'.LCRun::ifcond('arr.e1', $cx, $in, function($cx, $in) {return 'else block';}).' value is '.LCRun::evalVar('arr.e1', $cx, $in).'

';
}
?>


Welcome John , You win $10000 dollars!!
Welcome Peter , You win $1000 dollars!!
</pre>

CONSTANTS
---------

You can apply more flags by running LightMustache::compile($php, $options)
for example:

LightMustache::compile($template, Array('flags' => LightMustache::FLAG_ERROR_LOG | LightMustache::FLAG_STANDALONE));

Default is to compile the template as php which can be run as fast as possible, all flags are off.

* FLAG_ERROR_LOG : output error_log when found any template error
* FLAG_ERROR_EXCEPTION : throw exception when found any template error
* FLAG_STANDALONE : generate stand alone php codes which can be execute without including LightMustache. The compiled php code will contain scopped user function, somehow larger. And, the performance of the template will slow 1 ~ 10%.
* FLAG_JSTRUE: generate 'true' when value is true (handlebars.js behavior). Otherwise, true will generate ''.
* FLAG_JSOBJECT: generate '[object Object]' for associated array, generate ',' seperated values for array (handlebars.js behavior). Otherwise, all php array will generate ''.
* FLAG_THIS: support {{this}} or {{.}} in template. Otherwise, {{this}} and {{.}} will cause template error.
* FLAG_WITH: support {{#with var}} . Otherwise, {{#with var}} will cause template error.
* FLAG_PARENT: support {{../var}} . Otherwise, {{../var}} will cause template error.
* FLAG_HANDLEBARSJS: align with handlebars.js behaviors, same as FLAG_JSTRUE + FLAG_JSOBJECT + FLAG_THIS + FLAG_WITH + FLAG_PARENT.
* FLAG_ECHO (experimental): compile to echo 'a', $b, 'c'; to improve performance. This will slow down rendering when the template and data are simple, but will improve 1% ~ 7% when the data is big and looping in the template.
* FLAG_BESTPERFORMANCE: same as FLAG_ECHO now. This flag may be changed base on performance testing result.

Partial Support
---------------

LightMustache supports partial when compile time. When compile(), LightMustache will search template file in current directory by default. You can define more then 1 template directories with 'basedir' option. Default template file name is *.tmpl, you can change or add more template file extensions with 'fileext' option. 

for example:
<pre>
LightMustache::compile($template, Array(
    'flags' => LightMustache::FLAG_STANDALONE,
    'basedir' => Array(
        '/usr/local/share/handlebars/templates',
        '/usr/local/share/my_project/templates',
        '/usr/local/share/my_project/partials',
    ),
    'fileext' => Array(
        '.tmpl',
        '.mustache',
        '.handlebars',
    )
));
</pre>

LightMustache supports parent context access in partial {{../vars}}, so far no other php/javascript library can handle this correctly.

Unsupported Feature (so far)
----------------------------

Register a helper function (We wish you to not use custom helper to keep your template generic, then you can reuse these templates in different languages.)

LightMustache Design Concept
--------------------------

* Do not OO everywhere. Single inc file, keep it simple and fast.
* Simulate all mustache/handlebars/javascript behavior, including true, false, Object, Array output behavior.
* Make almost everything happened in compile time, including 'partial' support.

Suggested Handlebars Template Practices
---------------------------------------

* Prevent to use {{#with}} . I think {{path.to.val}} is more readable then {{#with path.to}}{{val}}{{/with}}, when using {{#with}} you will confusing on scope changing. {{#with}} only save you very little time when you access many variables under same path, but cost you a lot time when you need to understand then maintain a template.
* use {{{val}}} when you do not require urlencode. It is better performance, too.
* Prevent to use custom helper if you want to reuse your template in different language. Or, you may need to implement different versions of helper in different languages.

Detail Feature list
-------------------

Go http://handlebarsjs.com/ to see more feature description about handlebars.js. All features align with it.

* Exact same CR/LF behavior with handlebars.js
* Exact same 'true' output with handlebars.js (require FLAG_JSTRUE)
* Exact same '[object Object]' output or join(',' array) output with handlebars.js (require FLAG_JSOBJECT)
* Can place heading/tailing space, tab, CR/LF inside {{ var }} or {{{ var }}}
* {{{value}}} : raw variable
   * true as 'true' (require FLAG_JSTRUE)
   * false as ''
* {{value}} : html encoded variable
   * true as 'true' (require FLAG_JSTRUE)
   * false as ''
* {{{path.to.value}}} : dot notation, raw
* {{path.to.value}} : dot notation, html encoded
* {{.}} : current context, html encoded
* {{this}} : current context, html encoded (require FLAG_THIS)
* {{{.}}} : current context, raw (require FLAG_THIS)
* {{{this}}} : current context, raw (require FLAG_THIS)
* {{#value}} : section
   * false, undefined and null will skip the section
   * true will run the section with original scope
   * All others will run the section with new scope (include 0, 1, -1, '', '1', '0', '-1', 'false', Array, ...)
* {{/value}} : end section
* {{^value}} : inverted section
   * false, undefined and null will run the section with original scope
   * All others will skip the section (include 0, 1, -1, '', '1', '0', '-1', 'false', Array, ...)
* {{! comment}} : comment
* {{#each var}} : each loop
* {{/each}} : end loop
* {{#if var}} : run if logic with original scope (null, false, empty Array and '' will skip this block)
* {{/if}} : end if
* {{else}} : run else logic, should between {{#if var}} and {{/if}} , or between {{#unless var}} and {{/unless}}
* {{#unless var}} : run unless logic with original scope (null, false, empty Array and '' will render this block)
* {{#with var}} : change context scope. If the var is false, skip included section. (require FLAG_WITH)
* {{../var}}: parent template scope. (require FLAG_PARENT)
* {{> file}}: partial; include another template inside a template.
* {{@index}}: reference to current index in a {{#each}} loop on an array.
* {{@key}}: reference to current key in a {{#each}} loop on an object.


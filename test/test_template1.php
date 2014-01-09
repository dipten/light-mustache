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

Test case 10 :
Check of conditional expression for variable of the form arr.e2. Output should be else block
'.LCRun::sec('arr.e2', $cx, $in, false, function($cx, $in) {return 'if block';}).'
'.LCRun::ifcond('arr.e2', $cx, $in, function($cx, $in) {return 'else block';}).' value is '.LCRun::evalVar('arr.e2', $cx, $in).'

Test case 11:
Printing an nonassoc array with '.@$in.'
'.LCRun::sec('nonassoc_arr', $cx, $in, false, function($cx, $in) {return '
    '.@$in.'
';}).'

Test case 12:
Printing an assoc array 
'.LCRun::sec('assoc_arr', $cx, $in, false, function($cx, $in) {return '
    '.htmlentities(@$in['var'], ENT_QUOTES).'
';}).'

';
}
?>
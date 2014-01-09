<?php return function ($in) {
    $cx = Array(
        'flags' => Array(
            'jstrue' => false,
            'jsobj' => false,
        ),
        'scopes' => Array($in),
        'path' => Array(),

    );
    return ''.LCRun::sec('var1', $cx, $in, false, function($cx, $in) {return '
    '.LCRun::sec('var2', $cx, $in, false, function($cx, $in) {return '
       '.LCRun::sec('var3', $cx, $in, false, function($cx, $in) {return '
           '.LCRun::sec('var1.f', $cx, $in, false, function($cx, $in) {return '
                Hello
           ';}).'
           '.LCRun::ifcond('var1.f', $cx, $in, function($cx, $in) {return '
                World
           ';}).'
       ';}).'
    ';}).'
';}).'
';
}
?>
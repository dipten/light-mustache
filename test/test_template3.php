<?php return function ($in) {
    $cx = Array(
        'flags' => Array(
            'jstrue' => false,
            'jsobj' => false,
        ),
        'scopes' => Array($in),
        'path' => Array(),

    );
    return ''.LCRun::evalVar('a.b', $cx, $in, true).'
'.LCRun::evalVar('a.b.c.d', $cx, $in).'
';
}
?>
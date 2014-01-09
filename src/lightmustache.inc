<?php
/*

Copyrights for code authored by Yahoo! Inc. is licensed under the following terms:
MIT License
Copyright (c) 2013 Yahoo! Inc. All Rights Reserved.
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Origin: https://github.com/dipten/light-mustache
*/

class LightMustache {
    // Compile time error handling flags
    const FLAG_ERROR_LOG = 1;
    const FLAG_ERROR_EXCEPTION = 2;

    // Compile the template as standalone php code which can execute without including LightMustache
    const FLAG_STANDALONE = 4;

    // Handlebars.js compatibility
    const FLAG_JSTRUE = 8;
    const FLAG_JSOBJECT = 16;
    const FLAG_THIS = 32;
    const FLAG_WITH = 64;
    const FLAG_PARENT = 128;

    // PHP performance flags
    const FLAG_ECHO = 256;

    const FLAG_BESTPERFORMANCE = 256; // FLAG_ECHO
    const FLAG_HANDLEBARSJS = 248; // FLAG_JSTRUE + FLAG_JSOBJECT + FLAG_THIS + FLAG_WITH + FLAG_PARENT

    const PARTIAL_SEARCH = '/\\{\\{>[ \\t]*(.+?)[ \\t]*\\}\\}/s';
    const TOKEN_SEARCH = '/(\\{{2,3})(.+?)(\\}{2,3})/s';

    private static $lastContext;

    /**
     * Compile handlebars template into PHP code
     *
     * @param string $template handlebars template string
     * @param array $options LightMustache compile time and run time options, default is Array('flags' => LightMustache::FLAG_BESTPERFORMANCE)
     *
     * @return string Compiled PHP code when successed. If error happened and compile failed, return false.
     */
    public static function compile($template, $options = 0) {
        return self::compiledCode($template, $options);
    }

    /**
     * Internal method used by compile(). Get compiled code from a template string
     *
     * @param string $template handlebars template string
     * @param array $options LightMustache compile time and run time options, default is Array('flags' => LightMustache::FLAG_BESTPERFORMANCE)
     *
     * @return string Compiled PHP code when successed. If error happened and compile failed, return false.
     */
    protected static function compiledCode($template, $options) {
        $flags = isset($options['flags']) ? $options['flags'] : self::FLAG_BESTPERFORMANCE;

        $context = Array(
            'flags' => Array(
                'errorlog' => $flags & self::FLAG_ERROR_LOG,
                'exception' => $flags & self::FLAG_ERROR_EXCEPTION,
                'standalone' => $flags & self::FLAG_STANDALONE,
                'jstrue' => $flags & self::FLAG_JSTRUE,
                'jsobj' => $flags & self::FLAG_JSOBJECT,
                'this' => $flags & self::FLAG_THIS,
                'with' => $flags & self::FLAG_WITH,
                'parent' => $flags & self::FLAG_PARENT,
                'echo' => $flags & self::FLAG_ECHO,
            ),
            'level' => 0,
            'stack' => Array(),
            'error' => false,
            'useVar' => false,
            'vars' => Array(),
            'sp_vars' => Array(),
            'jsonSchema' => Array(
                '$schema' => 'http://json-schema.org/draft-03/schema',
                'description' => 'Template Json Schema'
            ),
            'basedir' => self::_basedir($options),
            'fileext' => self::_fileext($options),
            'usedPartial' => Array(),
            'usedFeature' => Array(
                'rootvar' => 0,
                'rootthis' => 0,
                'enc' => 0,
                'raw' => 0,
                'sec' => 0,
                'isec' => 0,
                'if' => 0,
                'else' => 0,
                'unless' => 0,
                'each' => 0,
                'this' => 0,
                'parent' => 0,
                'with' => 0,
                'dot' => 0,
                'comment' => 0,
                'partial' => 0,
                'yrb' => 0,
            )
        );

        $context['ops'] = $context['flags']['echo'] ? Array(
            'seperator' => ',',
            'f_start' => 'echo ',
            'f_end' => ';',
            'op_start' => 'ob_start();echo ',
            'op_end' => ';return ob_get_clean();',
            'cnd_start' => ';if ',
            'cnd_then' => '{echo ',
            'cnd_else' => ';}else{echo ',
            'cnd_end' => ';}echo ',
        ) : Array(
            'seperator' => '.',
            'f_start' => 'return ',
            'f_end' => ';',
            'op_start' => 'return ',
            'op_end' => ';',
            'cnd_start' => '.(',
            'cnd_then' => ' ? ',
            'cnd_else' => ' : ',
            'cnd_end' => ').',
        );

        // Scan for partial and replace partial with template
        $template = LightMustache::expandPartial($template, $context);

        if (self::_error($context)) {
            return false;
        }

        // Do first time scan to find out used feature, detect template error.
        if (preg_match_all(self::TOKEN_SEARCH, $template, $tokens, PREG_SET_ORDER) > 0) {
            foreach ($tokens as $token) {
                self::scan($token, $context);
            }
        }
        if (self::_error($context)) {
            return false;
        }

        // Check used features and compile flags. If the template is simple enough,
        // we can generate best performance code with enable 'useVar' internal flag.
        if (!$context['flags']['jsobj'] && (($context['usedFeature']['sec'] + $context['usedFeature']['parent'] < 1) || !$context['flags']['jsobj'])) {
            $context['useVar'] = Array('$in');
        }

        // Do PHP code and json schema generation.
        $code = preg_replace_callback(self::TOKEN_SEARCH, function ($matches) use (&$context) {
            return '\'' . LightMustache::tokens($matches, $context) . '\'';
        }, addcslashes($template, "'"));

        if (self::_error($context)) {
            return false;
        }

        $flagJStrue = self::_on($context['flags']['jstrue']);
        $flagJSObj = self::_on($context['flags']['jsobj']);

        $libstr = self::exportLCRun($context);

        // return generated PHP code string
        return "<?php return function (\$in) {
    \$cx = Array(
        'flags' => Array(
            'jstrue' => $flagJStrue,
            'jsobj' => $flagJSObj,
        ),
        'scopes' => Array(\$in),
        'path' => Array(),
$libstr
    );
    {$context['ops']['op_start']}'$code'{$context['ops']['op_end']}
}
?>";
    }

    /**
     * expand partial string recursively
     *
     * @param string $template template string
     *
     * @return string partial file content
     */
    public static function expandPartial($template, &$context) {
        $template = preg_replace_callback(self::PARTIAL_SEARCH, function ($matches) use (&$context) {
            return LightMustache::expandPartial(LightMustache::readPartial($matches[1], $context), $context);
        }, $template);
        return $template;
    }

    /**
     * read partial file content as string
     *
     * @param string $name partial file name
     * @param array $context Current context of compiler progress.
     *
     * @return string partial file content
     */
    public static function readPartial($name, &$context) {
        $f = preg_split('/[ \\t]/', $name);
        $context['usedFeature']['partial']++;
        foreach ($context['basedir'] as $dir) {
            foreach ($context['fileext'] as $ext) {
                $fn = "$dir/$f[0]$ext";
                if (file_exists($fn)) {
                    return file_get_contents($fn);
                }
            }
        }
        $context['error'] = "can not find partial file for '$name', you should set correct basedir and fileext in options";
    }

    /**
     * Internal method used by compile(). check options and handle fileext 
     *
     * @param string $options current compile option
     *
     * @return array file extensions
     */
    protected static function _fileext($options) {
        $exts = isset($options['fileext']) ? $options['fileext'] : '.tmpl';
        return is_array($exts) ? $exts : Array($exts);
    }

    /**
     * Internal method used by compile(). check options and handle basedir
     *
     * @param string $options current compile option
     *
     * @return array base directories
     */
    protected static function _basedir($options) {
        $dirs = isset($options['basedir']) ? $options['basedir'] : 0;
        $dirs = is_array($dirs) ? $dirs : Array($dirs);

        foreach ($dirs as $index => $dir) {
            if (!is_dir($dir)) {
                unset($dirs[$index]);
            }
        }

        if (count(array_keys($dirs)) === 0) {
            $dirs[] = getcwd();
        }

        return $dirs;
    }

    /**
     * Internal method used by compile(). export required standalone functions.
     *
     * @param string $context current scaning context
     */
    protected static function exportLCRun($context) {
        if ($context['flags']['standalone'] == 0) {
            return '';
        }

        $class = new ReflectionClass('LCRun');
        $fname = $class->getFileName();
        $lines = file_get_contents($fname);
        $file = new SplFileObject($fname);
        $ret = "'funcs' => Array(\n";

        foreach ($class->getMethods() as $method) {
            $file->seek($method->getStartLine() - 2);
            $spos = $file->ftell();
            $file->seek($method->getEndLine() - 2);
            $epos = $file->ftell();
            $ret .= preg_replace('/self::(.+)\(/', '\\$cx[\'funcs\'][\'$1\'](', preg_replace('/public static function (.+)\\(/', '\'$1\' => function (', substr($lines, $spos, $epos - $spos))) . "    },\n";
        }
        unset($file);
        return "$ret)\n";
    }

    /**
     * Internal method used by compile().
     *
     * @param array $context Current context of compiler progress.
     *
     * @return boolean True when error detected
     */
    protected static function _error($context) {
        self::$lastContext = $context;
        if ($context['level'] !== 0) {
            $token = array_pop($context['stack']);
            $context['error'] = "Unclosed token {{{$token}}} !!";
        }

        if ($context['error']) {
            echo "ERROR occurred while compilation : " . $context['error'];
            if ($context['flags']['errorlog']) {
                error_log($context['error']);
            }
            if ($context['flags']['exception']) {
                throw new Exception($context['error']);
            }
            return true;
        }
        return false;
    }

    /**
     * Internal method used by compile().
     *
     * @param mixed $v value
     *
     * @return boolean True when the value larger then 0
     */
    protected static function _on($v) {
        return ($v > 0) ? 'true' : 'false';
    }

    /**
     * get last compiler context.
     *
     * @return array Context data
     */
    public static function getContext() {
        return self::$lastContext;
    }

    /**
     * get JsonSchema of last compiled handlebars template.
     *
     * @return array JsonSchema data
     */
    public static function getJsonSchema() {
        return self::$lastContext['jsonSchema'];
    }

    /**
     * get JsonSchema of last compiled handlebars template as pretty printed string.
     *
     * @return string JsonSchema string
     */
    public static function getJsonSchemaString($indent = '  ') {
        $level = 0;
        return preg_replace_callback('/\\{|\\[|,|\\]|\\}|:/', function ($matches) use (&$level) {
            switch ($matches[0]) {
                case '}':
                case ']':
                    $level--;
                    $is = str_repeat('  ', $level);
                    return "\n$is{$matches[0]}";
                case ':':
                    return ': ';
            }
            $br = '';
            switch ($matches[0]) {
                case '{':
                case '[':
                    $level++;
                case ',':
                    $br = "\n";
            }
            $is = str_repeat('  ', $level);
            return "{$matches[0]}$br$is";
        }, json_encode(self::getJsonSchema()));
    }

    /**
     * include a string as php codes. this method requires php setting allow_url_include=1 and allow_url_fopen=1
     *
     * @param string $php php codes
     * @param string $tmp_dir optional, change temp directory for php include file saved by prepare() when can not include php code with data:// format.
     *
     * @return mixed result of include()
     */
    public static function prepare($php, $tmp_dir = false) {
        if (!ini_get('allow_url_include') || !ini_get('allow_url_fopen')) {
            if (!is_dir($tmp_dir)) {
                $tmp_dir = sys_get_temp_dir();
            }
        }

        if ($tmp_dir) {
            $fn = tempnam($tmp_dir, 'lci_');
            if (!$fn) {
                die("Can not generate tmp file under $tmp_dir!!\n");
            }
            if (!file_put_contents($fn, $php)) {
                die("Can not include saved temp php code from $fn, you should add $tmp_dir into open_basedir!!\n");
            }
            return include($fn);
        }

        return include('data://text/plain,' . urlencode($php));
    }

    /**
     * include a compiled and saved template php file, then render with input data.
     *
     * @param string $compiled compiled template php file name
     *
     * @return string rendered result
     */
    public static function render($compiled, $data) {
        $func = include($compiled);
        return $func($data);
    }

    /**
     * Internal method used by compile(). Get function name for standalone or none standalone tempalte.
     *
     * @param array $context Current context of compiler progress.
     * @param string $name base function name
     *
     * @return string compiled Function name
     */
    protected static function _fn($context, $name) {
        return $context['flags']['standalone'] ? "\$cx['funcs']['$name']" : "LCRun::$name";
    }

    /**
     * Internal method used by compile(). Get variable names translated array, Ex: a.b.c => ["'a'", "'b'", "'c'"]
     *
     * @param array $scopes an array of variable names with single quote
     *
     * @return string PHP array names string
     */
    protected static function _scope($scopes) {
        return count($scopes) ? '[' . implode('][', $scopes) . ']' : '';
    }

    /**
     * Internal method used by compile(). Get variable names translated string, Ex: a.b.c => "['a']['b']['c']"
     *
     * @param array $scopes an array of variable names. ex: ['a', 'b', 'c', ...]
     *
     * @return string Translated variable name as input array notation.
     */
    protected static function _qscope($list) {
        return self::_scope(array_map(function ($v) {return "'$v'";}, $list));
    }

    /**
     * Internal method used by compile(). Get variable names translated string, Ex: a.b.c => "['a']['b']['c']"
     *
     * @param string $vn variable name.
     *
     * @return string Translated variable name as input array notation.
     */
    protected static function _vn($vn) {
        return $vn ? self::_qscope(explode('.', $vn)) : '';
    }

    /**
     * Internal method used by compile().
     *
     * @param mixed $v variable name to be fixed.
     * @param array $context Current compile content.
     */
    protected static function _vx(&$v, $context) {
        $v = trim($v);
        if (($v == 'this') || $v == '.') {
            if ($context['flags']['this']) {
                $v = null;
            }
        }
    }

    /**
     * Internal method used by compile().
     *
     * @param string $v variable name.
     *
     * @return mixed Variable names array or null.
     */
    protected static function _vs($v) {
        if ($v == '.') {
            return Array('.');
        }
        return $v ? explode('.', $v) : null;
    }

    /**
     * Internal method used by compile(). Find current json schema target, return childrens.
     *
     * @param array $target current json schema target
     * @param mixed $key move target to child specified with the key
     *
     * @return array children of new json schema target 
     */
    protected static function &_jst(&$target, $key = false) {
        if ($key) {
            if (!isset($target['properties'])) {
                $target['type'] = 'object';
                $target['properties'] = Array();
            }
            if (!isset($target['properties'][$key])) {
                $target['properties'][$key] = Array();
            }
            return $target['properties'][$key];
        } else {
            if (!isset($target['items'])) {
                $target['type'] = 'array';
                $target['items'] = Array();
            }
            return $target['items'];
        }
    }

    /**
     * Internal method used by compile(). Find current json schema target, prepare target parent.
     *
     * @param array $context current compile context
     * @param string $var current variable name
     */
    protected static function &_jsp(&$context) {
        $target = &$context['jsonSchema'];
        foreach ($context['vars'] as $var) {
            if ($var) {
                foreach ($var as $v) {
                    $target = &self::_jst($target, $v);
                }
            }
            $target = &self::_jst($target);
        }
        return $target;
    }

    /**
     * Internal method used by compile(). Define a json schema string/number with the variable name.
     *
     * @param array $context current compile context
     * @param string $var current variable name
     */
    protected static function _jsv(&$context, $var) {
        $target = &self::_jsp($context);
        $vs = self::_vs($var);
        if (is_array($vs)) {
            foreach ($vs as $v) {
                $target = &self::_jst($target, $v);
            }
        }
        $target['type'] = Array('string', 'number');
        $target['required'] = true;
    }

    /**
     * Internal method used by compile(). Collect handlebars usage information, detect template error.
     *
     * @param string $token detected handlebars {{ }} token
     * @param string $context current scaning context
     */
    protected static function scan($token, &$context) {
        $head = substr($token[2], 0, 1);
        $act = substr($token[2], 1);
        $raw = ($token[1] === '{{{');

        if (count($token[1]) !== count($token[3])) {
            $context['error'] = "Bad token {$token[1]}{$token[2]}{$token[3]} ! Do you mean {{}} or {{{}}}?";
            return;
        }

        if ($raw) {
            if (preg_match('/\\^|\\/|#/', $head)) {
                $context['error'] = "Bad token {$token[1]}{$token[2]}{$token[3]} ! Do you mean \{\{{$token[2]}\}\}?";
                return;
            }
        }

        switch ($head) {
        case '^':
            $context['stack'][] = $token[2];
            $context['level']++;
            return $context['usedFeature']['isec']++;

        case '/':
            array_pop($context['stack']);
            $context['level']--;
            return;

        case '#':
            $context['stack'][] = $token[2];
            $context['level']++;
            $acts = explode(' ', $act);
            switch ($acts[0]) {
            case 'with':
                if (isset($acts[1]) && !$context['flags']['with']) {
                    $context['error'] = 'do not support {{#with var}}, you should do compile with LightMustache::FLAG_WITH flag';
                }
            case 'each':
            case 'unless':
            case 'if':
                return $context['usedFeature'][$acts[0]]++;

            case 'yrb':
                return $context['usedFeature']['yrb']++;

            default:
                return $context['usedFeature']['sec']++;
            }

        case '!':
            return $context['usedFeature']['comment']++;

        default:
            $fn = $raw ? 'raw' : 'enc';
            $context['usedFeature'][$fn]++;
            $token[2] = trim($token[2]);
            switch ($token[2]) {
                case 'else':
                    return $context['usedFeature']['else']++;

                case 'this':
                    if ($context['level'] == 0) {
                        $context['usedFeature']['rootthis']++;
                    }
                    if (!$context['flags']['this']) {
                        $context['error'] = 'do not support {{this}}, you should do compile with LightMustache::FLAG_THIS flag';
                    }
                    return $context['usedFeature']['this']++;

                case '.':
                    if ($context['level'] == 0) {
                        $context['usedFeature']['rootthis']++;
                    }
                    if (!$context['flags']['this']) {
                        $context['error'] = 'do not support {{.}}, you should do compile with LightMustache::FLAG_THIS flag';
                    }
                    return $context['usedFeature']['dot']++;

                default:
                    if (preg_match('/\\.\\.(\\/.+)*/', $token[2])) {
                        if (!$context['flags']['parent']) {
                            $context['error'] = 'do not support {{../var}}, you should do compile with LightMustache::FLAG_PARENT flag';
                        }
                        return $context['usedFeature']['parent']++;
                    }
            }
            if ($context['level'] == 0) {
                $context['usedFeature']['rootvar']++;
            }
        }
    }

    /**
     * Internal method used by compile(). Return compiled PHP code partial for a handlebars token.
     *
     * @param string $token detected handlebars {{ }} token
     * @param string $context current scaning context
     */
    public static function tokens($token, &$context) {
        $head = substr($token[2], 0, 1);
        $act = substr($token[2], 1);
        $raw = ($token[1] === '{{{');

        switch ($head) {
        case '^':
            $context['stack'][] = $act;
            $context['stack'][] = '^';
            if ($context['useVar']) {
                $actParts = explode(".", $act);
                $actStr = '';
                foreach ($actParts as $actPart) {
                    $actStr .=  "['{$actPart}']";
                }

                //When the variable contains a '.' then it needs to be evaluated with respect to the global context
                if (count($actParts) == 1) {
                    $v = end($context['useVar']) . $actStr;
                    return "{$context['ops']['cnd_start']}(@$v === '' || empty($v) || @$v === false){$context['ops']['cnd_then']}"; 
                }
                else {
                    return $context['ops']['seperator'] . self::_fn($context, 'ifcond') . "('$act', \$cx, \$in, function(\$cx, \$in) {{$context['ops']['f_start']}";
                }
            } else {
                return "{$context['ops']['cnd_start']}(" . self::_fn($context, 'isec') . "('$act', \$cx, \$in)){$context['ops']['cnd_then']}";
            }
        case '/':
            $each = false;
            switch ($act) {
            case 'if':
            case 'unless':
                $pop = array_pop($context['stack']);
                if ($pop == ':') {
                    $pop = array_pop($context['stack']);
                    return $context['usedFeature']['parent'] ? "{$context['ops']['f_end']}}){$context['ops']['seperator']}" : "{$context['ops']['cnd_end']}";
                }
                return $context['usedFeature']['parent'] ? "{$context['ops']['f_end']}}){$context['ops']['seperator']}" : "{$context['ops']['cnd_else']}''{$context['ops']['cnd_end']}";
            case 'with':
                $pop = array_pop($context['stack']);
                if ($pop !== 'with') {
                   $context['error'] = 'Unexpect token /with !';
                   return;
                }
                return "{$context['ops']['f_end']}}){$context['ops']['seperator']}";
            case 'yrb':
                return "{$context['ops']['f_end']}}){$context['ops']['seperator']}";
            case 'each':
                $each = true;
            default:
                self::_vx($act, $context);
                array_pop($context['vars']);
                $pop = array_pop($context['stack']);
                switch($pop) {
                case '#':
                case '^':
                    $pop2 = array_pop($context['stack']);
                    if (!$each && ($pop2 !== $act)) {
                        $context['error'] = "Unexpect token {$token[2]} ! Previous token $pop$pop2 is not closed";
                        return;
                    }
                    if ($pop == '^') {
                        //The variable contains a '.'
                        if (!preg_match('/(.+?)\\.(.+)/', $act, $matched)) {
                            return $context['usedFeature']['parent'] ? "{$context['ops']['f_end']}}){$context['ops']['seperator']}" : "{$context['ops']['cnd_else']}''{$context['ops']['cnd_end']}";
                        }
                    }
                    return "{$context['ops']['f_end']}}){$context['ops']['seperator']}";
                default:
                    $context['error'] = "Unexpect token: {$token[2]} !";
                    return;
                }
            }
        case '#':
            $each = 'false';
            $acts = explode(' ', $act);
            switch ($acts[0]) {
            case 'if':
                $context['stack'][] = 'if';
                self::_vx($acts[1], $context);
                return $context['usedFeature']['parent'] 
                       ? $context['ops']['seperator'] . self::_fn($context, 'ifv') . "('{$acts[1]}', \$cx, \$in, function(\$cx, \$in) {{$context['ops']['f_start']}"
                       : "{$context['ops']['cnd_start']}(" . self::_fn($context, 'ifvar') . "('{$acts[1]}', \$cx, \$in)){$context['ops']['cnd_then']}";
            case 'unless':
                $context['stack'][] = 'unless';
                self::_vx($acts[1], $context);
                return $context['usedFeature']['parent']
                       ? $context['ops']['seperator'] . self::_fn($context, 'unl') . "('{$acts[1]}', \$cx, \$in, function(\$cx, \$in) {{$context['ops']['f_start']}"
                       : "{$context['ops']['cnd_start']}(!" . self::_fn($context, 'ifvar') . "('{$acts[1]}', \$cx, \$in)){$context['ops']['cnd_then']}";
            case 'each':
                $each = 'true';
            case 'with':
                $act = $acts[1];
            case 'yrb':
                return $context['ops']['seperator'] . self::_fn($context, 'yrb') . "('', \$cx, \$in, function(\$cx, \$in) {{$context['ops']['f_start']}";
            default:
                self::_vx($act, $context);
                $context['vars'][] = self::_vs($act);
                if (($acts[0] === 'with') && $context['flags']['with']) {
                    $context['stack'][] = 'with';
                    return $context['ops']['seperator'] . self::_fn($context, 'wi') . "('{$acts[1]}', \$cx, \$in, function(\$cx, \$in) {{$context['ops']['f_start']}";
                }
                self::_jsp($context);
                $context['stack'][] = $act;
                $context['stack'][] = '#';
                return $context['ops']['seperator'] . self::_fn($context, 'sec') . "('$act', \$cx, \$in, $each, function(\$cx, \$in) {{$context['ops']['f_start']}";
            }
        case '!':
            return $context['ops']['seperator'];
        default:
            self::_vx($token[2], $context);
            if ($token[2] ==='else') {
                $context['stack'][] = ':';
                return $context['usedFeature']['parent'] ? "{$context['ops']['f_end']}}, function(\$cx, \$in) {{$context['ops']['f_start']}" : "{$context['ops']['cnd_else']}";
            }
            self::_jsv($context, $token[2]);
            $fn = $raw ? 'raw' : 'enc';
            if ($context['useVar']) {
                $v = end($context['useVar']) . self::_vn($token[2]);
                if ($context['flags']['jstrue']) {
                    return $raw ? "{$context['ops']['cnd_start']}($v === true){$context['ops']['cnd_then']}'true'{$context['ops']['cnd_else']}$v{$context['ops']['cnd_end']}" : "{$context['ops']['cnd_start']}($v === true){$context['ops']['cnd_then']}'true'{$context['ops']['cnd_else']}htmlentities(@$v, ENT_QUOTES){$context['ops']['cnd_end']}";
                } else {
                    //In case of {{{.}}}
                    if ($token[2] == '') {
                        return "{$context['ops']['seperator']}@$v{$context['ops']['seperator']}";
                    }
                    else if (preg_match('/(.+?)\\.(.+)/', $token[2], $matched)) {
                        return $raw ? "{$context['ops']['seperator']}" . self::_fn($context, 'evalVar') . "('{$token[2]}', \$cx, \$in)" . "{$context['ops']['seperator']}" : "{$context['ops']['seperator']}" . self::_fn($context, 'evalVar') . "('{$token[2]}', \$cx, \$in, true)" . "{$context['ops']['seperator']}";
                    }
                    else {
                        return $raw ? "{$context['ops']['seperator']}@$v{$context['ops']['seperator']}" : "{$context['ops']['seperator']}htmlentities(@$v, ENT_QUOTES){$context['ops']['seperator']}"; 
                    }
                }
            } else {
                return $context['ops']['seperator'] . self::_fn($context, $fn) . "('{$token[2]}', \$cx, \$in){$context['ops']['seperator']}";
            }
        }
    }
}

/**
 * LightMustache static class for compiled template runtime methods.
 */
class LCRun {
    public static $yrb; 
    public static $locale;

    /**
     * LightMustache runtime method initialization
     */
    public static function init($yrb, $locale) {
        self::$yrb = $yrb;
        self::$locale = $locale;
    }

    /**
     * LightMustache runtime method for {{#if var}}
     *
     * @param string $var variable name to be tested
     * @param array $cx render time context
     * @param array $in input data with current scope
     *
     * @return boolean Return true when the value is not null nor false.
     */
    public static function ifvar($var, $cx, $in) {
        $v = self::val($var, $cx, $in);
        return !is_null($v) && ($v !== false) && ($v !== 0) && ($v !== '') && (is_array($v) ? (count($v) > 0) : true);
    }

    /**
     * LightMustache runtime method for {{#if var}} when {{../var}} used
     *
     * @param string $var variable name to be tested
     * @param array $cx render time context
     * @param array $in input data with current scope
     * @param function $truecb callback function when test result is true
     * @param function $falsecb callback function when test result is false
     *
     * @return string The rendered string of the section
     */
    public static function ifv($var, $cx, $in, $truecb, $falsecb = null) {
        $v = self::val($var, $cx, $in);
        $ret = '';
        if (is_null($v) || ($v === false) || ($v === '0') || ($v === 0) || ($v === '') || (is_array($v) && (count($v) == 0))) {
            if ($falsecb) {
                $cx['scopes'][] = $in;
                $ret = $falsecb($cx, $in);
                array_pop($cx['scopes']);
            }
        } else {
            if ($truecb) {
                $cx['scopes'][] = $in;
                $ret = $truecb($cx, $in);
                array_pop($cx['scopes']);
            }
        }
        return $ret;
    }

    /**
     * LightMustache runtime method for {{$unless var}} when {{../var}} used
     *
     * @param string $var variable name to be tested
     * @param array $cx render time context
     * @param array $in input data with current scope
     *
     * @return boolean Return true when the value is not null nor false.
     */
    public static function unl($var, $cx, $in, $truecb, $falsecb = null) {
        return self::ifv($var, $cx, $in, $falsecb, $truecb);
    }

    /**
     * LightMustache runtime method for {{^var}} inverted section
     *
     * @param string $var variable name to be tested
     * @param array $cx render time context
     * @param array $in input data with current scope
     *
     * @return boolean Return true when the value is not null nor false.
     */
    public static function isec($var, $cx, $in) {
        $v = self::val($var, $cx, $in);
        return ($v === '') || empty($v) || ($v === false);
    }

    /**
     * LightMustache runtime method to get input value
     *
     * @param string $var variable name to get the raw value
     * @param array $cx render time context
     * @param array $in input data with current scope
     *
     * @return mixed The raw value of the specified variable
     */
    public static function val($var, $cx, $in) {
        //echo "Inside val \nvar " . $var . "\n cx " . print_r($cx, 1) . "\n in " . print_r($in, 1) ."\n";
        $levels = 0;
        if ($var === '@index') {
            return $cx['sp_vars']['index'];
        }
        if ($var === '@key') {
            return $cx['sp_vars']['key'];
        }
        $var = preg_replace_callback('/\\.\\.\\//', function($matches) use (&$levels) {
            $levels++;
            return '';
        }, $var);
        if ($levels > 0) {
            $pos = count($cx['scopes']) - $levels;
            if ($pos >= 0) {
                $in = $cx['scopes'][$pos];
            } else {
                return '';
            }
        }
        if (preg_match('/(.+?)\\.(.+)/', $var, $matched)) {
            if (array_key_exists($matched[1], $in)) {
                return self::val($matched[2], $cx, $in[$matched[1]]);
            } else {
                return null;
            }
        }
        return ($var === '') ? $in : (is_array($in) && isset($in[$var]) ? $in[$var] : null);
    }

    /**
     * LightMustache runtime method for {{{var}}}
     *
     * @param string $var variable name to get the raw value
     * @param array $cx render time context
     * @param array $in input data with current scope
     * @param boolean $loop true when in loop
     *
     * @return mixed The raw value of the specified variable
     */
    public static function raw($var, $cx, $in, $loop = false) {
        $v = self::val($var, $cx, $in);
        if ($v === true) {
            if ($cx['flags']['jstrue']) {
                return 'true';
            }
        }

        if ($loop && ($v === false)) {
            if ($cx['flags']['jstrue']) {
                return 'false';
            }
        }

        if (is_array($v)) {
            if ($cx['flags']['jsobj']) {
                if (count(array_diff_key($v, array_keys(array_keys($v)))) > 0) {
                    return '[object Object]';
                } else {
                    $ret = Array();
                    foreach ($v as $k => $vv) {
                        $ret[] = self::raw($k, $cx, $v, true);
                    }
                    return join(',', $ret);
                }
            }
        }
        return $v;
    }

    /**
     * LightMustache runtime method for {{var}}
     *
     * @param string $var variable name to get the htmlencoded value
     * @param array $cx render time context
     * @param array $in input data with current scope
     *
     * @return mixed The htmlencoded value of the specified variable
     */
    public static function enc($var, $cx, $in) {
        return htmlentities(self::raw($var, $cx, $in), ENT_QUOTES);
    }

    /**
     * LightMustache runtime method for {{#var}} section
     *
     * @param string $var variable name for section
     * @param array $cx render time context
     * @param array $in input data with current scope
     * @param boolean $each true when rendering #each
     * @param function $cb callback function to render child context
     *
     * @return string The rendered string of the section
     */
    public static function sec($var, &$cx, $in, $each, $cb) {
        if (preg_match('/(.+?)\\.(.+)/', $var, $matched)) {
            $v = self::evalVar($var, $cx, $in);
        }
        else {
            $v = self::val($var, $cx, $in);    
        }
        $isary = is_array($v);
        $loop = $each;
        //echo "variable " . $var . " cx " . print_r($cx,1) . " data " . print_r($in,1) . " value " . print_r($v,1) . " ****";
        if (!$loop && $isary) {
            $loop = (count(array_diff_key($v, array_keys(array_keys($v)))) == 0);
        }
        if ($loop && $isary) {
            if ($each) {
                $is_obj = count(array_diff_key($v, array_keys(array_keys($v)))) > 0;
            } else {
                $is_obj = false;
            }
            $ret = Array();
            $cx['scopes'][] = $in;
            foreach ($v as $index => $raw) {
                $cx['sp_vars'][$is_obj ? 'key' : 'index'] = $index;
                $ret[] = $cb($cx, $raw);
            }
            unset($cx['sp_vars'][$is_obj ? 'key' : 'index']);
            array_pop($cx['scopes']);
            return join('', $ret);
        }
        if ($each) {
            return '';
        }
        if ($isary) {
            $cx['scopes'][] = $v;
            $ret = $cb($cx, $v);
            array_pop($cx['scopes']);
            return $ret;
        }
        if ($v === true) {
            return $cb($cx, $in);
        }
        if (!is_array($v) && $v != '') {
            return $cb($cx, $in);
        }
        if (!empty($v) && ($v !== false) && $v !== '') {
            return $cb($cx, $v);
        }
        return '';
    }

    /**
     * LightMustache runtime method for {{#yrb}} section
     *
     * @param string $var variable name for section
     * @param array $cx render time context
     * @param array $in input data with current scope
     * @param function $cb callback function to render child context
     *
     * @return string The rendered string of the section
     */
    public static function yrb($var, $cx, $in, $cb) {
        $text = $cb($cx, $in);
        //echo "cx " . print_r($cx, 1) . " in " . print_r($in, 1) . "text " . $text;
        $str_arr = explode("|", $text);

        // Without processing MessageFormatter
        if (count($str_arr) == 1)
            return self::$yrb->get_string($text);

        // Format string using current locale
        $fmt_str = array_shift($str_arr);
        $fmt = msgfmt_create(self::$locale, self::$yrb->get_string($fmt_str));
        if ($fmt == null) {
            error_log("Formatting error : yala key fmt_str " . $fmt_str);
        }
        //echo "locale " . self::$locale . " string " . self::$yrb->get_string($fmt_str);
        return msgfmt_format($fmt, $str_arr);
    }

    /**
     * LightMustache runtime method for {{#with var}}
     *
     * @param string $var variable name for section
     * @param array $cx render time context
     * @param array $in input data with current scope
     * @param function $cb callback function to render child context
     *
     * @return string The rendered string of the token
     */
    public static function wi($var, &$cx, $in, $cb) {
        $v = self::val($var, $cx, $in);
        if (($v === false) || ($v === null)) {
            return '';
        }
        $cx['scopes'][] = $in;
        $ret = $cb($cx, $v);
        array_pop($cx['scopes']);
        return $ret;
    }

    /**
     * LightMustache runtime method for {{#foo.bar}} kind of conditional expression
     *
     * @param string $var variable name for section
     * @param array $cx render time context
     * @param array $in input data with current scope
     * @param function $cb callback function to render child context
     *
     * @return string The rendered string of the token
     */
    public static function ifcond($var, &$cx, $in, $cb) {
        $v = self::evalVar($var, $cx, $in);
        $ret = '';
        if (empty($v)) {
            if ($cb) {
                $cx['scopes'][] = $in;
                $ret = $cb($cx, $in);
                array_pop($cx['scopes']);
            }
        } 
        return $ret;          
    }

    /**
     * LightMustache runtime method to recursive find a variable in global scope
     *
     * @param string $arr global context 
     * @param array $k1 first key
     * @param array $k2 second key
     *
     * @return string The value of $arr[$k1][$k2] if found and '' otherwise
     */
    public static function _rfind($arr, $k1, $k2) {
        if (!is_array($arr)) return '';

        if (isset($arr[$k1])) {
            if (is_array($arr[$k1]) && isset($arr[$k1][$k2])) {
                return $arr[$k1][$k2];
            }
        }
        else {
            $ret = '';
            foreach($arr as $k => $ele) {
                $ret = self::_rfind($ele, $k1, $k2);
                if (!empty($ret)) {
                    break;
                }
            }
            return $ret;
        }
        return '';
    }

    /**
     * LightMustache util method to look for a key recursively in a multi dim array
     *
     * @param string $needle The keyname which needs to be searched in the array
     * @param array $haystack The array in which the key needs to be searched
     * 
     * @return array The subarray of $haystack that contains the key $needle, false if not found
     */
    public static function recursive_array_search($needle, $haystack) {
        $ret = false;
        foreach($haystack as $key=>$value) {
            if (isset($haystack[$needle])) {
                $ret = $haystack;
                break;
            }
            else if(is_array($value)) {
                $ret = self::recursive_array_search($needle,$value);
            }
        }
        return $ret;
    }

    /**
     * LightMustache runtime method for {{var}} containing '.'
     *
     * @param string $var variable name for section
     * @param array $cx render time context
     * @param array $in input data with current scope
     *
     * @return string The rendered string of the token
     */
    public static function evalVar($var, $cx, $in, $htmlentities=false) {
        $parts = explode(".", $var);
        if (count($parts) > 2) {
            $val = self::recursive_array_search($parts[0], $in);
            if ($val == false) {
                $val = self::recursive_array_search($parts[0], $cx['scopes'][0]);    
            }
            foreach($parts as $part) {
                $val = @$val[$part];
            }
            $ret = $val;
        }
        else if (count($parts) > 1) {
            $ret = self::_rfind($in, $parts[0], $parts[1]);
            if ($ret == false) {
                $ret = self::_rfind($cx['scopes'][0], $parts[0], $parts[1]);
            }
        }
        else {
            $ret = @$in[$var];
        }

        if ($htmlentities == true) {
            return htmlentities($ret, ENT_QUOTES);
        }
        else {
            return $ret;
        }
    }
}
?>


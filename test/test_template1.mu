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

Test case 10 :
Check of conditional expression for variable of the form arr.e2. Output should be else block
{{#arr.e2}}if block{{/arr.e2}}
{{^arr.e2}}else block{{/arr.e2}} value is {{{arr.e2}}}

Test case 11:
Printing an nonassoc array with {{.}}
{{#nonassoc_arr}}
    {{.}}
{{/nonassoc_arr}}

Test case 12:
Printing an assoc array 
{{#assoc_arr}}
    {{var}}
{{/assoc_arr}}


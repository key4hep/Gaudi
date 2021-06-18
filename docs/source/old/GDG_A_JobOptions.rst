.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapJobo:

Appendix C Job Options Grammar and Error Codes
==============================================

C.1   The EBNF grammar of the Job Options files
-----------------------------------------------

    The syntax of the Job-Options-File is defined through the following
    EBNF-Grammar.

    Job-Options-File = {Statements} .

    Statements = {Include-Statement} \| {Assign-Statement} \| {Append-Statement} \|
    {Platform-Dependency} .

    AssertableStatements = {Include-Statement} \| {Assign-Statement} \| {Append-Statement} .

    AssertionStatement = '#ifdef' \| '#ifndef' .

    Platform-Dependency = AssertionStatement 'WIN32' <AsertableStatements> [ #else
    <AssertableStatements> ] #endif

    Include-Statement = \`#include' string .

    Assign-Statement = Identifier \`.' Identifier \`=' value \`;' .

    Append-Statement = Identifier \`.' Identifier \`+=' value \`;' .

    Identifier = letter {letter \| digit} .

    value = boolean \| integer \| double \| string \| vector .

    vector = \`{' vectorvalue { \`,' vectorvalue } \`}' .

    vectorvalue = boolean \| integer \| double \| string .

    boolean = \`true' \| \`false' .

    integer = prefix scientificdigit .

    double = | ( prefix <digit> \`.' [ scientificdigit ] ) \| |  ( prefix \`.' scientificdigit ) .

    string = \`"' {char} \`"' .

    scientificdigit = < digit> [ ( \`e' \| \`E' ) < digit> ] .

    digit = <figure> .

    prefix = [ \`+' \| \`-' ] .

    figure = \`0' \| \`1' \| \`2' \| \`3' \| \`4' \| \`5' \| \`6' \| \`7' \| \`8'
    \| \`9'.

    char = any character from the ASCII-Code

    letter = set of all capital- and non-capital letter

C.2   Job Options Error Codes and Error Messages
------------------------------------------------

    The table below lists the error codes and error messages that the
    Job Options compiler may generate, their reason and how to avoid
    them.

    .. table:: Possible Error-Codes

        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error-Code                    | Reason                                        | How to avoid it                               |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #000 Internal           | -                                             | This,code normally should never appear.       |
        | compiler error                |                                               | If this code is shown there is maybe,         |
        |                               |                                               | a problem with your memory, your              |
        |                               |                                               | disk-space or the property-file is,           |
        |                               |                                               | corrupted.                                    |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #001 Included           | - wrong path in #include-directive,           | Please check if any of the listed             |
        | property-file does            | - wrong file or mistyped filename,            | reasons occured in your case.                 |
        | not exists or can             | - file is exclusively locked by               |                                               |
        | not be opened                 |   another application,                        |                                               |
        |                               | - no memory available to open this file       |                                               |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Warning #001 File already     | The file was already included by another      | Remove the #include-directive                 |
        | included by another file      | file and will not be included a second        |                                               |
        |                               | time.,The compiler will ignore this           |                                               |
        |                               | #include-directive and will continue          |                                               |
        |                               | with the next statement.                      |                                               |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #002 syntax error:      | The compiler expected an object at the        | Maybe you mistyped the name of the            |
        | Object expected               | given position.                               | object or the object contains                 |
        |                               |                                               | unknown characters or does not fit            |
        |                               |                                               | the given rules.                              |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #003 syntax error:      | The compiler expect a dot between the         | Check if the dot between the Object           |
        | Missing dot between           | Object and the Propertyname.                  | and the Propertyname is missing.              |
        | Object and Propertyname       |                                               |                                               |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #004 syntax error:      | The compiler expected an identifier at        | Maybe you mistyped the name of the            |
        | Identifier expected           | the given position.                           | identifier or the identifier contains         |
        |                               |                                               | unknown characters or does not fit the        |
        |                               |                                               | given rules.                                  |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #005 syntax error:      | The compiler expected an operator             | Check if there is a valid operator            |
        | Missing operator '+='         | between the Propertyname and the value.       | after the Propertyname. Note that             |
        | or '='                        |                                               | a blank or tab is not allowed between         |
        |                               |                                               | '+='!                                         |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #006 String is not      | A string (value) was not terminated by a ".   | Check,if all your strings are beginning       |
        | terminated by a "             |                                               | and ending with ". Note that the,             |
        |                               |                                               | position given by the compiler can be         |
        |                               |                                               | wrong because the compiler may,               |
        |                               |                                               | thought that following statements             |
        |                               |                                               | are part of the string!                       |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #007 syntax error:      | The next token after the #include is not      | Make sure that after the #include-directive   |
        | #include-statement is         | a string.                                     | there is specified the file to include.       |
        | not correct                   |                                               | The file must be defined as a string!         |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #008 syntax error:      | The include-directive was terminated by a ;   | Remove the ; after the #include-directive.    |
        | #include does not end         |                                               |                                               |
        | with a ;                      |                                               |                                               |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #009 syntax error:      | One or more values within a vector were not   | Check,if every value in the vector is         |
        | Values must be separated      | separated with a ',' or one ore more values   | separated by a ','. If so the reason,         |
        | with ','                      | within a vector are mistyped.                 | for this message may result in mistyped       |
        |                               |                                               | values in the vector (maybe,there is a        |
        |                               |                                               | blank or tab between numbers).                |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #010 syntax error:      | The closing bracket is missing or the vector  | Check, if the vector ends with a '}' and      |
        | Vector must end with '}'      | is not terminated correctly.                  | if there is no semicolon before the           |
        |                               |                                               | ending-bracket.                               |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #011 syntax error:      | The statement is not terminated correctly.    | Check if the statement ends with a            |
        | Statement must end with a ;   |                                               | semicolon ';'.                                |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Runtime-Error #012: Cannot    | The compiler cannot append the values to      | Check,if the refered object is defined        |
        | append to object because      | the object.propertyname because the           | in one of the included files, if so,          |
        | it does not exists            | object does not exist.                        | check if you writed the object-name           |
        |                               |                                               | exactly like in the include-file.             |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Runtime-Error #013 Cannot     | The compiler cannot append the values to      | Check,if there was already something          |
        | append to object because      | the object.propertyname because the           | assigned to the refered property (in the,     |
        | Property does not exists      | property does not exist.                      | include-file or in the current file).         |
        |                               |                                               | If not then modify the, append-statement      |
        |                               |                                               | into a assign-statement. If there was         |
        |                               |                                               | already something assigned, check if the      |
        |                               |                                               | object-name and the property-name are typed   |
        |                               |                                               | correctly.                                    |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #014 Elements in the    | One,or more elements in the vector have       | Check declaration of vector, check the types  |
        | vector are not of the same    | a different type than the first,element       | and check, if maybe a value is mistyped.      |
        | type                          | in the vector. All elements must have         |                                               |
        |                               | the same type like the,first declarated       |                                               |
        |                               | element.                                      |                                               |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #015 Value(s) expected  | The compiler didn't find values to            | Check the statement if there exists values    |
        |                               | append or assign                              | and if they are written correctly. Maybe      |
        |                               |                                               | this error is a result of a previous error!   |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+
        | Error #016 Specified          | The compiler was not able to include a        | Check,if you are using enviornment-variables  |
        | property-file does not exist  | property-file or didn't found the file.       | to resolve the file, if they are,mistyped     |
        | or can not be resolved        | A reason can be that the compiler was         | (wether in the system or in the               |
        |                               | not able to resolve an,environment-variable   | #include-directive) or not set,correctly.     |
        |                               | which points to the location of the           |                                               |
        |                               | property-file.                                |                                               |
        +-------------------------------+-----------------------------------------------+-----------------------------------------------+

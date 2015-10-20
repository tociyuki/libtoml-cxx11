TOML decoder and encoder
========================

This is an implementation TOML v0.4.0 decoder and encoder
written by C++11.

Version
------

0.1.0

Build
-----

    $ make all-test

Clean
-----

    $ make clean

Grammar
------

    toml    :
            | statements
            | statements sections
            | sections

    sections
            : sections '[' keypath ']' ENDLINE statements
            | sections '[' '[' keypath ']' ']' ENDLINE statements
            | sections '[' keypath ']' ENDLINE
            | sections '[' '[' keypath ']' ']' ENDLINE
            | '[' keypath ']' ENDLINE statements
            | '[' '[' keypath ']' ']' ENDLINE statements
            | '[' keypath ']' ENDLINE
            | '[' '[' keypath ']' ']' ENDLINE

    keypath : keypath '.' key
            | key

    key     : BAREKEY
            | STRKEY

    statements
            : statements pair ENDLINE
            | pair ENDLINE

    pair    : key '=' value     { turn on scanner key state }

    value   : BOOLEAN
            | INTEGER
            | FLOAT
            | DATETIME
            | STRING
            | STRKEY
            | '[' list ']'
            | '{' table '}'     { turn on scanner value state }

    array   : optional_endline
            | value_list
            | value_list ',' optional_endline

    value_list
            : optional_endline value optional_endline
            | value_list ',' optional_endline value optional_endline

    optional_endline
            :
            | ENDLINE

    table   :
            | pair_list
            | pair_list ','

    pair_list
            : pair
            | pair_list ',' pair

License
------

The BSD 3-Clause

Copyright (c) 2015, MIZUTANI Tociyuki
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 3. Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

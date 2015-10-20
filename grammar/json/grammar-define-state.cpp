// autogenerated by rzubr
//case  0: // start: value
//case  1: // value: SCALAR
//case  2: // value: STRING
//case  3: // value: "[" array "]"
//case  4: // value: "{" table "}"
//case  5: // value: "[" "]"
//case  6: // value: "{" "}"
//case  7: // array: array "," value
//case  8: // array: value
//case  9: // table: table "," STRING ":" value
//case 10: // table: STRING ":" value

    void
    define_state ()
    {
        enum { start = 10, value, array, table };

        to (1, TOKEN_SCALAR, 3);
        to (1, TOKEN_STRING, 4);
        to (1, TOKEN_LBRACKET, 5);
        to (1, TOKEN_LBRACE, 6);
        to (1, value, 2);

        to (2, TOKEN_ENDMARK, 255);

        to (3, TOKEN_ENDMARK, 253);
        to (3, TOKEN_RBRACKET, 253);
        to (3, TOKEN_COMMA, 253);
        to (3, TOKEN_RBRACE, 253);

        to (4, TOKEN_ENDMARK, 252);
        to (4, TOKEN_RBRACKET, 252);
        to (4, TOKEN_COMMA, 252);
        to (4, TOKEN_RBRACE, 252);

        to (5, TOKEN_RBRACKET, 8);
        to (5, TOKEN_SCALAR, 3);
        to (5, TOKEN_STRING, 4);
        to (5, TOKEN_LBRACKET, 5);
        to (5, TOKEN_LBRACE, 6);
        to (5, array, 7);
        to (5, value, 9);

        to (6, TOKEN_RBRACE, 11);
        to (6, TOKEN_STRING, 12);
        to (6, table, 10);

        to (7, TOKEN_RBRACKET, 13);
        to (7, TOKEN_COMMA, 14);

        to (8, TOKEN_ENDMARK, 249);
        to (8, TOKEN_RBRACKET, 249);
        to (8, TOKEN_COMMA, 249);
        to (8, TOKEN_RBRACE, 249);

        to (9, TOKEN_RBRACKET, 246);
        to (9, TOKEN_COMMA, 246);

        to (10, TOKEN_RBRACE, 15);
        to (10, TOKEN_COMMA, 16);

        to (11, TOKEN_ENDMARK, 248);
        to (11, TOKEN_RBRACKET, 248);
        to (11, TOKEN_COMMA, 248);
        to (11, TOKEN_RBRACE, 248);

        to (12, TOKEN_COLON, 17);

        to (13, TOKEN_ENDMARK, 251);
        to (13, TOKEN_RBRACKET, 251);
        to (13, TOKEN_COMMA, 251);
        to (13, TOKEN_RBRACE, 251);

        to (14, TOKEN_SCALAR, 3);
        to (14, TOKEN_STRING, 4);
        to (14, TOKEN_LBRACKET, 5);
        to (14, TOKEN_LBRACE, 6);
        to (14, value, 18);

        to (15, TOKEN_ENDMARK, 250);
        to (15, TOKEN_RBRACKET, 250);
        to (15, TOKEN_COMMA, 250);
        to (15, TOKEN_RBRACE, 250);

        to (16, TOKEN_STRING, 19);

        to (17, TOKEN_SCALAR, 3);
        to (17, TOKEN_STRING, 4);
        to (17, TOKEN_LBRACKET, 5);
        to (17, TOKEN_LBRACE, 6);
        to (17, value, 20);

        to (18, TOKEN_RBRACKET, 247);
        to (18, TOKEN_COMMA, 247);

        to (19, TOKEN_COLON, 21);

        to (20, TOKEN_RBRACE, 244);
        to (20, TOKEN_COMMA, 244);

        to (21, TOKEN_SCALAR, 3);
        to (21, TOKEN_STRING, 4);
        to (21, TOKEN_LBRACKET, 5);
        to (21, TOKEN_LBRACE, 6);
        to (21, value, 22);

        to (22, TOKEN_RBRACE, 245);
        to (22, TOKEN_COMMA, 245);

        nrhs.assign ({1, 1, 1, 3, 3, 2, 2, 3, 1, 5, 3});

        colgoto.assign ({start, value, value, value, value, value, value, array, array, table, table});

    }
#include "taptests.hpp"
#include "value.hpp"
#include "yaml.hpp"
#include "json.hpp"

struct testspec {
    char const* name;
    char const* input;
    char const* expected;
} spec[]{

{
"Example 2.1.  Sequence of Scalars",
R"__(
- Mark McGwire
- Sammy Sosa
- Ken Griffey
)__",
R"__(["Mark McGwire","Sammy Sosa","Ken Griffey"])__"
},

{
"Example 2.2.  Mapping Scalars to Scalars",
R"__(
hr:  65    # Home runs
avg: 0.278 # Batting average
rbi: 147   # Runs Batted In
)__",
R"__({"avg":0.278,"hr":65,"rbi":147})__"
},

{
"Example 2.3. Mapping Scalars to Sequences",
R"__(
american:
  - Boston Red Sox
  - Detroit Tigers
  - New York Yankees
national:
  - New York Mets
  - Chicago Cubs
  - Atlanta Braves
)__",
R"__({)__"
  R"__("american":["Boston Red Sox","Detroit Tigers","New York Yankees"],)__"
  R"__("national":["New York Mets","Chicago Cubs","Atlanta Braves"]})__"
},

{
"Example 2.4. Sequence of Mappings",
R"__(
-
  name: Mark McGwire
  hr:   65
  avg:  0.278
-
  name: Sammy Sosa
  hr:   63
  avg:  0.288
)__",
R"__([)__"
  R"__({"avg":0.278,"hr":65,"name":"Mark McGwire"},)__"
  R"__({"avg":0.288,"hr":63,"name":"Sammy Sosa"}])__"
},

{
"Example 2.5. Sequence of Sequences",
R"__(
- [name        , hr, avg  ]
- [Mark McGwire, 65, 0.278]
- [Sammy Sosa  , 63, 0.288]
)__",
R"__([)__"
  R"__(["name","hr","avg"],)__"
  R"__(["Mark McGwire",65,0.278],)__"
  R"__(["Sammy Sosa",63,0.288]])__"
},

{
"Example 2.6. Mapping of Mappings",
R"__(
Mark McGwire: {hr: 65, avg: 0.278}
Sammy Sosa: {
    hr: 63,
    avg: 0.288
  }
)__",
R"__({)__"
  R"__("Mark McGwire":{"avg":0.278,"hr":65},)__"
  R"__("Sammy Sosa":{"avg":0.288,"hr":63}})__"
},

{
"Example 2.7. Two Documents in a Stream (NOT SUPPORT)",
R"__(
# Ranking of 1998 home runs
---
- Mark McGwire
- Sammy Sosa
- Ken Griffey

## Team ranking
#---
#- Chicago Cubs
#- St Louis Cardinals
)__",
R"__(["Mark McGwire","Sammy Sosa","Ken Griffey"])__"
},

{
"Example 2.8 Play by Play Feed from a Game (NOT SUPPORT)",
R"__(
#---
#time: 20:03:20
#player: Sammy Sosa
#action: strike (miss)
#...
#---
#time: 20:03:47
#player: Sammy Sosa
#action: grand slam
#...
- time: 20:03:20
  player: Sammy Sosa
  action: strike (miss)
- time: 20:03:47
  player: Sammy Sosa
  action: grand slam
)__",
R"__([)__"
  R"__({"action":"strike (miss)","player":"Sammy Sosa","time":"20:03:20"},)__"
  R"__({"action":"grand slam","player":"Sammy Sosa","time":"20:03:47"}])__"
},

{
"Example 2.9 Single Document with Two Comments",
R"__(
---
hr: # 1998 hr ranking
  - Mark McGwire
  - Sammy Sosa
rbi:
  # 1998 rbi ranking
  - Sammy Sosa
  - Ken Griffey
)__",
R"__({)__"
  R"__("hr":["Mark McGwire","Sammy Sosa"],)__"
  R"__("rbi":["Sammy Sosa","Ken Griffey"]})__"
},

{
R"__(Example 2.10 Node for "Sammy Sosa" (NOT SUPPORT))__",
R"__(
---
hr:
  - Mark McGwire
  - Sammy Sosa
  ## Following node labeled SS
  #- &SS Sammy Sosa
rbi:
  - Sammy Sosa
  #- *SS # Subsequent occurrence
  - Ken Griffey
)__",
R"__({)__"
  R"__("hr":["Mark McGwire","Sammy Sosa"],)__"
  R"__("rbi":["Sammy Sosa","Ken Griffey"]})__"
},

{
"Example 2.11 Mapping between Sequences (NOT SUPPORT)",
/* only implicit mappings */
R"__(
---
"? - Detroit Tigers\n  - Chicago cubs" :
#? - Detroit Tigers
#  - Chicago cubs
#:
  - 2001-07-23

"? [ New York Yankees,\n    Atlanta Braves ]" :
#? [ New York Yankees,
#    Atlanta Braves ]
#:
  [ 2001-07-02, 2001-08-12,
    2001-08-14 ]
)__",
R"__({)__"
  R"__("? - Detroit Tigers\n  - Chicago cubs":)__"
    R"__(["2001-07-23"],)__"
  R"__("? [ New York Yankees,\n    Atlanta Braves ]":)__"
    R"__(["2001-07-02","2001-08-12","2001-08-14"]})__"
},

{
"Example 2.12 Compact Nested Mapping",
R"__(
---
# Products purchased
- item    : Super Hoop
  quantity: 1
- item    : Basketball
  quantity: 4
- item    : Big Shoes
  quantity: 1
)__",
R"__([)__"
  R"__({"item":"Super Hoop","quantity":1},)__"
  R"__({"item":"Basketball","quantity":4},)__"
  R"__({"item":"Big Shoes","quantity":1}])__"
},

{
"Example 2.13 In literals, newlines are preserved",
R"__(
# ASCII Art
--- |
  \//||\/||
  // ||  ||__
)__",
R"__("\\\/\/||\\\/||\n\/\/ ||  ||__\n")__"
},

{
"Example 2.14 In the folded scalars, newlines become spaces",
R"__(
--- >
  Mark McGwire's
  year was crippled
  by a knee injury.
)__",
R"__("Mark McGwire's year was crippled by a knee injury.\n")__"
},

{
"Example 2.15 Folded newlines are preserved",
R"__(
>
 Sammy Sosa completed another
 fine season with great stats.

   63 Home Runs
   0.288 Batting Average

 What a year!
)__",
R"__("Sammy Sosa completed another fine season with great stats.\n)__"
 R"__(\n)__"
 R"__(  63 Home Runs\n)__"
 R"__(  0.288 Batting Average\n)__"
 R"__(\n)__"
 R"__(What a year!\n")__"
},

{
"Example 2.16 Indentation determines scope",
R"__(
name: Mark McGwire
accomplishment: >
  Mark set a major league
  home run record in 1998.
stats: |
  65 Home Runs
  0.278 Batting Average
)__",
R"__({)__"
  R"__("accomplishment":"Mark set a major league home run record in 1998.\n",)__"
  R"__("name":"Mark McGwire",)__"
  R"__("stats":"65 Home Runs\n0.278 Batting Average\n"})__"
},

{
"Example 2.17 Quoted Scalars",
R"__(
unicode: "Sosa did fine.\u263A"
control: "\b1998\t1999\t2000\n"
hex esc: "\x0d\x0a is \r\n"

single: '"Howdy!" he cried.'
quoted: ' # Not a ''comment''.'
tie-fighter: '|\-*-/|'
)__",
R"__({)__"
  R"__("control":"\b1998\t1999\t2000\n",)__"
  R"__("hex esc":"\r\n is \r\n",)__"
  R"__("quoted":" # Not a 'comment'.",)__"
  R"__("single":"\"Howdy!\" he cried.",)__"
  R"__("tie-fighter":"|\\-*-\/|",)__"
  R"__("unicode":"Sosa did fine.)__" u8"\u263A" R"__("})__"
},

{
"Example 2.18 Multi-line Flow Scalars",
R"__(
plain:
  This unquoted scalar
  spans many lines.

quoted: "So does this
  quoted scalar.\n"
)__",
R"__({)__"
  R"__("plain":"This unquoted scalar spans many lines.",)__"
  R"__("quoted":"So does this quoted scalar.\n"})__"
},

{
"Example 2.19 Integers",
R"__(
canonical: 12345
decimal: +12345
octal: 0o14
hexadecimal: 0xC
)__",
R"__({)__"
  R"__("canonical":12345,)__"
  R"__("decimal":12345,)__"
  R"__("hexadecimal":12,)__"
  R"__("octal":12})__"
},

{
"Example 2.20 Floating Point",
R"__(
canonical: 1.23015e+3
exponential: 12.3015e+02
fixed: 1230.15
negative infinity: -.inf
not a number: .NaN
)__",
R"__({)__"
  R"__("canonical":1230.15,)__"
  R"__("exponential":1230.15,)__"
  R"__("fixed":1230.15,)__"
  R"__("negative infinity":"-.inf",)__"
  R"__("not a number":".NaN"})__"
},

{
"Example 2.21 Miscellaneous",
R"__(
null:
booleans: [ true, false ]
string: '012345'
)__",
R"__({)__"
  R"__("booleans":[true,false],)__"
  R"__("null":null,)__"
  R"__("string":"012345"})__"
},

{
"Example 2.22 Timestamps",
R"__(
canonical: 2001-12-15T02:59:43.1Z
iso8601: 2001-12-14t21:59:43.10-05:00
spaced: 2001-12-14 21:59:43.10 -5
date: 2002-12-14
)__",
R"__({)__"
  R"__("canonical":"2001-12-15T02:59:43.1Z",)__"
  R"__("date":"2002-12-14",)__"
  R"__("iso8601":"2001-12-14t21:59:43.10-05:00",)__"
  R"__("spaced":"2001-12-14 21:59:43.10 -5"})__"
},

{
"Example 2.23 Various Explicit Tags (NOT SUPPORT)",
R"__(
---
#not-date: !!str 2002-04-28
not-date: 2002-04-28

#picture: !!binary |
picture: |
 R0lGODlhDAAMAIQAAP//9/X
 17unp5WZmZgAAAOfn515eXv
 Pz7Y6OjuDg4J+fn5OTk6enp
 56enmleECcgggoBADs=

#application specific tag: !something |
application specific tag: |
 The semantics of the tag
 above may be different for
 different documents.
)__",
R"__({)__"
  R"__("application specific tag":)__"
    R"__("The semantics of the tag\n)__"
     R"__(above may be different for\n)__"
     R"__(different documents.\n",)__"
  R"__("not-date":"2002-04-28",)__"
  R"__("picture":)__"
    R"__("R0lGODlhDAAMAIQAAP\/\/9\/X\n)__"
     R"__(17unp5WZmZgAAAOfn515eXv\n)__"
     R"__(Pz7Y6OjuDg4J+fn5OTk6enp\n)__"
     R"__(56enmleECcgggoBADs=\n"})__"
},

{
"Example 2.24 Global Tags (NOT SUPPORT)",
R"__(
#%TAG ! tag:clarkevans.com,2002:
--- #!shape
  # Use the ! handle for presenting
  # tag:clarkevans.com,2002:circle
- #!circle
  #center: &ORIGIN {x: 73, y: 129}
  center: {x: 73, y: 129}
  radius: 7
- #!line
  #start: *ORIGIN
  start: {x: 73, y: 129}
  finish: { x: 89, y: 102 }
- #!label
  #start: *ORIGIN
  start: {x: 73, y: 129}
  color: 0xFFEEBB
  text: Pretty vector drawing.
)__",
R"__([)__"
  R"__({"center":{"x":73,"y":129},"radius":7},)__"
  R"__({"finish":{"x":89,"y":102},"start":{"x":73,"y":129}},)__"
  R"__({)__"
    R"__("color":16772795,)__"
    R"__("start":{"x":73,"y":129},)__"
    R"__("text":"Pretty vector drawing."}])__"
},

{
"Example 2.25 Unordered Sets (NOT SUPPORT)",
R"__(
# Sets are represented as a
# Mapping where each key is
# associated with a null value
--- #!!set
 #? Mark McGwire
 - Mark McGwire
 #? Sammy Sosa
 - Sammy Sosa
 #? Ken Griffy
 - Ken Griffy
)__",
R"__(["Mark McGwire","Sammy Sosa","Ken Griffy"])__"
},

{
"Example 2.26 Ordered Mappings (NOT SUPPORT)",
R"__(
# Ordered maps are represented as
# A sequence of mappings, with
# each mapping having one key
--- #!!omap
- Mark McGwire: 65
- Sammy Sosa: 63
- Ken Griffy: 58
)__",
R"__([{"Mark McGwire":65},{"Sammy Sosa":63},{"Ken Griffy":58}])__"
},

{
"Example 2.27 Invoice (NOT SUPPORT)",
R"__(
--- #!<tag:clarkevans.com,2002:invoice>
invoice: 34843
date   : 2001-01-23
bill-to: #&id001
    given  : Chris
    family : Dumars
    address:
        lines: |
            458 Walkman Dr.
            Suite #292
        city    : Royal Oak
        state   : MI
        postal  : 48046
#ship-to: *id001
product:
    - sku         : BL394D
      quantity    : 4
      description : Basketball
      price       : 450.00
    - sku         : BL4438H
      quantity    : 1
      description : Super Hoop
      price       : 2392.00
tax  : 251.42
total: 4443.52
comments:
    Late afternoon is best.
    Backup contact is Nancy
    Billsmer @ 338-4338.
)__",
R"__({)__"
  R"__("bill-to":{)__"
    R"__("address":{)__"
      R"__("city":"Royal Oak",)__"
      R"__("lines":"458 Walkman Dr.\nSuite #292\n",)__"
      R"__("postal":48046,)__"
      R"__("state":"MI"},)__"
    R"__("family":"Dumars",)__"
    R"__("given":"Chris"},)__"
  R"__("comments":"Late afternoon is best. Backup contact is Nancy Billsmer @ 338-4338.",)__"
  R"__("date":"2001-01-23",)__"
  R"__("invoice":34843,)__"
  R"__("product":[)__"
    R"__({)__"
      R"__("description":"Basketball",)__"
      R"__("price":450.0,)__"
      R"__("quantity":4,)__"
      R"__("sku":"BL394D"},)__"
    R"__({)__"
      R"__("description":"Super Hoop",)__"
      R"__("price":2392.0,)__"
      R"__("quantity":1,)__"
      R"__("sku":"BL4438H"}],)__"
  R"__("tax":251.42,)__"
  R"__("total":4443.52})__"
},

{
"Example 2.28 Log File (NOT SUPPORT)",
R"__(
#---
-
  Time: 2001-11-23 15:01:42 -5
  User: ed
  Warning:
    This is an error message
    for the log file
#---
-
  Time: 2001-11-23 15:02:31 -5
  User: ed
  Warning:
    A slightly different error
    message.
#---
-
  Date: 2001-11-23 15:03:17 -5
  User: ed
  Fatal:
    Unknown variable "bar"
  Stack:
    - file: TopClass.py
      line: 23
      code: |
        x = MoreObject("345\n")
    - file: MoreClass.py
      line: 58
      code: |-
        foo = bar
)__",
R"__([)__"
  R"__({)__"
    R"__("Time":"2001-11-23 15:01:42 -5",)__"
    R"__("User":"ed",)__"
    R"__("Warning":"This is an error message for the log file"},)__"
  R"__({)__"
    R"__("Time":"2001-11-23 15:02:31 -5",)__"
    R"__("User":"ed",)__"
    R"__("Warning":"A slightly different error message."},)__"
  R"__({)__"
    R"__("Date":"2001-11-23 15:03:17 -5",)__"
    R"__("Fatal":"Unknown variable \"bar\"",)__"
    R"__("Stack":[)__"
      R"__({)__"
        R"__("code":"x = MoreObject(\"345\\n\")\n",)__"
        R"__("file":"TopClass.py",)__"
        R"__("line":23},)__"
      R"__({)__"
        R"__("code":"foo = bar",)__"
        R"__("file":"MoreClass.py",)__"
        R"__("line":58}],)__"
    R"__("User":"ed"}])__"
},

{
"Example 5.3 Block Structure Indicators (NOT SUPPORT)",
R"__(
sequence:
- one
- two
mapping:
  #? sky
  #: blue
  sky : blue
  sea : green
)__",
R"__({"mapping":{"sea":"green","sky":"blue"},"sequence":["one","two"]})__"
},

{
"Example 5.4 Flow Collection Indicators",
R"__(
sequence: [ one, two, ]
mapping: { sky: blue, sea: green }
)__",
R"__({"mapping":{"sea":"green","sky":"blue"},"sequence":["one","two"]})__"
},

{
"Example 5.6 Node Property Indicators (NOT SUPPORT)",
R"__(
#anchored: !local &anchor value
#alias: *anchor
anchored: value
alias: value
)__",
R"__({"alias":"value","anchored":"value"})__"
},

{
"Example 5.7 Block Scalar Indicators",
R"__(
literal: |
  some
  text
folded: >
  some
  text
)__",
R"__({"folded":"some text\n","literal":"some\ntext\n"})__"
},

{
"Example 5.8 Quoted Scalar Indicators",
R"__(
single: 'text'
double: "text"
)__",
R"__({"double":"text","single":"text"})__"
},

{
"Example 5.9 Directive Indicator",
R"__(
--- text
)__",
R"__("text")__"
},

{
"Example 5.12 Tabs and Spaces",
R"__(
# Tabs and spaces
quoted: "Quoted 	"
block:	|
  void main() {
  	printf("Hello, world!\n");
  }
)__",
R"__({)__"
  R"__("block":"void main() {\n\tprintf(\"Hello, world!\\n\");\n}\n",)__"
  R"__("quoted":"Quoted \t"})__"
},

{
"Example 5.13 Escaped Characters",
R"__(
"Fun with \\
\" \a \b \e \f \
\n \r \t \v \0 \
\  \N \_ \L \P \
\x41 \u0041 \U00000041"
)__",
R"__("Fun with \\ )__"
R"__(\" \u0007 \b \u001b \f )__"
R"__(\n \r \t \u000b \u0000 )__"
  u8"  \u0085 \u00a0 \u2028 \u2029 "
R"__(A A A")__"
},

{
"Example 6.1 Indentation Spaces",
R"__(
  # Leading comment line spaces are
   # neither content nor indentation.
    
Not indented:
 By one space: |
    By four
      spaces
 Flow style: [    # Leading spaces
   By two,        # in flow style
  Also by two,    # are neither
  	Still by two   # content nor
    ]             # indentation.
)__",
R"__({)__"
  R"__("Not indented":{)__"
    R"__("By one space":"By four\n  spaces\n",)__"
    R"__("Flow style":[)__"
      R"__("By two","Also by two",)__"
      R"__("Still by two"]}})__"
},

{
"Example 6.2 Indentation Indicators (NOT SUPPORT)",
R"__(
#? a
#: -	b
a:
   - b
   -  -	c
      - d
)__",
R"__({"a":["b",["c","d"]]})__"
},

{
"Example 6.3 Separation Spaces",
R"__(
- foo:	 bar
- - baz
  -	baz
)__",
R"__([{"foo":"bar"},["baz","baz"]])__"
},

{
"Example 6.4 Line Prefixes",
R"__(
plain: text
  lines
quoted: "text
  	lines"
block: |
  text
   	lines
)__",
R"__({"block":"text\n \tlines\n","plain":"text lines","quoted":"text lines"})__"
},

{
"Example 6.5 Empty Lines",
R"__(
Folding:
  "Empty line
   	
  as a line feed"
Chomping: |
  Clipped empty lines
 
)__",
R"__({"Chomping":"Clipped empty lines\n","Folding":"Empty line\nas a line feed"})__"
},

{
"Example 6.6 Line Folding",
R"__(
>-
  trimmed
  
 

  as
  space
)__",
R"__("trimmed\n\n\nas space")__"
},

{
"Example 6.7 Block Folding",
R"__(
>
  foo 
 
  	 bar

  baz
)__",
R"__("foo \n\n\t bar\n\nbaz\n")__"
},

{
"Example 6.8 Flow Folding",
R"__(
"
  foo 
 
  	 bar

  baz
"
)__",
R"__(" foo\nbar\nbaz ")__"
},

{
"Example 6.9 Separated Comment",
R"__(
key:    # Comment
  value
)__",
R"__({"key":"value"})__"
},

{
"Example 6.11 Multi-Line Comments",
R"__(
key:    # Comment
        # lines
  value

)__",
R"__({"key":"value"})__"
},

{
"Example 6.12 Separation Spaces (NOT SUPPORT)",
R"__(
#{ first: Sammy, last: Sosa }:
'{ first: Sammy, last: Sosa }':
# Statistics:
  hr:  # Home runs
     65
  avg: # Average
   0.278
)__",
R"__({"{ first: Sammy, last: Sosa }":{"avg":0.278,"hr":65}})__"
},

{
"Example 6.13 Reserved Directives (NOT SUPPORT)",
R"__(
#%FOO  bar baz # Should be ignored
               # with a warning.
--- "foo"
)__",
R"__("foo")__"
},

{
"Example 6.14 YAML directive (IGNORE)",
R"__(
%YAML 1.3 # Attempt parsing
          # with a warning
---
"foo"
)__",
R"__("foo")__"
},

{
"Example 6.16 TAG directive (IGNORE)",
R"__(
%TAG !yaml! tag:yaml.org,2002:
---
#!yaml!str "foo"
"foo"
)__",
R"__("foo")__"
},

{
"Example 6.18 Primary Tag Handle (NOT SUPPORT)",
R"__(
# Private
#!foo "bar"
- "bar"
#...
# Global
#%TAG ! tag:example.com,2000:app/
#---
#!foo "bar"
- "bar"
)__",
R"__(["bar","bar"])__"
},

{
"Example 6.19 Secondary Tag Handle (NOT SUPPORT)",
R"__(
%TAG !! tag:example.com,2000:app/
---
#!!int 1 - 3 # Interval, not integer
1 - 3
...
)__",
R"__("1 - 3")__"
},

{
"Example 6.20 Tag Handles (NOT SUPPORT)",
R"__(
%TAG !e! tag:example.com,2000:app/
---
#!e!foo "bar"
"bar"
---
)__",
R"__("bar")__"
},

{
"Example 6.21 Local Tag Prefix (NOT SUPPORT)",
R"__(
#%TAG !m! !my-
#--- # Bulb here
#!m!light fluorescent
#...
#%TAG !m! !my-
#--- # Color here
#!m!light green
#...
- fluorescent
- green
)__",
R"__(["fluorescent","green"])__"
},

{
"Example 6.22 Global Tag Prefix (NOT SUPPORT)",
R"__(
%TAG !e! tag:example.com,2000:app/
---
#- !e!foo "bar"
- "bar"
)__",
R"__(["bar"])__"
},

{
"Example 6.23 Node Properties (NOT SUPPORT)",
R"__(
#!!str &a1 "foo":
#  !!str bar
#&a2 baz : *a1
"foo":
  bar
baz : "foo"
)__",
R"__({"baz":"foo","foo":"bar"})__"
},

{
"Example 6.24 Verbatim Tags (NOT SUPPORT)",
R"__(
#!<tag:yaml.org,2002:str> foo :
#  !<!bar> baz
foo :
  baz
)__",
R"__({"foo":"baz"})__"
},

{
"Example 6.26 Tag Shorthands (NOT SUPPORT)",
R"__(
%TAG !e! tag:example.com,2000:app/
---
#- !local foo
#- !!str bar
#- !e!tag%21 baz
- foo
- bar
- baz
)__",
R"__(["foo","bar","baz"])__"
},

{
"Example 6.28 Non-Specific Tags (NOT SUPPORT)",
R"__(
# Assuming conventional resolution:
- "12"
- 12
#- ! 12
- 12
)__",
R"__(["12",12,12])__"
},

{
"Example 6.29 Node Anchors (NOT SUPPORT)",
R"__(
#First occurrence: &anchor Value
#Second occurrence: *anchor
First occurrence: Value
Second occurrence: Value
)__",
R"__({"First occurrence":"Value","Second occurrence":"Value"})__"
},

{
"Example 7.1 Alias Nodes (NOT SUPPORT)",
R"__(
#First occurrence: &anchor Foo
#Second occurrence: *anchor
#Override anchor: &anchor Bar
#Reuse anchor: *anchor
First occurrence: Foo
Second occurrence: Foo
Override anchor: Bar
Reuse anchor: Bar
)__",
R"__({)__"
  R"__("First occurrence":"Foo",)__"
  R"__("Override anchor":"Bar",)__"
  R"__("Reuse anchor":"Bar",)__"
  R"__("Second occurrence":"Foo"})__"
},

{
"Example 7.2 Empty Content (NOT SUPPORT)",
R"__(
{
#  foo : !!str,
  foo : ,
#  !!str : bar,
  "" : bar,
}
)__",
R"__({"":"bar","foo":null})__"
},

{
"Example 7.3 Completely Empty Flow Nodes (NOT SUPPORT)",
R"__(
{
  #? foo :,
  foo :,
  #: bar,
  "": bar,
}
)__",
R"__({"":"bar","foo":null})__"
},

{
"Example 7.4 Double Quoted Implicit Keys (NOT SUPPORTED)",
R"__(
"implicit block key" : [
  #"implicit flow key" : value,
  {"implicit flow key" : value},
 ]
)__",
R"__({"implicit block key":[{"implicit flow key":"value"}]})__"
},

{
"Example 7.5 Double Quoted Line Breaks",
R"__(
"folded 
to a space,	
 
to a line feed, or 	\
 \ 	non-content"
)__",
R"__("folded to a space,\nto a line feed, or \t \tnon-content")__"
},

{
"Example 7.6 Double Quoted Lines",
R"__(
" 1st non-empty

 2nd non-empty 
	3rd non-empty "
)__",
R"__(" 1st non-empty\n2nd non-empty 3rd non-empty ")__"
},

{
"Example 7.7 Single Quoted Characters",
R"__(
 'here''s to "quotes"'
)__",
R"__("here's to \"quotes\"")__"
},

{
"Example 7.8 Single Quoted Implicit Keys (NOT SUPPORTED)",
R"__(
'implicit block key' : [
  #'implicit flow key' : value,
  {'implicit flow key' : value},
 ]
)__",
R"__({"implicit block key":[{"implicit flow key":"value"}]})__"
},

{
"Example 7.9 Single Quoted Lines",
R"__(
' 1st non-empty

 2nd non-empty 
	3rd non-empty '
)__",
R"__(" 1st non-empty\n2nd non-empty 3rd non-empty ")__"
},

{
"Example 7.10 Plain Characters",
R"__(
# Outside flow collection:
- ::vector
- ": - ()"
- Up, up, and away!
- -123
- http://example.com/foo#bar
# Inside flow collection:
- [ ::vector,
  ": - ()",
  "Up, up and away!",
  -123,
  http://example.com/foo#bar ]
)__",
R"__([)__"
  R"__("::vector",)__"
  R"__(": - ()",)__"
  R"__("Up, up, and away!",)__"
  R"__(-123,)__"
  R"__("http:\/\/example.com\/foo#bar",)__"
  R"__([)__"
    R"__("::vector",": - ()",)__"
    R"__("Up, up and away!",)__"
    R"__(-123,)__"
    R"__("http:\/\/example.com\/foo#bar"]])__"
},

{
"Example 7.11 Plain Implicit Keys (NOT SUPPORTED)",
R"__(
implicit block key : [
  #implicit flow key : value,
  {implicit flow key : value},
 ]
)__",
R"__({"implicit block key":[{"implicit flow key":"value"}]})__"
},

{
"Example 7.12 Plain Lines",
R"__(
1st non-empty

 2nd non-empty 
	3rd non-empty
)__",
R"__("1st non-empty\n2nd non-empty 3rd non-empty")__"
},

{
"Example 7.13 Flow Sequence",
R"__(
- [ one, two, ]
- [three ,four]
)__",
R"__([["one","two"],["three","four"]])__"
},

{
"Example 7.14 Flow Sequence Entries (NOT SUPPORTED)",
R"__(
[
"double
 quoted", 'single
           quoted',
plain
 text, [ nested ],
#single: pair,
{single: pair},
]
)__",
R"__(["double quoted","single quoted","plain text",["nested"],{"single":"pair"}])__"
},

{
"Example 7.15 Flow Mappings",
R"__(
- { one : two , three: four , }
- {five: six,seven : eight}
)__",
R"__([{"one":"two","three":"four"},{"five":"six","seven":"eight"}])__"
},

{
"Example 7.16 Flow Mapping Entries (NOT SUPPORTED)",
R"__(
{
#? explicit: entry,
explicit: entry,
implicit: entry,
#?
"":
}
)__",
R"__({"":null,"explicit":"entry","implicit":"entry"})__"
},

{
"Example 7.17 Flow Mapping Separate Values (NOT SUPPORTED)",
R"__(
{
unquoted : "separate",
http://foo.com,
omitted value:,
#: omitted key,
"": omitted key,
}
)__",
R"__({"":"omitted key","http:\/\/foo.com":null,"omitted value":null,"unquoted":"separate"})__"
},

{
"Example 7.18 Flow Mapping Adjacent Values",
R"__(
{
"adjacent":value,
"readable": value,
"empty":
}
)__",
R"__({"adjacent":"value","empty":null,"readable":"value"})__"
},

{
"Example 7.19 Single Pair Flow Mappings (NOT SUPPORTED)",
R"__(
[
#foo: bar
{foo: bar}
]
)__",
R"__([{"foo":"bar"}])__"
},

{
"Example 7.20 Single Pair Explicit Entry (NOT SUPPORTED)",
R"__(
[
#? foo
# bar : baz
{foo bar : baz}
]
)__",
R"__([{"foo bar":"baz"}])__"
},

{
"Example 7.21 Single Pair Implicit Entries (NOT SUPPORTED)",
R"__(
#- [ YAML : separate ]
#- [ : empty key entry ]
#- [ {JSON: like}:adjacent ]
- [ {YAML : separate} ]
- [ {"": empty key entry} ]
- [ {"{JSON: like}":adjacent} ]
)__",
R"__([)__"
  R"__([{"YAML":"separate"}],)__"
  R"__([{"":"empty key entry"}],)__"
  R"__([{"{JSON: like}":"adjacent"}]])__"
},

{
"Example 7.23 Flow Content",
R"__(
- [ a, b ]
- { a: b }
- "a"
- 'b'
- c
)__",
R"__([["a","b"],{"a":"b"},"a","b","c"])__"
},

{
"Example 7.24 Flow Nodes (NOT SUPPORTED)",
R"__(
#- !!str "a"
#- 'b'
#- &anchor "c"
#- *anchor
#- !!str
- "a"
- 'b'
- "c"
- "c"
- ""
)__",
R"__(["a","b","c","c",""])__"
},

{
"Example 8.1 Block Scalar Header",
R"__(
- | # Empty header
 literal
- >1 # Indentation indicator
  folded
- |+ # Chomping indicator
 keep

- >1- # Both indicators
  strip


)__",
R"__(["literal\n"," folded\n","keep\n\n"," strip"])__"
},

{
"Example 8.2 Block Indentation Indicator",
R"__(
- |
 detected
- >
 
  
  # detected
- |1
  explicit
- >
 	
 detected

...
)__",
R"__(["detected\n","\n\n# detected\n"," explicit\n","\t\ndetected\n"])__"
},

{
"Example 8.4 Chomping Final Line Break",
R"__(
strip: |-
  text
clip: |
  text
keep: |+
  text
)__",
R"__({"clip":"text\n","keep":"text\n","strip":"text"})__"
},

{
"Example 8.5 Chomping Trailing Lines",
R"__(
 # Strip
  # Comments:
strip: |-
  # text
  
 # Clip
  # comments:

clip: |
  # text
 
 # Keep
  # comments:

keep: |+
  # text

 # Trail
  # comments.

...
)__",
R"__({"clip":"# text\n","keep":"# text\n\n","strip":"# text"})__"
},

{
"Example 8.6. Empty Scalar Chomping",
R"__(
strip: >-


clip: >


keep: |+


...
)__",
R"__({"clip":"","keep":"\n\n","strip":""})__"
},

{
"Example 8.7. Literal Scalar",
R"__(
|
 literal
 	text

)__",
R"__("literal\n\ttext\n")__"
},

{
"Example 8.8. Literal Content",
R"__(
|
 
  
  literal
   
  
  text

 # Comment
)__",
R"__("\n\nliteral\n \n\ntext\n")__"
},

{
"Example 8.9. Folded Scalar",
R"__(
>
 folded
 text


)__",
R"__("folded text\n")__"
},

{
"Example 8.10. Folded Lines",
R"__(
>

 folded
 line

 next
 line
   * bullet

   * list
   * lines

 last
 line

# Comment
)__",
R"__("\nfolded line\nnext line\n  * bullet\n\n  * list\n  * lines\n\nlast line\n")__"
},

{
"Example 8.14. Block Sequence",
R"__(
block sequence:
  - one
  - two : three
)__",
R"__({"block sequence":["one",{"two":"three"}]})__"
},

{
"Example 8.15. Block Sequence Entry Types",
R"__(
- # Empty
- |
 block node
- - one # Compact
  - two # sequence
- one: two # Compact mapping
)__",
R"__([null,"block node\n",["one","two"],{"one":"two"}])__"
},

{
"Example 8.16. Block Mappings",
R"__(
block mapping:
 key: value
)__",
R"__({"block mapping":{"key":"value"}})__"
},

{
"Example 8.17. Explicit Block Mapping Entries (NOT SUPPORT)",
R"__(
#? explicit key # Empty value
#? |
#  block key
#: - one # Explicit compact
#  - two # block value
explicit key : # Empty value
"block key\n":
  - one # Explicit compact
  - two # block value
)__",
R"__({"block key\n":["one","two"],"explicit key":null})__"
},

{
"Example 8.18. Implicit Block Mapping Entries (NOT SUPPORT)",
R"__(
plain key: in-line value
#: # Both empty
"": # Both empty
"quoted key":
- entry
)__",
R"__({"":null,"plain key":"in-line value","quoted key":["entry"]})__"
},

{
"Example 8.19. Compact Block Mappings (NOT SUPPORT)",
R"__(
- sun: yellow
#- ? earth: blue
#  : moon: white
- 'earth: blue' :
    moon: white
)__",
R"__([{"sun":"yellow"},{"earth: blue":{"moon":"white"}}])__"
},

{
"Example 8.20. Block Node Types (NOT SUPPORTED)",
R"__(
-
  "flow in block"
- >
 Block scalar
- # !!map # Block collection
  foo : bar
)__",
R"__(["flow in block","Block scalar\n",{"foo":"bar"}])__"
},

{
"Example 8.21. Block Scalar Nodes (NOT SUPPORTED)",
R"__(
literal: |2
  value
folded:
#   !foo
  >1
 value

)__",
R"__({"folded":"value\n","literal":"value\n"})__"
},

{
"Example 8.22. Block Collection Nodes (NOT SUPPORTED)",
R"__(
sequence: #!!seq
- entry
- #!!seq
 - nested
mapping: #!!map
 foo: bar
)__",
R"__({"mapping":{"foo":"bar"},"sequence":["entry",["nested"]]})__"
},

{
"Example 9.1. Document Prefix",
R"__(
# Comment
# lines
Document
)__",
R"__("Document")__"
},

{
"Example 9.2. Document Markers",
R"__(
%YAML 1.2
---
Document
... # Suffix
)__",
R"__("Document")__"
},

{
"Example 9.3. Bare Documents",
R"__(
Bare
document
...
# No document
)__",
R"__("Bare document")__"
},

{
"Example 9.3. Bare Documents",
R"__(
|
%!PS-Adobe-2.0 # Not the first line
...
)__",
R"__("%!PS-Adobe-2.0 # Not the first line\n")__"
},

{
"Example 9.4. Explicit Documents",
R"__(
---
{ matches
% : 20 }
...
)__",
R"__({"matches %":20})__"
},

{
"Example 9.4. Explicit Documents empty",
R"__(
...
---
# Empty
...
)__",
R"__(null)__"
},

{
"Example 9.5. Directives Documents",
R"__(
%YAML 1.2
--- |
%!PS-Adobe-2.0
...
)__",
R"__("%!PS-Adobe-2.0\n")__"
},

{
"Example 9.5. Directives Documents empty",
R"__(
%YAML1.2
---
# Empty
...
)__",
R"__(null)__"
},

};// spec[]

#include <sstream>
#include <iomanip>

int main ()
{
    int tests = sizeof (spec) / sizeof (spec[0]);
    test::simple ts (tests);
    for (int i = 0; i < tests; i++) {
        std::string input (spec[i].input);
        wjson::value_type value;
        bool res = wjson::decode_yaml (input, value);
        std::string got = wjson::encode_json (value);
        ts.ok (res && got == spec[i].expected, spec[i].name);
        if (got != spec[i].expected) {
            ts.diag (got);
        }
    }
    return ts.done_testing ();
}


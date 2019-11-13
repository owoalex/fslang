# fslang
An esoteric programming language where everything is a file.

## Everything is a file!
An example of declaring an integer:

`/foo.int = 3;`

## Variables are global by default
Variables die with their function if defined locally though!

`/foo.func = {
./bar.float = 0.03;
-- e.g. /proc/self == 3
};`


`-- /proc/3/bar.float == undefined`

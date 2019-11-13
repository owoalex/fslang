--test1
/ignoreMe.int;
--test2
/hw.string = "Hello World";

/testFunction.void = function () {
    /print(/hw.string); --this is a comment
}

/testFunction.void();

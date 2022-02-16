# Json Parser in Standard C (C99)
This json parser is created for the project of [C++ Workflow](https://github.com/sogou/workflow).  
# Build tests
$ make
# Run tests
### Parse and print json document:
$ ./parse_json \< xxx.json
### Test parsing speed:
$ time ./test_speed \<repeat times\> \< xxx.json
# BUG
Do not support unicode (\\u hex hex hex hex).

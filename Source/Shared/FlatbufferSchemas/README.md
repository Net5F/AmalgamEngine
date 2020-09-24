To compile, first build the project to get a flatc binary to use.

Then run:
../../../Build/<platform>/Debug/Libraries/flatbuffers/flatc.exe -c -o ../Public Message.fbs --scoped-enums --cpp-std c++17

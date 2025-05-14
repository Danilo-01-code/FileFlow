# FileFlow
// TODO
an bash 
```
mkdir build
cd build
cmake ..
make
./FileFlow
```

system(CLEAR);

If you wanna to use FileFlow as a system command (make the program executable for any directory) you can use the follow command:

```
sudo cp FileFlow /usr/local/bin/
```

on Windows you should run this command as admin, to use FileFlow as a system command: 

```
Copy-Item -Path .\FileFlow.exe -Destination "C:\Windows\System32\"
```

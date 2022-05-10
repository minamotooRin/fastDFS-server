# FileCacheProxy

A reverse proxy for FastDFS, which provides http based interface for utilizing fastDFS.

## Compile 

Based on FastDFS & libevent. 
You need install libevent and libfastcommon for compiling.
Before running FileCacheProxy, you should deploy your fastDFS properly, and modify `FileCacheProxy.conf` correspondingly.

## Usage

Just execute the compile result `main`, or using `nohup`:
```
nohup ./build/main 2>&1 &
```

## TO DO

- when fdfs is logging, buffer overflow will be detected.
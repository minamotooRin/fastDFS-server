# FileCacheProxy

A reverse proxy for FastDFS, which provides http based interface for utilizing fastDFS.

## Compile 

Based on FastDFS & libevent. 
You need to install libevent and libfastcommon for compiling.
Before running FileCacheProxy, you should deploy your fastDFS properly, and modify `FileCacheProxy.conf` correspondingly.

## Usage

After modifying configuration in `fileCacheProxy.conf`, just execute the compiling result `main`, or using `nohup`:
```
nohup ./build/main 2>&1 &
```

Scripts in `scripts/` include monitor, killer and expired file cleaner.

## TO DO

- Recording receiving time. TB test
- batch deleting file. TB test
- when fdfs is logging, buffer overflow will be detected.
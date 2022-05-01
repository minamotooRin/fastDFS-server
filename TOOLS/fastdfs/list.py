import re
import sys
import os
 
def allFiles():
    path = '/data/fastdfs/storage/data' # basepath≈‰÷√¬∑æ∂
    rounds = 1
    fdfspath = 'group1/M00'
 
    with open('/home/timing/shelles/data.txt','w') as file_url:
         
        for dirpath, dirnames, filenames in os.walk(path):
            if rounds == 1:
                rounds+=1
            elif (dirpath == path + '/sync'):
                continue
            else:
                for file in filenames:
                    try:
                        paths = re.search(r'/data/fastdfs/storage/data(.*)',dirpath).group(1)
                        fullpath = os.path.join(fdfspath + paths, file)
                        print(fullpath)
                        file_url.write(fullpath + '\n')
                    except:
                        pass
 
                rounds+=1
        file_url.close()
  
def toRedis():
    with open('/home/redis/tuna/shelles/data.txt', 'r') as logfile:
        for line in logfile:
            print(line)
            redis_client.sadd('dfs_picture',line.replace('\n', ''))
        logfile.close()
 
if __name__ == '__main__':
    if(sys.argv[1] == 'allfiles'):
        allFiles()<br>    elif(sys.argv[1] == 'toredis'):<br>        toRedis()
    else:
        print("USAGE:allfiles|toredis")
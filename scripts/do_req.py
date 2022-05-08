import requests
import sys

ip = '172.17.0.4'
port = 9999
methods = ['upload', 'delete', 'fileinfo']

if __name__ == '__main__':
    
    if len(sys.argv) <= 1 :
        print("Usage: python3 do_req.py upload/delete/fileinfo")
        exit(0)

    method = sys.argv[1]

    if not method in methods :
        print("Usage: python3 do_req.py upload/delete/fileinfo")
        exit(0)

    register_url = "http://{}:{}/{}".format(ip, port, method)
    print(register_url)
    
    FileID = str()
    if len(sys.argv) >= 3 :
        FileID = sys.argv[2]

    header = {
        "FileID" : FileID,    
        "FileExt" : "abc"
    }

    # json = {
    #     "mobile_phone": "15612345678",
    #     "pwd": "Test1234",
    #     "type": 0
    # }
    # response = requests.post(url=register_url, json=json, headers=header)

    # upload_files = {'file': open('fpmoni.sh', 'rb')}
    # response = requests.post(url=register_url, files=upload_files, headers=header)

    response = requests.post(url=register_url, headers=header, data='this is a string')

    print(response.status_code, response.reason)
    print(response.headers)
    print(response.text)

import requests

register_url = "http://172.17.0.4:9999/fileinfo"

header = {
    "Content-Type": "application/json",
    "FileID" : "group1/M00/00/00/fwAAAWJuQzWAc53lAABEiaWfr5k08874.h"
}

# json类型的参数
json = {
    "mobile_phone": "15612345678",
    "pwd": "Test1234",
    "type": 0
}
# 发送post请求
response = requests.post(url=register_url, json=json, headers=header)
print(response.status_code)
print(response.content)

# 打印结果：{'code': 200, 'msg': 'success', 'password': '321', 'username': '123'}
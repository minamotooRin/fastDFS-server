#include<iostream>
#include<string>
#include "../include/json.hpp"

using namespace std;

int main()
{
    // nlohmann::json j = nlohmann::json::object();
    nlohmann::json j;
    j["a"] = string("123");

    cout << j.dump() << endl;

    return 0;
}
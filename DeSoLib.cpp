#include "DeSoLib.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include "ArduinoJson.h"

#define DEBUG_LOG false

DeSoLib::DeSoLib()
{
}
void DeSoLib::addNodePath(const char *url, const char *cert)
{
    Node n;
    strncpy(n.url, url, sizeof(n.url));
    n.caRootCert = cert;
    nodePaths.push_back(n);
}
int DeSoLib::getMaxNodes()
{
    return nodePaths.size();
}

void DeSoLib::selectDefaultNode(int index)
{
    selectedNodeIndex = index;
}
char *DeSoLib::getSelectedNodeUrl()
{
    return nodePaths[selectedNodeIndex].url;
}
bool DeSoLib::getSelectedNodeStatus()
{
    return nodePaths[selectedNodeIndex].status;
}

char *DeSoLib::getRequest(const char *apiPath)
{
    HTTPClient https;
    char url_str[100];
    memset(buff_large, 0, sizeof(buff_large));
    espClientSecure.setCACert(nodePaths[selectedNodeIndex].caRootCert);
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    snprintf(url_str, sizeof(url_str), "%s%s", nodePaths[selectedNodeIndex].url, apiPath);

    if (https.begin(espClientSecure, url_str))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            if (httpCode == HTTP_CODE_OK)
            {
                strncpy(buff_large, https.getString().c_str(), sizeof(buff_large));
            }
        }
    }
    https.end();
    return buff_large;
}
const char *DeSoLib::postRequest(const char *apiPath, const char *data)
{
    HTTPClient https;
    static char buff_null[] = "{}";
    const char *buff_ptr;
    buff_ptr = buff_null;
    memset(buff_large, 0, sizeof(buff_large));
    espClientSecure.setCACert(nodePaths[selectedNodeIndex].caRootCert);
    snprintf(buff_small_1, sizeof(buff_small_1), "%s%s", nodePaths[selectedNodeIndex].url, apiPath);

    if (https.begin(espClientSecure, buff_small_1))
    {
        https.addHeader("User-Agent", "Mozilla/5.0");
        https.addHeader("Accept", "application/json");
        https.addHeader("Content-Type", "application/json");
        int httpCode = https.POST((uint8_t *)data, strlen(data));
        if (httpCode > 0)
        {
            if (httpCode == HTTP_CODE_OK)
            {

                //if (https.getSize() < 80000)
                //{
                    buff_ptr = https.getString().c_str();
                    if (strcmp(buff_ptr, "") == 0){
                        buff_ptr = buff_null;
                    }
                //}
            }
            else
            {
            }
        }
        else
        {
        }
    }
    else
    {
    }
    https.end();

    //return buff_large;
    return buff_ptr;
}

char *DeSoLib::getNodeHealthCheck()
{
    return getRequest(RoutePathHealthCheck);
}

void DeSoLib::updateNodeHealthCheck()
{
    if (strcmp(getNodeHealthCheck(), "200") == 0)
    {
        nodePaths[selectedNodeIndex].status = true;
    }
    else
    {
        nodePaths[selectedNodeIndex].status = false;
    }
}

char *DeSoLib::getExchangeRates()
{
    return getRequest(ExchangeRateRoute);
}

void DeSoLib::updateExchangeRates()
{
    DynamicJsonDocument doc(1024);
    const char *payload = getExchangeRates();
    DeserializationError error = deserializeJson(doc, payload);
    if (!error)
    {
        USDCentsPerBitCloutExchangeRate = doc["USDCentsPerBitCloutExchangeRate"];
        USDCentsPerBitcoinExchangeRate = doc["USDCentsPerBitcoinExchangeRate"];
    }
}
const char *DeSoLib::getSingleProfile(const char *messagePayload)
{
    return postRequest(RoutePathGetSingleProfile, messagePayload);
}

void DeSoLib::updateSingleProfile(const char *username, const char *PublicKeyBase58Check, Profile *prof)
{
    static char postData[100];
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() / 2 - 5000);
    if (strlen(username) > 0)
    {
        doc["Username"] = username;
    }
    if (strlen(PublicKeyBase58Check) > 0)
    {
        doc["PublicKeyBase58Check"] = PublicKeyBase58Check;
    }

    serializeJson(doc, postData);
    doc.clear();
    const char *payload = getSingleProfile(postData);

    DeserializationError error = deserializeJson(doc, payload);
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {
        strncpy(prof->Username, doc["Profile"]["Username"], sizeof(prof->Username));
        prof->CoinPriceBitCloutNanos = doc["Profile"]["CoinPriceBitCloutNanos"];
        prof->CoinsInCirculationNanos = doc["Profile"]["CoinEntry"]["CoinsInCirculationNanos"];
        strcpy(prof->PublicKeyBase58Check, doc["Profile"]["PublicKeyBase58Check"]);
    }
    else
    {
        debug_print("Json Error");
    }
}

const char *DeSoLib::getUsersStateless(const char *messagePayload)
{
    return postRequest(RoutePathGetUsersStateless, messagePayload);
}

void DeSoLib::updateUsersStateless(const char *PublicKeysBase58Check, bool skipHodlings, Profile *prof)
{
    static char messagePayload[200];
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() - 5000);
    doc["PublicKeysBase58Check"][0] = PublicKeysBase58Check;
    doc["skipHodlings"] = skipHodlings;
    serializeJson(doc, messagePayload);
    doc.clear();
    const char *payload = getUsersStateless(messagePayload);
    //StaticJsonDocument<200> filter;
    DynamicJsonDocument filter(400);
    filter["UserList"][0]["BalanceNanos"] = true;
    if (skipHodlings == false)
    {   
        for(int i=0;i<5;i++){
            filter["UserList"][0]["UsersYouHODL"][i]["BalanceNanos"] = true;
            filter["UserList"][0]["UsersYouHODL"][i]["ProfileEntryResponse"]["CoinPriceBitCloutNanos"]=true;
        }
    }

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    prof->BalanceNanos = doc["UserList"][0]["BalanceNanos"];
    double HODLBalance = 0;
    double perUserHODLBalanceNanos = 0;
    double perUserHODLValue = 0;
    int TotalHodleNum = 0;
    if (skipHodlings == false)
    {
        JsonArray arr = doc["UserList"][0]["UsersYouHODL"].as<JsonArray>();
        for (JsonVariant value : arr)
        {
            perUserHODLBalanceNanos = value["BalanceNanos"].as<double>();
            perUserHODLValue = value["ProfileEntryResponse"]["CoinPriceBitCloutNanos"].as<double>();
            HODLBalance += ((perUserHODLBalanceNanos / 1000000000.0) * perUserHODLValue) / 1000000000.0;
            TotalHodleNum++;
        }
        //Serial.println(HODLBalance);
        //Serial.println(ESP.getMaxAllocHeap());
        prof->TotalHODLBalanceClout = HODLBalance;
        prof->TotalHodleNum = TotalHodleNum;
    }
    if (!error)
    {
    }
    else
    {
        debug_print("\nJson Error,incomplete due to low memory\n");
    }
    // Print the result
    //serializeJsonPretty(doc, Serial);
}

const char *DeSoLib::getHodlersForPublicKey(const char *messagePayload)
{
    return postRequest(RoutePathGetHodlersForPublicKey, messagePayload);
}
void DeSoLib::updateHodlersForPublicKey(const char *username, const char *PublicKeyBase58Check, int NumToFetch, Profile *prof)
{
    static char postData[100];
    DynamicJsonDocument doc(ESP.getMaxAllocHeap() - 5000);
    if (strlen(username) > 0)
    {
        doc["Username"] = username;
        strncpy(prof->Username, username, sizeof(prof->Username));
    }
    if (strlen(PublicKeyBase58Check) > 0)
    {
        doc["PublicKeyBase58Check"] = PublicKeyBase58Check;
    }
    if (NumToFetch > sizeof(prof->TopHodlersUserNames[0]))
    {
        NumToFetch = sizeof(prof->TopHodlersUserNames[0]);
    }
    doc["NumToFetch"] = NumToFetch + 1;

    serializeJson(doc, postData);
    doc.clear();
    const char *payload = getHodlersForPublicKey(postData);
    DynamicJsonDocument filter(500);
    //StaticJsonDocument<200> filter;
    //filter["Hodlers"][0]["BalanceNanos"] = true;
    for (int i = 0; i < NumToFetch + 1; i++)
    {
        filter["Hodlers"][i]["ProfileEntryResponse"]["Username"] = true;
    }

    // Deserialize the document
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    if (doc.isNull())
    {
        serializeJsonPretty(doc, Serial);
    }
    if (!error)
    {
        JsonArray arr = doc["Hodlers"].as<JsonArray>();
        int count = 0;
        for (JsonVariant value : arr)
        {
            if (strcmp(value["ProfileEntryResponse"]["Username"].as<char *>(), prof->Username) != 0)
            {
                strncpy(prof->TopHodlersUserNames[count], value["ProfileEntryResponse"]["Username"].as<char *>(), sizeof(prof->TopHodlersUserNames[count]));
                count++;
                if (count >= 10)
                    break;
            }
        }
    }
    else
    {
        debug_print("Json Error");
    }
}

void DeSoLib::clearTopHodlersUserNames(Profile *prof)
{
    for (int i = 0; i < sizeof(prof->TopHodlersUserNames[0]); i++)
    {
        strcpy(prof->TopHodlersUserNames[i], "");
    }
}

DeSoLib::~DeSoLib()
{
}
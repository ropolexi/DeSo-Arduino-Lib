#include "DeSoLib.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include "ArduinoJson.h"

#define DEBUG_LOG true

DeSoLib::DeSoLib()
{
}
void DeSoLib::addNodePath(const char* url,const char* cert)
{
    Node n;
    strncpy(n.url, url, sizeof(n.url));
    n.caRootCert = cert;
    nodePaths.push_back(n);
}
int DeSoLib::getMaxNodes(){
    return nodePaths.size();
}

void DeSoLib::selectDefaultNode(int index)
{
    selectedNodeIndex = index;
}
char* DeSoLib::getSelectedNodeUrl()
{
    return nodePaths[selectedNodeIndex].url;
}
bool DeSoLib::getSelectedNodeStatus()
{
    return nodePaths[selectedNodeIndex].status;
}

char* DeSoLib::getRequest(const char* apiPath){
    HTTPClient https;
    char url_str[100];
    memset(buff_large,0,sizeof(buff_large));
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
char * DeSoLib::postRequest(const char* apiPath,const char* data){
    HTTPClient https;
    memset(buff_large,0,sizeof(buff_large));
    espClientSecure.setCACert(nodePaths[selectedNodeIndex].caRootCert);
    snprintf(buff_small_1, sizeof(buff_small_1), "%s%s", nodePaths[selectedNodeIndex].url, apiPath);

    if (https.begin(espClientSecure, buff_small_1)) 
    { 
        https.addHeader("User-Agent","Mozilla/5.0");
        https.addHeader("Accept", "application/json");
        https.addHeader("Content-Type", "application/json");
        int httpCode = https.POST((uint8_t*)data,strlen(data));
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

char * DeSoLib::getNodeHealthCheck()
{
    return getRequest(RoutePathHealthCheck); 
}

void DeSoLib::updateNodeHealthCheck(){
    if(strcmp(getNodeHealthCheck(),"200")==0){
        nodePaths[selectedNodeIndex].status=true;
    }else{
        nodePaths[selectedNodeIndex].status=false;
    }
}

char * DeSoLib::getExchangeRates(){
    return getRequest(ExchangeRateRoute);
}

void DeSoLib::updateExchangeRates(){
    DynamicJsonDocument doc(1024);
    const char *payload = getExchangeRates();
    DeserializationError error = deserializeJson(doc, payload);
    if (!error)
    {
        USDCentsPerBitCloutExchangeRate = doc["USDCentsPerBitCloutExchangeRate"];
        USDCentsPerBitcoinExchangeRate = doc["USDCentsPerBitcoinExchangeRate"];

    }
}
char * DeSoLib::getSingleProfile(const char *messagePayload){
    return postRequest(RoutePathGetSingleProfile,messagePayload);

}

void DeSoLib::updateSingleProfile(const char *username,const char *PublicKeyBase58Check,Profile *prof){
    static char postData[100];
    DynamicJsonDocument doc(1024);
    if(strlen(username)>0){
        doc["Username"]=username;
    }
    if(strlen(PublicKeyBase58Check)>0){
        doc["PublicKeyBase58Check"]=PublicKeyBase58Check;
    }
        
    serializeJson(doc, postData);

    const char *payload = getSingleProfile(postData);
    doc.clear();

    DeserializationError error = deserializeJson(doc, payload);
    if (!error)
    {
        strncpy(prof->Username,doc["Profile"]["Username"],sizeof(prof->Username));
        prof->CoinPriceBitCloutNanos = doc["Profile"]["CoinPriceBitCloutNanos"];
        prof->CoinsInCirculationNanos = doc["Profile"]["CoinEntry"]["CoinsInCirculationNanos"];
        strcpy(prof->PublicKeyBase58Check,doc["Profile"]["PublicKeyBase58Check"]);

    }else{
        debug_print("Json Error");
    }
}

char * DeSoLib::getUsersStateless(const char *messagePayload){
    return postRequest(RoutePathGetUsersStateless,messagePayload);
}

void DeSoLib::updateUsersStateless(const char *PublicKeysBase58Check,bool skipHodlings,Profile *prof){
    static char messagePayload[200];
    DynamicJsonDocument doc(200);
    doc["PublicKeysBase58Check"][0]=PublicKeysBase58Check;
    doc["skipHodlings"]=skipHodlings;
   
    serializeJson(doc, messagePayload);
    const char *payload = getUsersStateless(messagePayload);
    StaticJsonDocument<200> filter;
    filter["UserList"][0]["BalanceNanos"] = true;

    // Deserialize the document
    deserializeJson(doc, payload, DeserializationOption::Filter(filter));
    prof->BalanceNanos = doc["UserList"][0]["BalanceNanos"];
    // Print the result
    //serializeJsonPretty(doc, Serial);
}
DeSoLib::~DeSoLib()
{
}
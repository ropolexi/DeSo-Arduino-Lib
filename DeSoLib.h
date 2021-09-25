#ifndef DeSoLib_h
#define DeSoLib_h

#include <HTTPClient.h>
#include "WiFi.h"
#include <stdlib.h>

#define DEBUG_LOG false

#define debug_print(...) \
            do { if (DEBUG_LOG) Serial.print(__VA_ARGS__); } while (0)
class DeSoLib{
    public:
    WiFiClientSecure espClientSecure;
    DeSoLib();
    const char *RoutePathHealthCheck = "/api/v0/health-check";
    const char *ExchangeRateRoute = "/api/v0/get-exchange-rate";
    const char *RoutePathGetSingleProfile = "/api/v0/get-single-profile";
    struct Node{
        char url[50];
        bool status=false;
        const char* caRootCert;

    };
    struct Profile{
        char PublicKeyBase58Check[56];
        char Username[20];
        double CoinsInCirculationNanos;
        double CoinPriceBitCloutNanos;
    };

    char buff_small_1[200];
    char buff_small_2[200];
    char buff_large[1024];//heavy usage on web response

    double USDCentsPerBitCloutExchangeRate;
    double USDCentsPerBitcoinExchangeRate;
   
    int selectedNodeIndex=0;
    void addNodePath(const char* url,const char* cert);
    int getMaxNodes();
    void selectDefaultNode(int index);
    char* getSelectedNodeUrl();
    bool getSelectedNodeStatus();
    char* getRequest(const char* apiPath);
    char * postRequest(const char* apiPath,const char *data);
    char* getNodeHealthCheck();
    void updateNodeHealthCheck();
    char* getExchangeRates();
    void updateExchangeRates();
    char * getSingleProfile(const char *messagePayload);
    void updateSingleProfile(const char *username,const char *PublicKeyBase58CheckProfile,Profile *prof);

    ~DeSoLib();

    private:
    std::vector<Node> nodePaths;

};
#endif
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
    const char *RoutePathGetUsersStateless = "/api/v0/get-users-stateless";
    const char *RoutePathGetHodlersForPublicKey = "/api/v0/get-hodlers-for-public-key";
    const char *RoutePathGetPostsForPublicKey = "/api/v0/get-posts-for-public-key";
    const char *RoutePathGetBalance = "/api/v1/balance";

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
        double BalanceNanos;
        double UnconfirmedBalanceNanos;
        double TotalHODLBalanceClout;
        int TotalHodleNum;
        char TopHodlersUserNames[10][20];
        int lastPostLikes;
        int lastPostDiamonds;
        int lastNPostLikes;
        int lastNPostDiamonds;

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
    const char* getRequest(const char* apiPath);
    const char * postRequest(const char* apiPath,const char *data);
    const char* getNodeHealthCheck();
    int updateNodeHealthCheck();
    const char* getExchangeRates();
    int updateExchangeRates();
    const char * getSingleProfile(const char *messagePayload);
    int updateSingleProfile(const char *username,const char *PublicKeyBase58Check,Profile *prof);
    const char * getUsersStateless(const char *messagePayload);
    int updateUsersStateless(const char *PublicKeysBase58Check,bool SkipHodlings,Profile *prof);
    const char *getHodlersForPublicKey(const char *messagePayload);
    int updateHodlersForPublicKey(const char *username,const char *PublicKeyBase58Check,int NumToFetch,Profile *prof);
    void clearTopHodlersUserNames(Profile *prof);
    const char *getPostsForPublicKey(const char *messagePayload);
    int updateLastPostForPublicKey(const char *PublicKeysBase58Check,Profile *prof);
    int updateLastNumPostsForPublicKey(const char *PublicKeysBase58Check,int NumToFetch,Profile *prof);
    const char * getUserBalance(const char *messagePayload);
    int updateUsersBalance(const char *PublicKeysBase58Check,Profile *prof);
    ~DeSoLib();

    private:
    std::vector<Node> nodePaths;

};
#endif
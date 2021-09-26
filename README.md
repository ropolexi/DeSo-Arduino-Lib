# DeSo Arduino Library

## Introduction
This arduino library is for the DeSo (Decentralized Social) Network Blockchain.

## Features
- Multiple DeSo Nodes supported for decentralization 
- DeSo Coin Value
- Creator Coin Price using username or PublicKey
- Wallet Balance


## Serial Output Results
```
DeSo Node: https://bitclout.com
Node Status: Synced OK
DeSo Coin Value: $130.34
=======Profile========
Username: ropolexi
PublicKey: BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8
Creator Coin Price: $5.20
Wallet Balance:0.30
======================

DeSo Node: https://nachoaverage.com
Node Status: Synced OK
DeSo Coin Value: $130.34
=======Profile========
Username: ropolexi
PublicKey: BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8
Creator Coin Price: $5.20
Wallet Balance:0.30
======================

DeSo Node: https://members.giftclout.com
Node Status: Synced OK
DeSo Coin Value: $130.34
=======Profile========
Username: ropolexi
PublicKey: BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8
Creator Coin Price: $5.20
Wallet Balance:0.30
======================
```
## Device Supported

ESP32 Module (US $5.99)

https://www.ebay.com/itm/312621586322?hash=item48c9b2d792:g:BtUAAOSwYMlc4sAF


## Dependency Libraries
ArduinoJson - https://github.com/bblanchon/ArduinoJson

## Limitations
Tested an account with the following parameters
- following 540
- assets (Hodle) 13
No low memory issues.

But if the number of following is huge (more than 600) and number of assets are huge number(greater than 50) 
account balance or all the assets hodle balance will not be shown due to memeory limitation.


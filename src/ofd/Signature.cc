#include <sodium.h>
#include <stdint.h>
#include <inttypes.h>
#include <string>
#include "ofd/Signature.h"
#include "utils/logger.h"

using namespace ofd;

void test_libsodium(){
    const char *msg = "test";
    size_t msgSize = 4;

    uint8_t pk[crypto_sign_PUBLICKEYBYTES];
    uint8_t sk[crypto_sign_SECRETKEYBYTES];
    crypto_sign_keypair(pk, sk);

    uint8_t signedMessage[crypto_sign_BYTES + msgSize];
    unsigned long long signedMessageLen = 0;

    crypto_sign(signedMessage, &signedMessageLen, (const uint8_t *)msg, msgSize, sk);


    uint8_t unsignedMessage[msgSize+1];
    unsigned long long unsignedMessageLen = 0;
    if ( crypto_sign_open(unsignedMessage, &unsignedMessageLen, 
                signedMessage, signedMessageLen, pk) != 0 ){
        LOG(ERROR) << "Message sign is wrong.";
    } else {
        unsignedMessage[unsignedMessageLen] = '\0';
        LOG(INFO) << "Success signature. Message: " << std::string((const char*)unsignedMessage);
    }
}

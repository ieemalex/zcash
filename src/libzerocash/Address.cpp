/**
 * @file       Address.cpp
 *
 * @brief      Address and PublicAddress classes for the Zerocash library.
 *
 * @author     Christina Garman, Matthew Green, and Ian Miers
 * @date       April 2014
 *
 * @copyright  Copyright 2014 Christina Garman, Matthew Green, and Ian Miers
 * @license    This project is released under the MIT license.
 **/

//#include <stdexcept>

#include <cryptopp/osrng.h>
using CryptoPP::AutoSeededRandomPool;

#include <cryptopp/eccrypto.h>
using CryptoPP::ECP;    // Prime field
using CryptoPP::ECIES;

//#include <cryptopp/asn.h>
#include <cryptopp/oids.h>
namespace ASN1 = CryptoPP::ASN1;

#include <cryptopp/filters.h>
using CryptoPP::StringSink;
using CryptoPP::StringStore;

#include "Zerocash.h"
#include "Address.h"

namespace libzerocash {

//PublicAddress class
PublicAddress::PublicAddress(): a_pk(a_pk_size) {
    this->pk_enc = "";
}

PublicAddress::PublicAddress(const std::vector<unsigned char>& a_sk, const std::string sk_enc): a_pk(a_pk_size) {
    createPublicAddress(a_sk, sk_enc);
}

void PublicAddress::createPublicAddress(const std::vector<unsigned char>& a_sk, const std::string sk_enc) {
    //a_pk
    std::vector<bool> a_sk_bool(a_sk_size * 8);
    convertBytesVectorToVector(a_sk, a_sk_bool);

    std::vector<bool> zeros_256(256, 0);

    std::vector<bool> a_pk_internal;
    concatenateVectors(a_sk_bool, zeros_256, a_pk_internal);

    std::vector<bool> a_pk_bool(a_pk_size * 8);
    hashVector(a_pk_internal, a_pk_bool);

    convertVectorToBytesVector(a_pk_bool, this->a_pk);

    //pk_enc
    ECIES<ECP>::PublicKey publicKey;

    ECIES<ECP>::PrivateKey decodedPrivateKey;
    decodedPrivateKey.Load(StringStore(sk_enc).Ref());

    decodedPrivateKey.MakePublicKey(publicKey); 

    std::string encodedPublicKey;
    publicKey.Save(StringSink(encodedPublicKey).Ref());

    this->pk_enc = encodedPublicKey;
}

const std::string PublicAddress::getEncryptionPublicKey() const {
    return this->pk_enc;
}

const std::vector<unsigned char>& PublicAddress::getPublicAddressSecret() const {
    return this->a_pk;
}

bool PublicAddress::operator==(const PublicAddress& rhs) const {
	return ((this->a_pk == rhs.a_pk) && (this->pk_enc == rhs.pk_enc));
}

bool PublicAddress::operator!=(const PublicAddress& rhs) const {
	return !(*this == rhs);
}

//Address class
Address::Address(): addr_pk(), a_sk(a_sk_size) {
    //a_sk
    unsigned char a_sk_bytes[a_sk_size];
    getRandBytes(a_sk_bytes, a_sk_size);
    convertBytesToBytesVector(a_sk_bytes, this->a_sk);

    //sk_enc
    AutoSeededRandomPool prng;

    ECIES<ECP>::PrivateKey privateKey;
    privateKey.Initialize(prng, ASN1::secp256r1());

    std::string encodedPrivateKey;

    privateKey.Save(StringSink(encodedPrivateKey).Ref());

    this->sk_enc = encodedPrivateKey;

    addr_pk.createPublicAddress(this->a_sk, this->sk_enc);
}

const PublicAddress& Address::getPublicAddress() const {
	return this->addr_pk;
}

const std::string Address::getEncryptionSecretKey() const {
    return this->sk_enc;
}

const std::vector<unsigned char>& Address::getAddressSecret() const {
    return this->a_sk;
}

bool Address::operator==(const Address& rhs) const {
	return ((this->a_sk == rhs.a_sk) && (this->sk_enc == rhs.sk_enc) && (this->addr_pk == rhs.addr_pk));
}

bool Address::operator!=(const Address& rhs) const {
	return !(*this == rhs);
}

} /* namespace libzerocash */

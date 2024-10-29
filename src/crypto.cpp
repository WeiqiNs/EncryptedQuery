#include "crypto.hpp"

const EVP_CIPHER* Aes::get_cipher() const{
    switch (byte_length){
    case 16:
        return EVP_aes_128_cbc();
    case 24:
        return EVP_aes_192_cbc();
    case 32:
        return EVP_aes_256_cbc();
    default:
        throw std::invalid_argument("Key length must be 128, 192, or 256 bits.");
    }
}

Aes::Aes(const int& key_length){
    if (key_length != 128 && key_length != 192 && key_length != 256){
        throw std::invalid_argument("Key length must be 128, 192, or 256 bits.");
    }

    byte_length = key_length / 8;

    // Sample a random key and an IV.
    key.resize(byte_length);
    RAND_bytes(key.data(), byte_length);
}

CharVec Aes::get_key(){ return key; }

CharVec Aes::encrypt(const CharVec& plaintext) const{
    CharVec iv(EVP_MAX_IV_LENGTH);
    RAND_bytes(iv.data(), EVP_MAX_IV_LENGTH);
    CharVec ciphertext(plaintext.size() + EVP_CIPHER_block_size(get_cipher()));

    int len;
    const int plaintext_len = static_cast<int>(plaintext.size());

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create EVP_CIPHER_CTX");

    if (EVP_EncryptInit_ex(ctx, get_cipher(), nullptr, key.data(), iv.data()) != 1){
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex failed");
    }

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext_len) != 1){
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }

    int ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1){
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }

    EVP_CIPHER_CTX_free(ctx);

    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);
    ciphertext.insert(ciphertext.begin(), iv.begin(), iv.end());

    return ciphertext;
}

CharVec Aes::decrypt(const CharVec& ciphertext) const{
    const CharVec iv(ciphertext.begin(), ciphertext.begin() + EVP_MAX_IV_LENGTH);
    const CharVec actual_ciphertext(ciphertext.begin() + EVP_MAX_IV_LENGTH, ciphertext.end());
    CharVec plaintext(actual_ciphertext.size() + EVP_CIPHER_block_size(get_cipher()));

    int len;
    const int ciphertext_len = static_cast<int>(actual_ciphertext.size());

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create EVP_CIPHER_CTX");

    if (EVP_DecryptInit_ex(ctx, get_cipher(), nullptr, key.data(), iv.data()) != 1){
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptInit_ex failed");
    }

    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, actual_ciphertext.data(), ciphertext_len) != 1){
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptUpdate failed");
    }

    int plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1){
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptFinal_ex failed");
    }

    EVP_CIPHER_CTX_free(ctx);

    plaintext_len += len;
    plaintext.resize(plaintext_len);

    return plaintext;
}

Hash::Hash(){
    iv.resize(HASH_SIZE);
    key.resize(HASH_SIZE);
    RAND_bytes(iv.data(), HASH_SIZE);
    RAND_bytes(key.data(), HASH_SIZE);
}

CharVec Hash::get_iv(){ return iv; }

CharVec Hash::get_key(){ return key; }

CharVec Hash::digest(const CharVec& data) const{
    CharVec ciphertext(data.size() + EVP_CIPHER_block_size(EVP_aes_128_cbc()));

    int len;
    const int plaintext_len = static_cast<int>(data.size());

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create EVP_CIPHER_CTX");

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key.data(), iv.data()) != 1){
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex failed");
    }

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, data.data(), plaintext_len) != 1){
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }

    int ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1){
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }

    EVP_CIPHER_CTX_free(ctx);

    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    return {ciphertext.end() - HASH_SIZE, ciphertext.end()};
}

FpVec Hash::digest_int_vec(const IntVec& x) const{
    FpVec r;

    for (const int& i : x) r.push_back(Helper::char_vec_to_fp(digest(Helper::int_to_char_vec(i))));

    return r;
}


FpVec Hash::digest_str_vec(const StrVec& x) const{
    FpVec r;

    for (const str& i : x) r.push_back(Helper::char_vec_to_fp(digest(Helper::str_to_char_vec(i))));

    return r;
}

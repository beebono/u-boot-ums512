#ifndef _AUTHENTICATION_H_
#define _AUTHENTICATION_H_

#define SHA1_RNG_FAILED -1
#define GET_UID_FAILED -2
#define ENCRYPT_FAILED -3
#define DECRYPT_FAILED -4
#define AUTH_FAILED -5
#define AUTH_SUCCESS 1

/* arguments for function authentication() */
#define ENCRYPT 1
#define DECRYPT 0

/* the size of result after encrypting with RSA-2048 */
#define ENCRYPT_SUCCESSED 256

/* get rand */
extern int prand_gen(u8 *rand, size_t rand_len);

/* get socid */
extern int sprd_get_chip_hex_uid (char *buf);

/* SHA256 */
extern void sha256_csum_wd_sw (const unsigned char *input,unsigned int ilen,unsigned char *output,unsigned int chunk_sz);

/*
 * flg = 1 : respond to authentication requests
 * [OUT]Msg : encrypted rand||socid with A
 *Return value:
 * sizeof(M1): respond successed
 * <= 0 : respond failed
 *
 * flg = 0 for return authentication result
 * [IN]Msg : encrypted R2 with d
 *Return value:
 * 0 : verify successed
 * <0 : verify failed
 */
extern int authentication (unsigned char *msg, int flg);

#endif

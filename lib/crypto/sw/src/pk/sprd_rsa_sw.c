/*
 * Copyright (c) 2019, Spreadtrum Communications.
 *
 * The above copyright notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The software implementation of rsa algorithm
 *
 */

#include <secureboot/sec_string.h>
#include <secureboot/sec_common.h>
#include <sprd_rsa.h>
#include <pk1.h>
#include <sprd_crypto_sw.h>
#include <base64.h>
#include <stdlib.h>
#include <sprd_pkcs1.h>
#include <sprd_crypto_log.h>

#define		BIGINT_MAXLEN 66
#define		DEC 10
#define		HEX 16
#define		RAW_RANDOMDATA_LEN 32

#define DEBUG_RSA_DECRYPT 0

typedef struct {
	int m_iLength32;
	unsigned int m_ulValue[BIGINT_MAXLEN];
} SBigInt;

SBigInt RSA_C, RSA_M, RSA_N, RSA_D, RSA_E;
SBigInt RSA_P, RSA_Q;

static unsigned char random_data[RAW_RANDOMDATA_LEN] = {0};


const static int PrimeTable[550] =
{ 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
	157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317,
	331, 337, 347, 349, 353,
	359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547,
	557, 563, 569, 571, 577,
	587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769,
	773, 787, 797, 809, 811,
	821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019,
	1021, 1031, 1033, 1039,
	1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
	1229, 1231, 1237, 1249,
	1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447,
	1451, 1453, 1459, 1471,
	1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627,
	1637, 1657, 1663, 1667,
	1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873,
	1877, 1879, 1889, 1901,
	1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089,
	2099, 2111, 2113, 2129,
	2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333,
	2339, 2341, 2347, 2351,
	2357, 2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551,
	2557, 2579, 2591, 2593,
	2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 2753, 2767,
	2777, 2789, 2791, 2797,
	2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011,
	3019, 3023, 3037, 3041,
	3049, 3061, 3067, 3079, 3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259,
	3271, 3299, 3301, 3307,
	3313, 3319, 3323, 3329, 3331, 3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511,
	3517, 3527, 3529, 3533,
	3539, 3541, 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719,
	3727, 3733, 3739, 3761,
	3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923, 3929, 3931, 3943, 3947,
	3967, 3989, 4001
};

static void RSA_initialize()
{
	int i;

	for (i = 0; i < BIGINT_MAXLEN; i++) {
		RSA_C.m_ulValue[i] = 0;
		RSA_D.m_ulValue[i] = 0;
		RSA_E.m_ulValue[i] = 0;
		RSA_M.m_ulValue[i] = 0;
		RSA_N.m_ulValue[i] = 0;
		RSA_P.m_ulValue[i] = 0;
		RSA_Q.m_ulValue[i] = 0;
	}
	RSA_C.m_iLength32 = 1;
	RSA_D.m_iLength32 = 1;
	RSA_E.m_iLength32 = 1;
	RSA_M.m_iLength32 = 1;
	RSA_N.m_iLength32 = 1;
	RSA_P.m_iLength32 = 1;
	RSA_Q.m_iLength32 = 1;
}

static void RSA_deinitialize()
{
}

static int Cmp(SBigInt * sIn, SBigInt * sIn2)
{
	int i;

	if (sIn->m_iLength32 > sIn2->m_iLength32)
		return 1;

	if (sIn->m_iLength32 < sIn2->m_iLength32)
		return -1;

	for (i = sIn->m_iLength32 - 1; i >= 0; i--) {
		if (sIn->m_ulValue[i] > sIn2->m_ulValue[i])
			return 1;
		if (sIn->m_ulValue[i] < sIn2->m_ulValue[i])
			return -1;
	}

	return 0;
}

static void Mov_bigint(SBigInt * sOut, SBigInt * sIn)
{
	int i;

	sOut->m_iLength32 = sIn->m_iLength32;

	for (i = 0; i < BIGINT_MAXLEN; i++)
		sOut->m_ulValue[i] = sIn->m_ulValue[i];

	return;
}

static void Mov_long(SBigInt * sOut, uint64_t A)
{
	int i;

	if (A > 0xffffffff) {
		sOut->m_iLength32 = 2;
		sOut->m_ulValue[1] = (uint64_t)(A >> 32);
		sOut->m_ulValue[0] = (uint64_t)A;
	} else {
		sOut->m_iLength32 = 1;
		sOut->m_ulValue[0] = (uint64_t)A;
	}

	for (i = sOut->m_iLength32; i < BIGINT_MAXLEN; i++)
		sOut->m_ulValue[i] = 0;
}

static void Add_bigint(SBigInt * sOut, SBigInt * sIn, SBigInt * sIn2)
{
	unsigned carry = 0;
	uint64_t sum = 0;
	int i;

	if (sIn->m_iLength32 < sIn2->m_iLength32)
		sOut->m_iLength32 = sIn2->m_iLength32;
	else
		sOut->m_iLength32 = sIn->m_iLength32;

	for (i = 0; i < sOut->m_iLength32; i++) {
		sum = sIn->m_ulValue[i];
		sum = sum + sIn2->m_ulValue[i] + carry;
		sOut->m_ulValue[i] = (uint64_t)sum;
		carry = (unsigned)(sum >> 32);
	}

	sOut->m_ulValue[sOut->m_iLength32] = carry;
	sOut->m_iLength32 += carry;
	for (i = sOut->m_iLength32; i < BIGINT_MAXLEN; i++)
		sOut->m_ulValue[i] = 0;

	return;
}

static void Add_long(SBigInt * sOut, SBigInt * sIn, uint64_t ulIn)
{
	uint64_t sum;
	int i;

	Mov_bigint(sOut, sIn);
	sum = sIn->m_ulValue[0];
	sum += ulIn;
	sOut->m_ulValue[0] = (uint64_t)sum;

	if (sum > 0xffffffff) {
		i = 1;
		while (sIn->m_ulValue[i] == 0xffffffff) {
			sOut->m_ulValue[i] = 0;
			i++;
		}
		sOut->m_ulValue[i]++;
		if (sOut->m_iLength32 == i)
			sOut->m_iLength32++;
	}

	return;
}

static void Sub_bigint(SBigInt * sOut, SBigInt * sIn, SBigInt * sIn2)
{
	unsigned carry = 0;
	uint64_t num;
	int i;

	if (Cmp(sIn, sIn2) <= 0) {
		Mov_long(sOut, 0);
		return;
	}

	sOut->m_iLength32 = sIn->m_iLength32;
	for (i = 0; i < sIn->m_iLength32; i++) {
		if ((sIn->m_ulValue[i] > sIn2->m_ulValue[i]) || ((sIn->m_ulValue[i] == sIn2->m_ulValue[i]) && (carry == 0))) {
			sOut->m_ulValue[i] = sIn->m_ulValue[i] - carry - sIn2->m_ulValue[i];
			carry = 0;
		} else {
			num = 0x100000000 + sIn->m_ulValue[i];
			sOut->m_ulValue[i] = (uint64_t)(num - carry - sIn2->m_ulValue[i]);
			carry = 1;
		}
	}

	while (sOut->m_ulValue[sOut->m_iLength32 - 1] == 0)
		sOut->m_iLength32--;

	for (i = sOut->m_iLength32; i < BIGINT_MAXLEN; i++)
		sOut->m_ulValue[i] = 0;

	return;
}

static void Sub_long(SBigInt * sOut, SBigInt * sIn, uint64_t ulIn)
{
	uint64_t num;
	int i;

	sOut->m_iLength32 = sIn->m_iLength32;
	if (sIn->m_ulValue[0] >= ulIn) {
		sOut->m_ulValue[0] = sIn->m_ulValue[0] - ulIn;
		for (i = 1; i < BIGINT_MAXLEN; i++)
			sOut->m_ulValue[i] = sIn->m_ulValue[i];
		return;
	}

	if (sIn->m_iLength32 == 1) {
		Mov_long(sOut, 0);
		return;
	}

	num = 0x100000000 + sIn->m_ulValue[0];
	sOut->m_ulValue[0] = (uint64_t)(num - ulIn);
	i = 1;
	while (sIn->m_ulValue[i] == 0) {
		sOut->m_ulValue[i] = 0xffffffff;
		i++;
	}
	sOut->m_ulValue[i] = sIn->m_ulValue[i] - 1;
	if (sOut->m_ulValue[i] == 0)
		sOut->m_iLength32--;
	for (i = sOut->m_iLength32; i < BIGINT_MAXLEN; i++)
		sOut->m_ulValue[i] = 0;

	return;
}

static void Mul_long(SBigInt * sOut, SBigInt * sIn, uint64_t ulIn)
{
	uint64_t mul;
	uint64_t carry;
	int i;

	carry = 0;
	sOut->m_iLength32 = sIn->m_iLength32;
	for (i = 0; i < sOut->m_iLength32; i++) {
		mul = sIn->m_ulValue[i];
		mul = mul * ulIn + carry;
		sOut->m_ulValue[i] = (uint64_t)mul;
		carry = (uint64_t)(mul >> 32);
	}

	if (carry) {
		sOut->m_ulValue[sOut->m_iLength32] = carry;
		sOut->m_iLength32++;
	}
	for (i = sOut->m_iLength32; i < BIGINT_MAXLEN; i++)
		sOut->m_ulValue[i] = 0;

	return;
}

static void Mul_bigint(SBigInt * sOut, SBigInt * sIn, SBigInt * sIn2)
{
	uint64_t sum, mul = 0, carry = 0;
	int i, j, k;

	if (sIn2->m_iLength32 == 1) {
		Mul_long(sOut, sIn, sIn2->m_ulValue[0]);
		return;
	}
	sOut->m_iLength32 = sIn->m_iLength32 + sIn2->m_iLength32 - 1;

	for (i = 0; i < sOut->m_iLength32; i++) {
		sum = carry;
		carry = 0;
		for (j = 0; j < sIn2->m_iLength32; j++) {
			k = i - j;
			if ((k >= 0) && (k < sIn->m_iLength32)) {
				mul = sIn->m_ulValue[k];
				mul = mul * sIn2->m_ulValue[j];
				carry += mul >> 32;
				mul = mul & 0xffffffff;
				sum += mul;
			}
		}
		carry += sum >> 32;
		sOut->m_ulValue[i] = (uint64_t)sum;
	}

	if (carry) {
		sOut->m_iLength32++;
		sOut->m_ulValue[sOut->m_iLength32 - 1] = (uint64_t)carry;
	}

	for (i = sOut->m_iLength32; i < BIGINT_MAXLEN; i++)
		sOut->m_ulValue[i] = 0;

	return;
}

static void Div_long(SBigInt * sOut, SBigInt * sIn, uint64_t ulIn)
{
	uint64_t div, mul;
	uint64_t carry = 0;
	int i;

	sOut->m_iLength32 = sIn->m_iLength32;
	if (sIn->m_iLength32 == 1) {
		sOut->m_ulValue[0] = sIn->m_ulValue[0] / ulIn;
		return;
	}

	for (i = sIn->m_iLength32 - 1; i >= 0; i--) {
		div = carry;
		div = (div << 32) + sIn->m_ulValue[i];
		sOut->m_ulValue[i] = (uint64_t)(div / ulIn);
		mul = (div / ulIn) * ulIn;
		carry = (uint64_t)(div - mul);
	}

	if (sOut->m_ulValue[sOut->m_iLength32 - 1] == 0)
		sOut->m_iLength32--;

	for (i = sOut->m_iLength32; i < BIGINT_MAXLEN; i++)
		sOut->m_ulValue[i] = 0;

	return;
}

static void Div_bigint(SBigInt * sOut, SBigInt * sIn, SBigInt * sIn2)
{
	unsigned i, len;
	uint64_t num, div;
	SBigInt sTemp1, sTemp2, sTemp3;

	if (sIn2->m_iLength32 == 1) {
		Div_long(sOut, sIn, sIn2->m_ulValue[0]);
		return;
	}

	Mov_bigint(&sTemp1, sIn);
	Mov_long(sOut, 0);
	while (Cmp(&sTemp1, sIn2) >= 0) {
		div = sTemp1.m_ulValue[sTemp1.m_iLength32 - 1];
		num = sIn2->m_ulValue[sIn2->m_iLength32 - 1];
		len = sTemp1.m_iLength32 - sIn2->m_iLength32;

		if ((div == num) && (len == 0)) {
			Add_long(sOut, sOut, 1);
			break;
		}

		if ((div <= num) && len) {
			len--;
			div = (div << 32) + sTemp1.m_ulValue[sTemp1.m_iLength32 - 2];
		}

		div = div / (num + 1);
		Mov_long(&sTemp2, div);

		if (len) {
			sTemp2.m_iLength32 += len;
			for (i = sTemp2.m_iLength32 - 1; i >= len; i--)
				sTemp2.m_ulValue[i] = sTemp2.m_ulValue[i - len];
			for (i = 0; i < len; i++)
				sTemp2.m_ulValue[i] = 0;
		}

		Add_bigint(sOut, sOut, &sTemp2);
		Mul_bigint(&sTemp3, sIn2, &sTemp2);
		Sub_bigint(&sTemp1, &sTemp1, &sTemp3);
	}

	return;
}

static void Mod_bigint(SBigInt * sOut, SBigInt * sIn, SBigInt * sIn2)
{
	SBigInt sTemp1, sTemp2;
	uint64_t div, num;
	unsigned i, len;

	Mov_bigint(sOut, sIn);
	while (Cmp(sOut, sIn2) >= 0) {
		div = sOut->m_ulValue[sOut->m_iLength32 - 1];
		num = sIn2->m_ulValue[sIn2->m_iLength32 - 1];
		len = sOut->m_iLength32 - sIn2->m_iLength32;

		if ((div == num) && (len == 0)) {
			Sub_bigint(sOut, sOut, sIn2);
			break;
		}

		if ((div <= num) && len) {
			len--;
			div = (div << 32) + sOut->m_ulValue[sOut->m_iLength32 - 2];
		}

		div = div / (num + 1);
		Mov_long(&sTemp1, div);
		Mul_bigint(&sTemp2, sIn2, &sTemp1);

		if (len) {
			sTemp2.m_iLength32 += len;
			for (i = sTemp2.m_iLength32 - 1; i >= len; i--)
				sTemp2.m_ulValue[i] = sTemp2.m_ulValue[i - len];
			for (i = 0; i < len; i++)
				sTemp2.m_ulValue[i] = 0;
		}

		Sub_bigint(sOut, sOut, &sTemp2);
	}

	return;
}

static uint64_t Mod_long(SBigInt * sIn, uint64_t ulIn)
{
	uint64_t div;
	uint64_t carry = 0;
	int i;

	if (sIn->m_iLength32 == 1)
		return sIn->m_ulValue[0] % ulIn;

	for (i = sIn->m_iLength32 - 1; i >= 0; i--) {
		div = sIn->m_ulValue[i] + carry * 0x100000000;
		carry = (uint64_t)(div % ulIn);
	}

	return carry;
}

static void SetParameter_int(SBigInt * sOut, unsigned char ulIn[], int iLength32)
{
	int i;

	sec_memcpy(sOut->m_ulValue, ulIn, iLength32 * sizeof(unsigned int));

	for (i = iLength32; i < BIGINT_MAXLEN; i++)
		sOut->m_ulValue[i] = 0;

	sOut->m_iLength32 = iLength32;

	return;
}

static void SetParameter_char_openssl(SBigInt * sOut, unsigned char *str, int strLen)
{
	invert_char(str, strLen);
	SetParameter_int(sOut, str, strLen >> 2);
	invert_char(str, strLen);		/* recovery the original input */

	return;
}

static void Put(unsigned char *str, int str_buflen, SBigInt * sOut, unsigned int system)
{
	unsigned char t[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	int a;
	unsigned char ch;
	SBigInt sTemp, sTemp2;
	int index = str_buflen - 1;

	memset(&sTemp2, 0, sizeof(SBigInt));
	if ((sOut->m_iLength32 == 1) && (sOut->m_ulValue[0] == 0)) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}

	str[index] = '\0';
	Mov_bigint(&sTemp, sOut);
	while (sTemp.m_ulValue[sTemp.m_iLength32 - 1] > 0) {
		a = Mod_long(&sTemp, system);
		ch = t[a];

		index -= 1;
		str[index] = ch;

		Div_long(&sTemp2, &sTemp, system);
		Mov_bigint(&sTemp, &sTemp2);
	}

	if (index > 0) {
		int i = 0;
		while (index < str_buflen)
			str[i++] = str[index++];
	}
}

static void RsaTrans(SBigInt * sOut, SBigInt * sIn, SBigInt * sE, SBigInt * sN)
{
	int i, j, k;
	int n;
	uint64_t num;
	SBigInt sTemp1, sTemp2;

	k = sE->m_iLength32 * 32 - 32;
	num = sE->m_ulValue[sE->m_iLength32 - 1];
	while (num) {
		num = num >> 1;
		k++;
	}

	Mov_bigint(sOut, sIn);
	for (i = k - 2; i >= 0; i--) {
		Mul_long(&sTemp1, sOut, sOut->m_ulValue[sOut->m_iLength32 - 1]);
		Mod_bigint(&sTemp1, &sTemp1, sN);

		for (n = 1; n < sOut->m_iLength32; n++) {
			for (j = sTemp1.m_iLength32; j > 0; j--)
				sTemp1.m_ulValue[j] = sTemp1.m_ulValue[j - 1];
			sTemp1.m_ulValue[0] = 0;
			sTemp1.m_iLength32++;
			Mul_long(&sTemp2, sOut, sOut->m_ulValue[sOut->m_iLength32 - n - 1]);
			Add_bigint(&sTemp1, &sTemp1, &sTemp2);
			Mod_bigint(&sTemp1, &sTemp1, sN);
		}

		Mov_bigint(sOut, &sTemp1);
		if ((sE->m_ulValue[i >> 5] >> (i & 31)) & 1) {
			Mul_long(&sTemp1, sIn, sOut->m_ulValue[sOut->m_iLength32 - 1]);
			Mod_bigint(&sTemp1, &sTemp1, sN);

			for (n = 1; n < sOut->m_iLength32; n++) {
				for (j = sTemp1.m_iLength32; j > 0; j--)
					sTemp1.m_ulValue[j] = sTemp1.m_ulValue[j - 1];
				sTemp1.m_ulValue[0] = 0;
				sTemp1.m_iLength32++;
				Mul_long(&sTemp2, sIn, sOut->m_ulValue[sOut->m_iLength32 - n - 1]);
				Add_bigint(&sTemp1, &sTemp1, &sTemp2);
				Mod_bigint(&sTemp1, &sTemp1, sN);
			}

			Mov_bigint(sOut, &sTemp1);
		}

	}

	return;
}

/****************************************************************************************
 ****************************************************************************************/
static int prv_Encrypt()
{
	if (Cmp(&RSA_M, &RSA_N) >= 0) {
		return -1;
	}

	RsaTrans(&RSA_C, &RSA_M, &RSA_E, &RSA_N);

	return 1;
}

/****************************************************************************************
 ****************************************************************************************/
static int pub_Decrypt()
{
	if (Cmp(&RSA_C, &RSA_N) >= 0) {
		return -1;
	}

	RsaTrans(&RSA_M, &RSA_C, &RSA_D, &RSA_N);

	return 1;
}

/****************************************************************************************
 ****************************************************************************************/
static int pub_Encrypt()
{
	if (Cmp(&RSA_M, &RSA_N) >= 0) {
		return -1;
	}

	RsaTrans(&RSA_C, &RSA_M, &RSA_D, &RSA_N);

	return 1;
}

/****************************************************************************************
 ****************************************************************************************/
static int prv_Decrypt()
{
	if (Cmp(&RSA_C, &RSA_N) >= 0) {
		return -1;
	}

	RsaTrans(&RSA_M, &RSA_C, &RSA_E, &RSA_N);

	return 1;
}

int RSA_Sign(unsigned char *prv_E, unsigned char *mod_N, int bitLen_N, unsigned char *from, int flen, unsigned char *to)
{
	int byteLen_N = bitLen_N >> 3;
	unsigned char textpadded[byteLen_N];

	RSA_initialize();

	if (prv_E != NULL)
		SetParameter_char_openssl(&RSA_E, prv_E, byteLen_N);
	if (mod_N != NULL)
		SetParameter_char_openssl(&RSA_N, mod_N, byteLen_N);
	if (padding_add_pkcs_type_1(textpadded, byteLen_N, from, flen) <= 0)
		return SPRD_CRYPTO_ERROR;

	SetParameter_char_openssl(&RSA_M, textpadded, byteLen_N);
	if (prv_Encrypt() < 0)
		return SPRD_CRYPTO_ERROR;

	sec_memcpy(to, (unsigned char *)RSA_C.m_ulValue, RSA_C.m_iLength32 * sizeof(unsigned int));
	invert_char(to, byteLen_N);

	return SPRD_CRYPTO_SUCCESS;
}

int RSA_Sign_pss(unsigned char *prv_E, unsigned char *mod_N, int bitLen_N,
					unsigned char *from, int flen, unsigned char *to, sprd_crypto_algo_t hash_type, int sLen)
{
	int byteLen_N = bitLen_N >> 3;
	unsigned char textpadded[byteLen_N];

	RSA_initialize();

	if (prv_E != NULL)
		SetParameter_char_openssl(&RSA_E, prv_E, byteLen_N);
	if (mod_N != NULL)
		SetParameter_char_openssl(&RSA_N, mod_N, byteLen_N);

	if (sprd_pkcs1_pss_encode_sw(from, flen, sLen, hash_type, hash_type, 2048, textpadded, &byteLen_N) > 0)
		return SPRD_CRYPTO_ERROR;

	SetParameter_char_openssl(&RSA_M, textpadded, byteLen_N);
	if (prv_Encrypt() < 0)
		return SPRD_CRYPTO_ERROR;

	sec_memcpy(to, (unsigned char *)RSA_C.m_ulValue, RSA_C.m_iLength32 * sizeof(unsigned int));
	invert_char(to, byteLen_N);

	return SPRD_CRYPTO_SUCCESS;
}

int RSA_PrvEnc_No_Pad(unsigned char *prv_E, unsigned char *mod_N, int bitLen_N, unsigned char *from, int flen, unsigned char *to)
{
	int byteLen_N = bitLen_N >> 3;

	RSA_initialize();

	if (prv_E != NULL)
		SetParameter_char_openssl(&RSA_E, prv_E, byteLen_N);
	if (mod_N != NULL)
		SetParameter_char_openssl(&RSA_N, mod_N, byteLen_N);

	SetParameter_char_openssl(&RSA_M, from, flen);
	if (prv_Encrypt() < 0)
		return SPRD_CRYPTO_ERROR;

	sec_memcpy(to, (unsigned char *)RSA_C.m_ulValue, RSA_C.m_iLength32 * sizeof(unsigned int));
	invert_char(to, byteLen_N);

	return SPRD_CRYPTO_SUCCESS;
}

int RSA_Verify(unsigned char *pub_D, unsigned char *mod_N, int bitLen_N, unsigned char *from, unsigned char *to)
{
	int byteLen_N = bitLen_N >> 3;
	unsigned char text[byteLen_N];

	RSA_initialize();
	memset(text, 0, byteLen_N);

	if (pub_D != NULL)
		SetParameter_char_openssl(&RSA_D, pub_D, 4);
	if (mod_N != NULL)
		SetParameter_char_openssl(&RSA_N, mod_N, byteLen_N);

	SetParameter_char_openssl(&RSA_C, from, byteLen_N);
	if (pub_Decrypt() < 0)
		return SPRD_CRYPTO_ERROR;

	sec_memcpy(text, (unsigned char *)RSA_M.m_ulValue, RSA_M.m_iLength32 * sizeof(unsigned int));
	invert_char(text, byteLen_N);

	return padding_check_pkcs_type_1(to, byteLen_N, text + 1, byteLen_N - 1, byteLen_N);
}

int RSA_Verify_pss(unsigned char *pub_D, unsigned char *mod_N, int bitLen_N, unsigned char *from,
					int flen, unsigned char *plain, int plain_len, int res, sprd_crypto_algo_t hash_type, int sLen)
{
	int byteLen_N = bitLen_N >> 3;
	unsigned char text[byteLen_N];

	RSA_initialize();
	memset(text, 0, byteLen_N);

	if (pub_D != NULL)
		SetParameter_char_openssl(&RSA_D, pub_D, 4);
	if (mod_N != NULL)
		SetParameter_char_openssl(&RSA_N, mod_N, byteLen_N);

	SetParameter_char_openssl(&RSA_C, from, byteLen_N);
	if (pub_Decrypt() < 0)
		return SPRD_CRYPTO_ERROR;

	sec_memcpy(text, (unsigned char *)RSA_M.m_ulValue, RSA_M.m_iLength32 * sizeof(unsigned int));
	invert_char(text, byteLen_N);

	return sprd_pkcs1_pss_decode_sw(plain, plain_len, text, byteLen_N, sLen,
								hash_type, hash_type, bitLen_N, &res);
}

int RSA_PubDec_No_Pad(unsigned char *pub_D, unsigned char *mod_N, int bitLen_N, unsigned char *from, unsigned char *to)
{
	int byteLen_N = bitLen_N >> 3;

	RSA_initialize();

	if (pub_D != NULL)
		SetParameter_char_openssl(&RSA_D, pub_D, 4);
	if (mod_N != NULL)
		SetParameter_char_openssl(&RSA_N, mod_N, byteLen_N);

	SetParameter_char_openssl(&RSA_C, from, byteLen_N);
	if (pub_Decrypt() < 0)
		return SPRD_CRYPTO_ERROR;

	sec_memcpy(to, (unsigned char *)RSA_M.m_ulValue, RSA_M.m_iLength32 * sizeof(unsigned int));
	invert_char(to, byteLen_N);

	return byteLen_N;
}

int RSA_PubEnc(unsigned char *pub_D, unsigned char *mod_N, int bitLen_N, unsigned char *from, int flen, unsigned char *to)
{
	int byteLen_N = bitLen_N >> 3;
	unsigned char textpadded[byteLen_N];

	RSA_initialize();

	if (pub_D != NULL) {
		SetParameter_char_openssl(&RSA_D, pub_D, 4);
	}
	if (mod_N != NULL) {
		SetParameter_char_openssl(&RSA_N, mod_N, byteLen_N);
	}

	if (padding_add_pkcs_type_2(textpadded, byteLen_N, from, flen) <= 0)
		return SPRD_CRYPTO_ERROR;

	SetParameter_char_openssl(&RSA_M, textpadded, byteLen_N);
	if (pub_Encrypt() < 0)
		return SPRD_CRYPTO_ERROR;

	sec_memcpy(to, (unsigned char *)RSA_C.m_ulValue, RSA_C.m_iLength32 * sizeof(unsigned int));
	invert_char(to, byteLen_N);

	return SPRD_CRYPTO_SUCCESS;
}

int RSA_PubEnc_No_Pad(unsigned char *pub_D, unsigned char *mod_N, int bitLen_N, unsigned char *from, int flen, unsigned char *to)
{
	int byteLen_N = bitLen_N >> 3;

	RSA_initialize();

	if (pub_D != NULL) {
		SetParameter_char_openssl(&RSA_D, pub_D, 4);
	}
	if (mod_N != NULL) {
		SetParameter_char_openssl(&RSA_N, mod_N, byteLen_N);
	}

	SetParameter_char_openssl(&RSA_M, from, flen);
	if (pub_Encrypt() < 0)
		return SPRD_CRYPTO_ERROR;

	sec_memcpy(to, (unsigned char *)RSA_C.m_ulValue, RSA_C.m_iLength32 * sizeof(unsigned int));
	invert_char(to, byteLen_N);

	return SPRD_CRYPTO_SUCCESS;
}

int RSA_PrvDec(unsigned char *prv_E, unsigned char *mod_N, int bitLen_N, unsigned char *from, unsigned char *to)
{
	int byteLen_N = bitLen_N >> 3;
	unsigned char text[byteLen_N];

	RSA_initialize();
	memset(text, 0, byteLen_N);

	if (prv_E != NULL) {
		SetParameter_char_openssl(&RSA_E, prv_E, byteLen_N);
	}
	if (mod_N != NULL) {
		SetParameter_char_openssl(&RSA_N, mod_N, byteLen_N);
	}

	SetParameter_char_openssl(&RSA_C, from, byteLen_N);
	if (prv_Decrypt() < 0)
		return SPRD_CRYPTO_ERROR;

	sec_memcpy(text, (unsigned char *)RSA_M.m_ulValue, RSA_M.m_iLength32 * sizeof(unsigned int));

	invert_char(text, byteLen_N);

	return padding_check_pkcs_type_2(to, byteLen_N, text + 1, byteLen_N - 1, byteLen_N);
}

int RSA_PrvDec_No_Pad(unsigned char *prv_E, unsigned char *mod_N, int bitLen_N, unsigned char *from, unsigned char *to)
{
	int byteLen_N = bitLen_N >> 3;

	RSA_initialize();

	if (prv_E != NULL) {
		SetParameter_char_openssl(&RSA_E, prv_E, byteLen_N);
	}
	if (mod_N != NULL) {
		SetParameter_char_openssl(&RSA_N, mod_N, byteLen_N);
	}

	SetParameter_char_openssl(&RSA_C, from, byteLen_N);
	if (prv_Decrypt() < 0)
		return SPRD_CRYPTO_ERROR;

	sec_memcpy(to, (unsigned char *)RSA_M.m_ulValue, RSA_M.m_iLength32 * sizeof(unsigned int));

	invert_char(to, byteLen_N);

	return byteLen_N;
}

int RSA_PubEnc_oaep(unsigned char *pub_D, unsigned char *mod_N, int bitLen_N, unsigned char *from, int flen, unsigned char *to, sprd_crypto_algo_t hash_type)
{
	unsigned long byteLen_N = bitLen_N >> 3;
	unsigned char textpadded[(unsigned int)byteLen_N];

	RSA_initialize();

	if (pub_D != NULL) {
		SetParameter_char_openssl(&RSA_D, pub_D, 4);
	}
	if (mod_N != NULL) {
		SetParameter_char_openssl(&RSA_N, mod_N, (unsigned int)byteLen_N);
	}

	if (sprd_pkcs1_oaep_encode_sw(from, flen, NULL, 0, bitLen_N, hash_type, hash_type, textpadded, &byteLen_N) > 0)
		return SPRD_CRYPTO_ERROR;

	SetParameter_char_openssl(&RSA_M, textpadded, (unsigned int)byteLen_N);
	if (pub_Encrypt() < 0)
		return SPRD_CRYPTO_ERROR;

	sec_memcpy(to, (unsigned char *)RSA_C.m_ulValue, RSA_C.m_iLength32 * sizeof(unsigned int));
	invert_char(to, (unsigned int)byteLen_N);

	return SPRD_CRYPTO_SUCCESS;
}

int RSA_PrvDec_oaep(unsigned char *prv_E, unsigned char *mod_N, int bitLen_N, unsigned char *from, unsigned char *to, sprd_crypto_algo_t hash_type)
{
	int byteLen_N = bitLen_N >> 3;
	unsigned char text[byteLen_N];
	int res = 0;

	RSA_initialize();
	memset(text, 0, byteLen_N);

	if (prv_E != NULL) {
		SetParameter_char_openssl(&RSA_E, prv_E, byteLen_N);
	}
	if (mod_N != NULL) {
		SetParameter_char_openssl(&RSA_N, mod_N, byteLen_N);
	}

	SetParameter_char_openssl(&RSA_C, from, byteLen_N);
	if (prv_Decrypt() < 0)
		return SPRD_CRYPTO_ERROR;

	sec_memcpy(text, (unsigned char *)RSA_M.m_ulValue, RSA_M.m_iLength32 * sizeof(unsigned int));

	invert_char(text, byteLen_N);

	return sprd_pkcs1_oaep_decode_sw(text, byteLen_N, NULL, 0, bitLen_N, hash_type, hash_type, to, &byteLen_N, &res);
}
/****************************************************************************************
 ****************************************************************************************/
#define RAND_MAX ((1U << 31) - 1)
static int g_seed = 0;
static int read_reg = 1;

int myrand()
{
	if (read_reg) {
		g_seed = 504723;
		if (g_seed > 0)
			read_reg = 0;
	}

	return g_seed = (g_seed * 1103515245 + 12345) & RAND_MAX;
}

uint64_t Rand()
{
	int ulNum;

	ulNum = myrand() * 0x10000 + myrand();

	return ulNum;
}

/****************************************************************************************
 ****************************************************************************************/
int Rab(SBigInt * sIn)
{
	int i, j, iPassMark;
	SBigInt sTempDiv, sN_1, sTempA, sTemp1;

	for (i = 0; i < 550; i++) {
		if (Mod_long(sIn, PrimeTable[i]) == 0)
			return SPRD_CRYPTO_ERROR;
	}

	Sub_long(&sN_1, sIn, 1);
	sTemp1.m_iLength32 = 0;

	for (i = 0; i < 8; i++) {
		iPassMark = 0;
		Mov_long(&sTempA, myrand() * myrand());
		Mov_bigint(&sTempDiv, &sN_1);

		while ((sTempDiv.m_ulValue[0] & 1) == 0) {
			for (j = 0; j < sTempDiv.m_iLength32; j++) {
				sTempDiv.m_ulValue[j] = sTempDiv.m_ulValue[j] >> 1;
				if (sTempDiv.m_ulValue[j + 1] & 1)
					sTempDiv.m_ulValue[j] = sTempDiv.m_ulValue[j] | 0x80000000;
			}

			if (sTempDiv.m_ulValue[sTempDiv.m_iLength32 - 1] == 0)
				sTempDiv.m_iLength32--;

			RsaTrans(&sTemp1, &sTempA, &sTempDiv, sIn);

			if (Cmp(&sTemp1, &sN_1) == 0) {
				iPassMark = 1;
				break;
			}
		}

		if ((sTemp1.m_iLength32 == 1) && (sTemp1.m_ulValue[0] == 1))
			iPassMark = 1;

		if (iPassMark == 0)
			return SPRD_CRYPTO_ERROR;
	}

	return SPRD_CRYPTO_SUCCESS;
}

/****************************************************************************************
 ****************************************************************************************/
void GetPrime(SBigInt * sOut, int iLength32)
{
	int i;
	sOut->m_iLength32 = iLength32;

	iLength32--;
	for (i = sOut->m_iLength32; i < BIGINT_MAXLEN; i++)
		sOut->m_ulValue[i] = 0;

	do {
		for (i = 0; i < sOut->m_iLength32; i++)
			sOut->m_ulValue[i] = Rand();

		sOut->m_ulValue[0] = sOut->m_ulValue[0] | 0x11;
		sOut->m_ulValue[iLength32] = sOut->m_ulValue[iLength32] | 0x80000000;
	} while (Rab(sOut) == SPRD_CRYPTO_ERROR);
}

/****************************************************************************************
 ****************************************************************************************/
void Euc(SBigInt * sOut, SBigInt * sIn, SBigInt * sN)
{
	SBigInt sTemp1, sTemp2, sTemp3, sTemp4, sTemp5, sTemp6;
	int x, y;

	Mov_bigint(&sTemp1, sN);
	Mov_bigint(&sTemp2, sIn);
	Mov_long(&sTemp3, 0);
	Mov_long(&sTemp4, 1);

	x = 1;
	y = 1;

	while (sTemp2.m_iLength32 != 1 || sTemp2.m_ulValue[0] != 0) {
		Div_bigint(&sTemp5, &sTemp1, &sTemp2);
		Mod_bigint(&sTemp6, &sTemp1, &sTemp2);
		Mov_bigint(&sTemp1, &sTemp2);
		Mov_bigint(&sTemp2, &sTemp6);
		Mov_bigint(&sTemp6, &sTemp4);
		Mul_bigint(sOut, &sTemp4, &sTemp5);
		Mov_bigint(&sTemp4, sOut);

		if (x == y) {
			if (Cmp(&sTemp3, &sTemp4) >= 0) {
				Sub_bigint(sOut, &sTemp3, &sTemp4);
				Mov_bigint(&sTemp4, sOut);
			} else {
				Sub_bigint(&sTemp4, &sTemp4, &sTemp3);
				y = 0;
			}
		} else {
			Add_bigint(&sTemp4, &sTemp4, &sTemp3);
			x = 1 - x;
			y = 1 - y;
		}

		Mov_bigint(&sTemp3, &sTemp6);
	}

	if (x == 0)
		Sub_bigint(sOut, sN, &sTemp3);
	else
		Mov_bigint(sOut, &sTemp3);

	return;
}

/****************************************************************************************
 ****************************************************************************************/
int RSA_Create(unsigned char *pubExp, int bitLen_N, unsigned char *mod_N, unsigned char *prvExp)
{
	int iLength32 = bitLen_N >> 6;

	if ((mod_N == NULL) || (prvExp == NULL)) {
		return SPRD_CRYPTO_ERROR;
	}

start:
	RSA_initialize();
	SetParameter_char_openssl(&RSA_D, pubExp, 4);

	GetPrime(&RSA_P, iLength32);
	GetPrime(&RSA_Q, iLength32);

	Mul_bigint(&RSA_N, &RSA_P, &RSA_Q);

	Sub_long(&RSA_P, &RSA_P, 1);
	Sub_long(&RSA_Q, &RSA_Q, 1);

	Mul_bigint(&RSA_C, &RSA_P, &RSA_Q);
	Euc(&RSA_E, &RSA_D, &RSA_C);
	Mov_long(&RSA_C, 0);

	if (RSA_E.m_iLength32 < (iLength32 << 1))
		goto start;

	sec_memcpy(prvExp, (unsigned char *)(RSA_E.m_ulValue), RSA_E.m_iLength32 * (sizeof(unsigned int)));
	invert_char(prvExp, bitLen_N >> 3);

	sec_memcpy(mod_N, (unsigned char *)(RSA_N.m_ulValue), RSA_E.m_iLength32 * (sizeof(unsigned int)));
	invert_char(mod_N, bitLen_N >> 3);

	return SPRD_CRYPTO_SUCCESS;
}

int rsa_encrypt_data_download(unsigned char *random_raw_data, unsigned char *send_data)
{
        sprd_crypto_err_t err = SPRD_CRYPTO_SUCCESS;
        unsigned char raw_data[32] = {0};
        int i = 0;

        struct rsa_data
        {
                int data_len;
                uint8_t rsa_n[256];
                uint8_t rsa_e[4];
                uint8_t rsa_d[256];
                uint8_t rsa_p[RAW_RANDOMDATA_LEN];
                uint8_t rsa_c[256];
        } auth_start_data =
        {
                RAW_RANDOMDATA_LEN,
                {
                        0x8A, 0x66, 0x42, 0x1D, 0xF9, 0xD5, 0xB1, 0x8D, 0x69, 0x45, 0x20, 0xFC, 0xD8, 0xB4, 0x90, 0x6C,
                        0x48, 0x23, 0xFF, 0xDB, 0xB7, 0x93, 0x6F, 0x4B, 0x27, 0x02, 0xDE, 0xBA, 0x96, 0x72, 0x4E, 0x2A,
                        0x05, 0xE1, 0xBD, 0x99, 0x75, 0x51, 0x2D, 0x08, 0xE4, 0xC0, 0x9C, 0x78, 0x54, 0x30, 0x0B, 0xE7,
                        0xC3, 0x9F, 0x7B, 0x57, 0x33, 0x0E, 0xEA, 0xC6, 0xA2, 0x7E, 0x5A, 0x36, 0x11, 0xED, 0xC9, 0xA5,
                        0x81, 0x5D, 0x39, 0x14, 0xF0, 0xCC, 0xA8, 0x84, 0x60, 0x3C, 0x17, 0xF3, 0xCF, 0xAB, 0x87, 0x63,
                        0x3F, 0x1A, 0xF6, 0xD2, 0xAE, 0x8A, 0x66, 0x42, 0x1D, 0xF9, 0xD5, 0xB1, 0x8D, 0x69, 0x45, 0x20,
                        0xFC, 0xD8, 0xB4, 0x90, 0x6C, 0x48, 0x23, 0xFF, 0xDB, 0xB7, 0x93, 0x6F, 0x4B, 0x27, 0x02, 0xDE,
                        0xBA, 0x96, 0x72, 0x4E, 0x2A, 0x05, 0xE1, 0xBD, 0x99, 0x75, 0x51, 0x2D, 0x08, 0xE4, 0xC1, 0x62,
                        0xB7, 0xDC, 0x00, 0x24, 0x48, 0x6C, 0x90, 0xB4, 0xD8, 0xFD, 0x21, 0x45, 0x69, 0x8D, 0xB1, 0xD5,
                        0xFA, 0x1E, 0x42, 0x66, 0x8A, 0xAE, 0xD2, 0xF7, 0x1B, 0x3F, 0x63, 0x87, 0xAB, 0xCF, 0xF4, 0x18,
                        0x3C, 0x60, 0x84, 0xA8, 0xCC, 0xF1, 0x15, 0x39, 0x5D, 0x81, 0xA5, 0xC9, 0xEE, 0x12, 0x36, 0x5A,
                        0x7E, 0xA2, 0xC6, 0xEB, 0x0F, 0x33, 0x57, 0x7B, 0x9F, 0xC3, 0xE8, 0x0C, 0x30, 0x54, 0x78, 0x9C,
                        0xC0, 0xE5, 0x09, 0x2D, 0x51, 0x75, 0x99, 0xBD, 0xE2, 0x06, 0x2A, 0x4E, 0x72, 0x96, 0xBA, 0xDF,
                        0x03, 0x27, 0x4B, 0x6F, 0x93, 0xB7, 0xDC, 0x00, 0x24, 0x48, 0x6C, 0x90, 0xB4, 0xD8, 0xFD, 0x21,
                        0x45, 0x69, 0x8D, 0xB1, 0xD5, 0xFA, 0x1E, 0x42, 0x66, 0x8A, 0xAE, 0xD2, 0xF7, 0x1B, 0x3F, 0x63,
                        0x87, 0xAB, 0xCF, 0xF4, 0x18, 0x3C, 0x60, 0x84, 0xA8, 0xCC, 0xF1, 0x15, 0x39, 0x5D, 0x85, 0x03,
                },
                /* e */
                { 0x00, 0x01, 0x00, 0x01, },
                 /* d */
                {0},
                {0},
                {0},
        };

        printf("%s\n", __func__);

#if 0
        printf("the random data: ");
        for(i = 0; i < 32; i++)
        {
            printf("0x%x, ", random_raw_data[i]);
        }
        printf("\n");
#endif
        memcpy(auth_start_data.rsa_p, random_raw_data, RAW_RANDOMDATA_LEN);

        err = RSA_PubEnc(auth_start_data.rsa_e, auth_start_data.rsa_n, 2048, auth_start_data.rsa_p, auth_start_data.data_len, send_data);
        if (err < 0)
        {
            printf("rsa pkcs sign failed!\n");
            return 1;
        }

#if 0
        printf("the encrypt data: ");
        for(i = 0; i < 256; i++)
        {
            printf("%x, ", send_data[i]);
        }
        printf("\n");
#endif

#if 0

        RSA_PrvDec(auth_start_data.rsa_d, auth_start_data.rsa_n, 2048, send_data, raw_data);

        printf("the raw random data: ");
        for(i = 0; i < 32; i++)
        {
            printf("%x, ", raw_data[i]);
        }
        printf("\n");
        if(!memcmp(random_raw_data, raw_data, RAW_RANDOMDATA_LEN))
        {
            printf("encrypt data success\n");
            return 0;
        }
        else
        {
            printf("encrypt data failed\n");
            return 1;
        }
#endif

        return 0;
}

#define CHAR_NUM_MIN '0'
#define CHAR_NUM_MAX '9'
#define CHAR_ML_MIN 'A'
#define CHAR_ML_MAX 'Z'
#define CHAR_LL_MIN 'a'
#define CHAR_LL_MAX 'z'

char convert_char(char cin)
{
	if ( cin < CHAR_ML_MIN ) {
		return cin%(CHAR_NUM_MAX-CHAR_NUM_MIN + 1) + CHAR_NUM_MIN;
	}
	
	if ( cin < CHAR_LL_MIN ) {
		return cin%(CHAR_ML_MAX-CHAR_ML_MIN + 1) + CHAR_ML_MIN;
	}
	
	if (cin >= CHAR_LL_MIN) {
		return cin%(CHAR_LL_MAX-CHAR_LL_MIN + 1) + CHAR_LL_MIN;
	}
	return cin;
}

int rsa_encrypt_data(unsigned char *send_data)
{
	unsigned char temp_auth_data[256] = {0};
	unsigned char raw_data[32] = {0};
	unsigned char *auth_data = NULL;
	size_t auth_data_len = 0;
	int err = 0;
	int i = 0;

	auth_data = send_data;

	struct rsa_data
        {
                int data_len;
                uint8_t rsa_n[256];
                uint8_t rsa_e[4];
                uint8_t rsa_d[256];
                uint8_t rsa_p[RAW_RANDOMDATA_LEN];
                uint8_t rsa_c[256];
        } auth_start_data =
        {
                RAW_RANDOMDATA_LEN,
		{ 	0xc6, 0x8d, 0xd6, 0x3f, 0x91, 0xd3, 0x47, 0x16, 0x3f, 0xac, 0xca, 0x9f, 0xd6, 0x89,
            0xdb, 0x8a, 0x1a, 0x99, 0x62, 0xd4, 0x4f, 0xc5, 0x27, 0xed, 0xff, 0xfe, 0x12, 0x06, 0xf0,
            0x01, 0x76, 0x6b, 0x1b, 0x49, 0xf6, 0x51, 0x9e, 0x88, 0xe5, 0x32, 0x1a, 0x83, 0xa3, 0xfc,
            0x26, 0xa4, 0x9a, 0x25, 0xc7, 0x0c, 0x54, 0x29, 0x28, 0x67, 0x44, 0xe4, 0xaf, 0x64, 0x49,
            0x43, 0x2e, 0xcf, 0x05, 0x79, 0x62, 0x9b, 0x97, 0x5a, 0x63, 0xc8, 0xc5, 0x58, 0xe0, 0x10,
            0x4f, 0xb8, 0xb3, 0xe4, 0xc7, 0xd5, 0x72, 0xbf, 0xb3, 0xa3, 0xc9, 0x4d, 0x1e, 0xa3, 0xfa,
            0x99, 0xa4, 0x11, 0xb7, 0x97, 0x65, 0x04, 0x33, 0x33, 0xa8, 0x2a, 0xb5, 0x7b, 0xde, 0xad,
            0xee, 0x06, 0xd8, 0xd6, 0xdf, 0x7d, 0x51, 0x94, 0x7e, 0x1f, 0xec, 0xe9, 0x0d, 0x01, 0xd4,
            0xc0, 0x85, 0xb2, 0xa9, 0xa2, 0x06, 0x62, 0xe2, 0xef, 0xbe, 0xaa, 0x08, 0xd2, 0x78, 0xfd,
            0x56, 0xe8, 0xad, 0x7d, 0x1b, 0xb0, 0xcb, 0x2d, 0x0e, 0xc7, 0x78, 0xe0, 0x13, 0x79, 0x8d,
            0xc9, 0xa3, 0x3c, 0xa8, 0xdd, 0xcd, 0x0d, 0xd3, 0xd6, 0xb0, 0xcc, 0x11, 0xec, 0xc2, 0x0f,
            0x3f, 0x8a, 0x5e, 0x1d, 0x58, 0xf7, 0x26, 0x80, 0x45, 0xe4, 0x84, 0xb1, 0xf7, 0x68, 0xa8,
            0x3f, 0xf1, 0xb3, 0xc7, 0x7b, 0x83, 0xcb, 0x4c, 0x79, 0x6e, 0x63, 0xee, 0xdc, 0x89, 0x85,
            0xe6, 0xea, 0xf3, 0x9e, 0xf7, 0xce, 0x3f, 0xef, 0x88, 0xc6, 0xd9, 0x89, 0xda, 0xb5, 0x67,
            0xcc, 0x57, 0x56, 0x4f, 0xa7, 0x88, 0xd4, 0x14, 0xb9, 0x33, 0xbb, 0x6f, 0x11, 0x07, 0x60,
            0xc6, 0x93, 0x10, 0xd7, 0x82, 0xb0, 0xe9, 0xa9, 0x40, 0x5e, 0xb3, 0xed, 0x79, 0x7a, 0x31,
            0x3e, 0xb6, 0x5c, 0x6a, 0x47, 0xea, 0x6c, 0xda, 0xba, 0x9d, 0xfd, 0x14, 0x5e, 0x4e, 0xa8,
            0x95, 0x75,
	},
	/* e */
	{ 0x00, 0x01, 0x00, 0x01, },
	/* d */
	{0},

#if 0
	/* d */
	{	0x61,0x12,0xe9,0x79,0xfa,0xde,0x90,0x4e,0x39,0x7b,0x59,0x5c,0x5a,0xaf,0x29,
		0x98,0x2e,0x27,0x94,0xec,0xe4,0x64,0xff,0xa9,0x22,0xdb,0x30,0x71,0x3b,0x3e,
		0xc3,0xee,0x90,0x8f,0x28,0xb5,0xb6,0x63,0x24,0xd8,0x63,0x81,0xa1,0x3e,0xd7,
		0x9e,0x00,0x1f,0x00,0x23,0xf9,0xdd,0x5b,0x67,0x88,0x16,0xdb,0x57,0xda,0xd4,
		0x7f,0x6a,0xbd,0x73,0x9d,0x2a,0x36,0x94,0x2d,0xa9,0xa9,0x3d,0x92,0x23,0xda,
		0x58,0x9a,0xa4,0x2e,0xa0,0xd7,0xc3,0x23,0x29,0xbd,0x5f,0x27,0xb2,0x93,0x26,
		0x3a,0x16,0x60,0x3f,0x3b,0xb1,0x4b,0x3d,0x17,0x41,0x06,0xb7,0x20,0xf8,0x08,
		0x0b,0x61,0x74,0xb3,0x4a,0x20,0x89,0x85,0x71,0x77,0x88,0x8d,0x64,0x25,0x01,
		0xe1,0xbb,0x47,0xaf,0xdb,0x53,0x5f,0xec,0xbd,0xa7,0x93,0x9d,0xdd,0xbe,0x53,
		0x63,0x21,0x0d,0xaf,0x48,0xec,0xaa,0xf2,0xd4,0x35,0x59,0x8f,0x80,0xef,0xd8,
		0x14,0x3d,0x3d,0x08,0xd1,0xed,0xd7,0x4a,0xad,0x92,0x38,0x44,0x01,0xbf,0x77,
		0x38,0x52,0xc5,0x41,0xa1,0x4f,0xee,0x69,0xa2,0x68,0xf7,0x98,0x3c,0x59,0x5d,
		0x18,0xf5,0xe9,0xcc,0xd0,0x69,0x64,0xa1,0x91,0xc9,0xc1,0x53,0x24,0xb4,0x47,
		0xd4,0x98,0x8c,0x3e,0x0c,0x3e,0xbd,0x3c,0x15,0x4b,0xc8,0x5f,0x6d,0xbd,0x84,
		0xde,0x95,0xbe,0xac,0x90,0x30,0x8b,0x49,0xe8,0xa2,0xe1,0xd5,0x2a,0xe9,0xa7,
		0x9d,0xb8,0xf5,0xfa,0x51,0x9c,0x5e,0x93,0x73,0x4b,0x1a,0x1e,0x28,0xc5,0xfd,
		0x5a,0x49,0x57,0x94,0xf5,0x24,0xe9,0xa5,0xfd,0x29,0x57,0xe9,0x4d,0x9b,0x9c,0x21,
	},
#endif
	{0},
	{0},
	};

	printf("%s\n", __func__);

	err = prand_gen(random_data, RAW_RANDOMDATA_LEN);
	if(err < 0)
	{
		printf("general random data failed\n");
		return 1;
	}
	
	for (i = 0; i < RAW_RANDOMDATA_LEN; i ++ ) 
	{
		random_data[i] = convert_char(random_data[i]);
	}

#if DEBUG_RSA_DECRYPT
    printf("---------------\n");
    printf("the raw random data(Char): \n");
	for(i = 0; i < 32; i++)
	{
		printf("%c", random_data[i]);
	}
	printf("\n");
	printf("---------------\n");
	printf("the raw random data: \n");
	for(i = 0; i < 32; i++)
	{
		printf("0x%x, ", random_data[i]);
	}
	printf("\n");
	printf("----------------\n");
#endif	
	memcpy(auth_start_data.rsa_p, random_data, RAW_RANDOMDATA_LEN);


	err = RSA_PubEnc(auth_start_data.rsa_e, auth_start_data.rsa_n, 2048, auth_start_data.rsa_p, auth_start_data.data_len, auth_start_data.rsa_c);
	if (err < 0)
	{
		printf("rsa pkcs sign failed!\n");
		return 1;
	}
#if DEBUG_RSA_DECRYPT	
    printf("-----------------\n");
	printf("the raw encrypt data: ");
	for(i = 0; i < 256; i++)
	{
		printf("%d, ", auth_start_data.rsa_c[i]);
	}
	printf("\n");
	printf("-----------------\n");
#endif	
	if (sizeof(auth_start_data.rsa_c) % 3 > 0)
	{
		auth_data_len = (sizeof(auth_start_data.rsa_c)/3 + 1) * 4;
	} else {
		auth_data_len = (sizeof(auth_start_data.rsa_c)/3) * 4;
	}

	b64_ntop(auth_start_data.rsa_c, sizeof(auth_start_data.rsa_c), auth_data, auth_data_len);
	
#if DEBUG_RSA_DECRYPT	
    printf("-----------------\n");
	printf("auth_data: \n");
	for(i = 0; i < auth_data_len; i++)
		printf("%c", auth_data[i]);
	printf("\n");
	printf("-----------------\n");
#endif	
#if 0
	printf("the b64 encrypt data: ");
	for(i = 0; i < 344; i++)
	{
		printf("%c",auth_data[i]);
	}
	printf("\n");

	b64_pton(auth_data, temp_auth_data, auth_data_len);

	RSA_PrvDec(auth_start_data.rsa_d, auth_start_data.rsa_n, 2048, temp_auth_data, raw_data);

	printf("the raw random data: ");
	for(i = 0; i < 32; i++)
	{
		printf("%d, ", raw_data[i]);
	}
	printf("\n");
	if(!memcmp(random_data, raw_data, RAW_RANDOMDATA_LEN))
	{
		printf("encrypt data success\n");
		return 0;
	}
	else
	{
		printf("encrypt data failed\n");
		return 1;
	}
#endif

	return 0;
}


int rsa_decrypt_data(unsigned char *data_type, unsigned char *revice_data, unsigned char *sn_data)
{
	unsigned char raw_data[48] = {0};
	unsigned char revice_raw_data[33 + PRODUCT_SN_TOKEN_MAX_SIZE] = {0x0};
	unsigned char temp_revice_data[256] = {0};
	unsigned char auth_data[512] = {0};
	unsigned char temp_auth_data[512] = {0};
	size_t auth_data_len = 0;
	int err = 0;
	int ret = 0;
	int i = 0;

	struct rsa_data
	{
		int data_len;
		uint8_t rsa_n[256];
		uint8_t rsa_e[4];
		uint8_t rsa_d[256];
		uint8_t rsa_p[RAW_RANDOMDATA_LEN];
		uint8_t rsa_c[256];
	} auth_start_data =
	{
		RAW_RANDOMDATA_LEN,
		{0},
		/* e */
		{0},
		/* d */
		{0},
		{0},
		{0},
	};

	unsigned char flash_e[4] = {0x00, 0x01, 0x00, 0x01};
	unsigned char flash_n[256] = {  0xb2, 0x83, 0x77, 0x9d, 0x72, 0x1c, 0xed, 0x3a, 0x50, 0x03, 0xdd, 0xef, 0x8e, 0x81,
                    0x28, 0xdf, 0x53, 0xa3, 0xbf, 0xf0, 0x5e, 0xc5, 0x8c, 0xd5, 0x70, 0x9a, 0xd3, 0x40, 0x9d,
                    0xe2, 0xd9, 0xe6, 0x99, 0x8d, 0x41, 0x8d, 0xf0, 0x7e, 0x54, 0x23, 0xe0, 0xcd, 0x4f, 0x04,
                    0x2e, 0x07, 0xaf, 0x87, 0x6f, 0xba, 0xd9, 0x90, 0x21, 0x19, 0x54, 0x2d, 0xe7, 0x54, 0x10,
                    0x0b, 0x7e, 0x17, 0xd9, 0x13, 0x09, 0x30, 0x19, 0x15, 0x39, 0x76, 0x06, 0xc8, 0x09, 0x1a,
                    0xe4, 0xa2, 0xae, 0x17, 0x78, 0x68, 0xb9, 0x65, 0xf0, 0x83, 0xb9, 0x12, 0xc5, 0xd5, 0xa7,
                    0x03, 0x5a, 0x8b, 0xfc, 0x59, 0x95, 0x0a, 0x5e, 0x96, 0x19, 0x5a, 0x89, 0x1a, 0x98, 0xb4,
                    0x33, 0x5c, 0xc8, 0x1c, 0xf2, 0xce, 0x09, 0xf0, 0xe4, 0xdc, 0x61, 0xfc, 0xca, 0xe8, 0x98,
                    0xa6, 0x65, 0x1d, 0xf1, 0x05, 0x15, 0x38, 0xf4, 0xb3, 0x8d, 0xfe, 0xf6, 0x5c, 0x7a, 0x39,
                    0x2c, 0x57, 0x53, 0xdf, 0x5b, 0x01, 0x08, 0x0f, 0x67, 0xbf, 0x5d, 0x5e, 0x92, 0x13, 0x1c,
                    0x47, 0xef, 0x7d, 0xbb, 0x85, 0x99, 0x31, 0x96, 0x83, 0x66, 0xc6, 0xda, 0x1d, 0xba, 0xf1,
                    0x2b, 0x14, 0xb5, 0xff, 0xde, 0x2d, 0xdc, 0x3c, 0x20, 0x5f, 0xe2, 0x6d, 0x03, 0xde, 0x91,
                    0xc5, 0x86, 0x33, 0x21, 0xac, 0x78, 0xa0, 0xe1, 0x4e, 0x3f, 0xea, 0x67, 0x9f, 0xcb, 0xa1,
                    0x39, 0x2c, 0xff, 0x4a, 0x7e, 0x30, 0x05, 0x57, 0xc0, 0x32, 0xf1, 0x54, 0x9a, 0xf5, 0xfe,
                    0x79, 0xdc, 0x23, 0x66, 0x86, 0x58, 0x44, 0x25, 0x26, 0xdb, 0xd0, 0xa2, 0x5d, 0x76, 0x97,
                    0x31, 0x53, 0xd1, 0x5b, 0x87, 0x7c, 0xe8, 0x9e, 0x0c, 0x3c, 0xc2, 0x7d, 0xcd, 0x59, 0x96,
                    0x6e, 0x07, 0x22, 0xb7, 0xf6, 0xf9, 0x77, 0x31, 0x8f, 0x4c, 0x7a, 0x9b, 0xa8, 0x75, 0x8c,
                    0x39, 0x23,};

	unsigned char simlock_e[4] = {0x00, 0x01, 0x00, 0x01};
	unsigned char simlock_n[256] = {0xe8, 0x7d, 0xc1, 0x24, 0x79, 0x87, 0x40, 0xef, 0x5e, 0xf9, 0x96, 0xd7, 0xed, 0xbd, 
                    0x76, 0x05, 0x3b, 0xdb, 0x26, 0xe2, 0x35, 0xef, 0x32, 0x61, 0x67, 0x12, 0x47, 0xc2, 0xfb,
                    0x51, 0xa4, 0x69, 0x82, 0xe5, 0xa5, 0xd3, 0xab, 0x2c, 0xee, 0x18, 0xe7, 0x8c, 0xc7, 0x08,
                    0x98, 0x1f, 0x85, 0xb9, 0x80, 0xf9, 0x1d, 0xeb, 0xdf, 0x70, 0x92, 0xe1, 0x92, 0xe8, 0xc6,
                    0x70, 0xe6, 0xa8, 0x81, 0xac, 0x62, 0x7c, 0xea, 0x10, 0xff, 0x13, 0x91, 0xfd, 0x06, 0xf4,
                    0x16, 0x6c, 0xde, 0x4c, 0x29, 0xc4, 0xe7, 0x24, 0x74, 0x1d, 0x16, 0x2e, 0xb4, 0x0e, 0x37,
                    0x6b, 0x2e, 0xe9, 0x87, 0xdf, 0xcf, 0x09, 0x64, 0x24, 0xac, 0xd6, 0x6a, 0xcc, 0x10, 0xed,
                    0x17, 0xfe, 0xb5, 0xfe, 0xf0, 0x59, 0xbe, 0xc6, 0x91, 0xb0, 0x01, 0x33, 0xfc, 0xc4, 0xe7,
                    0x3f, 0x3e, 0x52, 0x16, 0xda, 0x0a, 0x92, 0x22, 0xef, 0xe2, 0xdf, 0x5c, 0x9b, 0x31, 0x21,
                    0xd1, 0xa3, 0xc2, 0x51, 0x59, 0x86, 0x48, 0x8f, 0x26, 0x80, 0x64, 0xd4, 0xfe, 0x94, 0x59,
                    0x1c, 0x18, 0x40, 0xdc, 0xc8, 0x7d, 0x35, 0x35, 0x89, 0x11, 0xc9, 0x25, 0x9d, 0x8b, 0xb2,
                    0xb8, 0x77, 0x55, 0x73, 0x57, 0xdd, 0x70, 0xa3, 0x04, 0xe4, 0x33, 0x99, 0xd8, 0x6f, 0x29,
                    0xfe, 0xdd, 0xc0, 0x77, 0x0f, 0x5e, 0x86, 0x03, 0x72, 0xb6, 0xfa, 0x50, 0x8c, 0xa3, 0x46,
                    0x9e, 0x52, 0x80, 0x40, 0xa8, 0xfc, 0xa5, 0xc1, 0x52, 0x28, 0xfe, 0xb2, 0xbb, 0xaa, 0x3a,
                    0x82, 0x51, 0x87, 0xee, 0xef, 0x77, 0x38, 0xcf, 0x99, 0x5f, 0xa5, 0x5a, 0x19, 0x12, 0x26,
                    0x20, 0x40, 0x2a, 0x29, 0x1c, 0xe4, 0xf4, 0x2e, 0x95, 0x24, 0xa8, 0x74, 0x3d, 0xf3, 0x36,
                    0x18, 0x16, 0x2f, 0x47, 0xce, 0x9d, 0x73, 0xbb, 0x84, 0xe9, 0x70, 0x88, 0xb6, 0x17, 0xc8,
                    0xf6, 0xd7,};

	unsigned char repair_e[4] = {0x00, 0x01, 0x00, 0x01};
	unsigned char repair_n[256] = { 0xd6, 0x20, 0xac, 0xe2, 0xa7, 0xd5, 0x3b, 0x39, 0xb2, 0xa3, 0x92, 0xb0, 0x5c, 0x46,
                    0x3c, 0x4d, 0xe0, 0x9b, 0x35, 0x36, 0xc7, 0x1b, 0xc0, 0x6c, 0x2f, 0x51, 0x85, 0x44, 0x80,
                    0xec, 0xf5, 0xc1, 0x6c, 0x71, 0x96, 0x5b, 0x39, 0xd7, 0x37, 0x36, 0x58, 0xb3, 0x69, 0x2d,
                    0x33, 0xc9, 0x3b, 0xe4, 0xd1, 0x4c, 0x1a, 0xfa, 0x8f, 0x77, 0x48, 0x9f, 0xac, 0x01, 0x7b,
                    0x58, 0x90, 0xe3, 0x9b, 0xab, 0x7c, 0xb3, 0x85, 0xaa, 0x2e, 0x34, 0x63, 0xbc, 0x42, 0xcd,
                    0x5e, 0x2e, 0x6e, 0xef, 0xfa, 0x37, 0x11, 0x5a, 0xe3, 0x14, 0x49, 0xe7, 0x1f, 0x2e, 0xeb,
                    0xa0, 0x29, 0x04, 0x90, 0x02, 0x28, 0x65, 0xad, 0x19, 0xe4, 0x64, 0x6d, 0x01, 0xd5, 0x8e,
                    0xbd, 0x5f, 0xed, 0xb6, 0xae, 0xfb, 0x80, 0x66, 0x1f, 0xd1, 0xfd, 0x08, 0x51, 0xa4, 0x4e,
                    0xb6, 0xaa, 0xc2, 0x9a, 0x83, 0xaa, 0x56, 0xd4, 0x40, 0xa2, 0x41, 0xa8, 0xfb, 0x8c, 0x7e,
                    0xfc, 0x6b, 0x78, 0x26, 0x45, 0x1a, 0x84, 0xf5, 0xa7, 0xab, 0x87, 0xcb, 0xa5, 0x17, 0x41,
                    0x75, 0x18, 0x86, 0x32, 0xb1, 0xdc, 0x67, 0xea, 0x19, 0xd6, 0xc1, 0x51, 0xfb, 0x88, 0x41,
                    0xda, 0x5c, 0x29, 0xb4, 0xa8, 0x57, 0x8f, 0x3e, 0xd2, 0x67, 0x85, 0x15, 0xf2, 0x8c, 0x2d,
                    0x3c, 0x3a, 0xcc, 0xf8, 0x44, 0x7a, 0x51, 0x54, 0x75, 0xc8, 0x56, 0xb8, 0x09, 0xdb, 0x99,
                    0x18, 0xb2, 0x19, 0x3a, 0xde, 0xec, 0xe9, 0x02, 0x7b, 0xd1, 0xc6, 0xc8, 0x54, 0x15, 0xb6,
                    0x27, 0x92, 0x10, 0x5a, 0xc5, 0x4a, 0xfa, 0x8e, 0x72, 0x01, 0x24, 0x1b, 0x85, 0xb5, 0x21,
                    0xa5, 0xfb, 0x85, 0xd9, 0xf1, 0x68, 0x56, 0x13, 0x0d, 0x21, 0xf9, 0x3b, 0xd5, 0xb6, 0x6a,
                    0xb8, 0x50, 0x16, 0xa5, 0x72, 0x06, 0x46, 0x13, 0xa4, 0x27, 0x03, 0x66, 0xcd, 0x03, 0x44,
                    0x18, 0xeb,};

	printf("%s\n", __func__);
#if 0
	err = prand_gen(auth_start_data.rsa_p, RAW_RANDOMDATA_LEN);
	if(err < 0)
	{
		printf("general random data failed\n");
		return 0;
	}

	memcpy(&auth_start_data.rsa_p[32], ";0123456789ABCDEF", 17);
	for(i = 0; i < 32; i++)
	{
		auth_start_data.rsa_p[i] = 0x5a;
	}
	printf("the raw random data: ");
	for(i = 0; i < auth_start_data.data_len; i++)
	{
		printf("%x, ", auth_start_data.rsa_p[i]);
	}
	printf("\n");

	err = RSA_Sgin(auth_start_data.rsa_d, auth_start_data.rsa_n, 2048, auth_start_data.rsa_p, auth_start_data.data_len, auth_start_data.rsa_c);
	if (err < 0)
	{
		printf("rsa pkcs sign failed!\n");
		return 0;
	}

	printf("the raw sign data: ");
	for(i = 0; i < 256; i++)
	{
		printf("%x", auth_start_data.rsa_c[i]);
	}
	printf("\n");


	b64_ntop(auth_start_data.rsa_c, sizeof(auth_start_data.rsa_c), auth_data, auth_data_len);

	printf("the b64 sign data: ");
	for(i = 0; i < 344; i++)
	{
		printf("%c", auth_data[i]);
	}
	printf("\n");

	b64_pton(auth_data, temp_auth_data, auth_data_len);

#endif
	if (sizeof(auth_start_data.rsa_c) % 3 > 0)
	{
		auth_data_len = (sizeof(auth_start_data.rsa_c)/3 + 1) * 4;
	} else {
		auth_data_len = (sizeof(auth_start_data.rsa_c)/3) * 4;
	}

	if(!strcmp(data_type, "flash")){
		memcpy(auth_start_data.rsa_e, flash_e, sizeof(auth_start_data.rsa_e));
		memcpy(auth_start_data.rsa_n, flash_n, sizeof(auth_start_data.rsa_n));
		ret = 1;
	}else if(!strcmp(data_type, "simlock")){
		memcpy(auth_start_data.rsa_e, simlock_e, sizeof(auth_start_data.rsa_e));
		memcpy(auth_start_data.rsa_n, simlock_n, sizeof(auth_start_data.rsa_n));
		ret = 2;
	}else if(!strcmp(data_type, "repair")){
		memcpy(auth_start_data.rsa_e, repair_e, sizeof(auth_start_data.rsa_e));
		memcpy(auth_start_data.rsa_n, repair_n, sizeof(auth_start_data.rsa_n));
		ret = 4;
	}

#if 0
	RSA_Verify(auth_start_data.rsa_e, auth_start_data.rsa_n, 2048, temp_auth_data, raw_data);
	printf("the raw_data: ");
	for(i = 0; i < auth_start_data.data_len; i++)
	{
		printf("%x, ", raw_data[i]);
	}
	printf("\n");
#endif


#if DEBUG_RSA_DECRYPT
	printf("the revice char data: ");
	for(i = 0; i < 344; i++)
	{
		printf("%c", revice_data[i]);
	}
	printf("\n");
#endif

	for(i = 0; i < 344; i++)
	{
		revice_data[i] = (int)revice_data[i];
	}

#if DEBUG_RSA_DECRYPT
	printf("the revice data: ");
	for(i = 0; i < 344; i++)
	{
		printf("%x", revice_data[i]);
	}
	printf("\n");
#endif

	b64_pton(revice_data, temp_revice_data, auth_data_len);
#if DEBUG_RSA_DECRYPT
	printf("the b64 revice data: ");
	for(i = 0; i < 256; i++)
	{
		printf("%x", temp_revice_data[i]);
	}
    printf("\n");
	
	printf("the b64 revice data (char): ");
	for(i = 0; i < 256; i++)
		printf("%c", temp_revice_data[i]);

	printf("\n");
#endif
	err = RSA_Verify(auth_start_data.rsa_e, auth_start_data.rsa_n, 2048, temp_revice_data, revice_raw_data);
	if (err < 0)
	{
		printf("rsa verify dec failed!\n");
		ret = 0;
	}

#if DEBUG_RSA_DECRYPT
    printf("-------------------\n");
	printf("the revice_raw_data =>: ");
	for(i = 0; i < RAW_RANDOMDATA_LEN; i++) 
		printf("0x%x, ", revice_raw_data[i]);
	printf("\n");
	printf("the random_data =>: ");
	for(i = 0; i < RAW_RANDOMDATA_LEN; i++) 
		printf("0x%x, ", random_data[i]);
	printf("\n");
	printf("-------------------\n");
	printf("the revice_raw_data (SN): ");
	for(i = 0; i < 33 + PRODUCT_SN_TOKEN_MAX_SIZE; i++)
		printf("%c", revice_raw_data[i]);
	printf("\n");
	printf("the sn_data (SN): %d==%d", strlen(revice_raw_data) - 33, PRODUCT_SN_TOKEN_MAX_SIZE);
	for(i = 0; i < PRODUCT_SN_TOKEN_MAX_SIZE; i++)
		printf("%c", sn_data[i]);
	printf("\n");
	printf("-------------------\n");
	printf("the revice_raw_data (SN): ");
	for(i = 0; i < 33 + PRODUCT_SN_TOKEN_MAX_SIZE; i++)
		printf("0x%x, ", revice_raw_data[i]);
	printf("\n");
	printf("the sn_data (SN): %d==%d", strlen(revice_raw_data) - 33, PRODUCT_SN_TOKEN_MAX_SIZE);
	for(i = 0; i < PRODUCT_SN_TOKEN_MAX_SIZE; i++)
		printf("0x%x, ", sn_data[i]);
	printf("\n");
	printf("-------------------\n");
#endif

	if(memcmp(revice_raw_data, random_data, RAW_RANDOMDATA_LEN))
	{
		printf("decrypt random data failed\n");
		ret = 0;
	}
	else
	{
		printf("decrypt random data success\n");
	}


	if(memcmp(&revice_raw_data[33], sn_data, strlen(revice_raw_data) - 33))
	{
		printf("decrypt serial number data failed\n");
		ret = 0;
	}
	else
	{
		printf("decrypt serial number data success\n");
	}


	return ret;

}

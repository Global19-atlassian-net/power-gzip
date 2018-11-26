#include "../test_deflate.h"
#include "../test_utils.h"

static alloc_func zalloc = (alloc_func)0;
static free_func zfree = (free_func)0;

/* use nx to deflate */
static int _test_nx_deflate(Byte* src, unsigned int src_len, Byte* compr, unsigned int compr_len)
{
	int err;
	z_stream c_stream;
	
	c_stream.zalloc = zalloc;
	c_stream.zfree = zfree;
	c_stream.opaque = (voidpf)0;
	
	err = nx_deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
	if (err != 0) {
		printf("nx_deflateInit err %d\n", err);
		return TEST_ERROR;
	}
	
	c_stream.next_in  = (z_const unsigned char *)src;
	c_stream.next_out = compr;

	c_stream.avail_in = src_len;
	c_stream.avail_out = compr_len;
	err = nx_deflate(&c_stream, Z_FINISH);

	assert(c_stream.total_in == src_len);
	assert(err == Z_STREAM_END);

	err = nx_deflateEnd(&c_stream);
	if (err != 0) {
		return TEST_ERROR;
	}

	return TEST_OK;
}

/* use zlib inflate to infalte */
static int _test_inflate(Byte* compr, unsigned int comprLen, Byte* uncompr, unsigned int uncomprLen, Byte* src, unsigned int src_len, int step)
{
        int err;
        z_stream d_stream;

	memset(uncompr, 0, uncomprLen);

        d_stream.zalloc = zalloc;
        d_stream.zfree = zfree;
        d_stream.opaque = (voidpf)0;

        d_stream.next_in  = compr;
        d_stream.avail_in = 0;
        d_stream.next_out = uncompr;

        err = inflateInit(&d_stream);
        while (d_stream.total_out < uncomprLen && d_stream.total_in < comprLen) {
                d_stream.avail_in = d_stream.avail_out = step;
                err = inflate(&d_stream, Z_NO_FLUSH);
                if (err == Z_STREAM_END) break;
        }

        err = inflateEnd(&d_stream);

        if (compare_data(uncompr, src, src_len)) {
		return TEST_ERROR;
        }

	return TEST_OK;
}

/* use nx inflate to infalte */
static int _test_nx_inflate(Byte* compr, unsigned int comprLen, Byte* uncompr, unsigned int uncomprLen, Byte* src, unsigned int src_len, int step)
{
        int err;
        z_stream d_stream;

	memset(uncompr, 0, uncomprLen);

        d_stream.zalloc = zalloc;
        d_stream.zfree = zfree;
        d_stream.opaque = (voidpf)0;

        d_stream.next_in  = compr;
        d_stream.avail_in = 0;
        d_stream.next_out = uncompr;

        err = nx_inflateInit(&d_stream);
        while (d_stream.total_out < uncomprLen && d_stream.total_in < comprLen) {
                d_stream.avail_in = d_stream.avail_out = step;
                err = nx_inflate(&d_stream, Z_NO_FLUSH);
                if (err == Z_STREAM_END) break;
        }

        err = nx_inflateEnd(&d_stream);

        if (compare_data(uncompr, src, src_len)) {
		return TEST_ERROR;
        }

	return TEST_OK;
}

/* The total src buffer > nx_compress_threshold (10*1024) but avail_in is 1 */
static int run(unsigned int len, int step, const char* test)
{
	Byte *src, *compr, *uncompr;
	unsigned int src_len = len;
	unsigned int compr_len = src_len*2;
	unsigned int uncompr_len = src_len*2;
	generate_random_data(src_len);
	src = &ran_data[0];

	compr = (Byte*)calloc((uInt)compr_len, 1);
	uncompr = (Byte*)calloc((uInt)uncompr_len, 1);
	if (compr == NULL || uncompr == NULL ) {
		printf("*** alloc buffer failed\n");
		return TEST_ERROR;
	}

	if (_test_nx_deflate(src, src_len, compr, compr_len)) goto err;
	if (_test_inflate(compr, compr_len, uncompr, uncompr_len, src, src_len, step)) goto err;
	if (_test_nx_inflate(compr, compr_len, uncompr, uncompr_len, src, src_len, step)) goto err;

	printf("*** %s %s passed\n", __FILE__, test);
	free(compr);
	free(uncompr);
	return TEST_OK;
err:
	free(compr);
	free(uncompr);
	return TEST_ERROR;
}

/* case prefix is 30 ~ 39 */

/* The total src buffer < nx_compress_threshold (10*1024) */
int run_case30()
{
	return run(5*1024, 1,  __func__);
}

/* The total src buffer > nx_compress_threshold (10*1024) */
int run_case31()
{
	return run(64*1024, 1, __func__);
}

/* A large buffer > fifo_in len and and avail_in == total */
int run_case32()
{
	return run(1024*1024*8, 1024*1024*8, __func__);
}

/* A large buffer > fifo_in len and and avail_in > 10*1024 */
int run_case33()
{
	return run(1024*33, 1024*32, __func__);
}

int run_case33_1()
{
	return run(1024*33, 1024*32, __func__);
}
/* A large buffer > fifo_in len and and avail_in == total */
int run_case34()
{
	return run(1024*1024*20, 1024*1024*20, __func__);
}


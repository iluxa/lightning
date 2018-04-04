#include <stdio.h>
#include "bitcoin/block.h"
#include "bitcoin/pullpush.h"
#include "bitcoin/tx.h"
#include <ccan/str/hex/hex.h>
#include <common/type_to_string.h>

/* Encoding is <blockhdr> <varint-num-txs> <tx>... */
struct bitcoin_block *bitcoin_block_from_hex(const tal_t *ctx,
					     const char *hex, size_t hexlen)
{
	struct bitcoin_block *b;
	u8 *linear_tx;
	const u8 *p;
	size_t len, i, num;

	if (hexlen && hex[hexlen-1] == '\n')
		hexlen--;

	/* Set up the block for success. */
	b = tal(ctx, struct bitcoin_block);

	/* De-hex the array. */
	len = hex_data_size(hexlen);
	p = linear_tx = tal_arr(ctx, u8, len);
    printf("p: %p\n", p);
	if (!hex_decode(hex, hexlen, linear_tx, len)) {
        printf("hex_decode failed\n");
		return tal_free(b);
    }

	pull(&p, &len, &b->hdr, sizeof(b->hdr));
	//printf("blkid: %s\n", type_to_string(tmpctx, struct bitcoin_blkid, &b->hdr.blkid));
	printf("version: %d\n", b->hdr.version);
	printf("nonce: %d\n", b->hdr.nonce);
	printf("timestamp: %d\n", b->hdr.timestamp);
	//printf("prev_hash: %s\n", type_to_string(ctx, struct bitcoin_blkid, &b->hdr.prev_hash));fflush(stdout);
	//printf("merkle root: %s\n", type_to_string(ctx, struct sha256_double, &b->hdr.merkle_hash));
	//printf("hashStateRoot: %s\n", type_to_string(ctx, struct sha256_double, &b->hdr.hashStateRoot));
	//printf("hashUTXORoot: %s\n", type_to_string(ctx, struct sha256_double, &b->hdr.hashUTXORoot));
	//printf("prevoutStakehash: %s\n", type_to_string(ctx, struct sha256_double, &b->hdr.prevoutStakehash));
	//printf("prevoutStakeint: %d\n", b->hdr.prevoutStakeint);

	b->ch_block_sig_len = pull_varint(&p, &len);

    printf("p1: %p len: %u\n", p, b->ch_block_sig_len);
	pull(&p, &len, b->ch_block_sig, b->ch_block_sig_len);

	num = pull_varint(&p, &len);
    printf("p2: %p num: %lu\n", p, num);
	b->tx = tal_arr(b, struct bitcoin_tx *, num);
	for (i = 0; i < num; i++) {
        printf("p2[%lu]: %p num: %lu\n", i, p, num);
		b->tx[i] = pull_bitcoin_tx(b->tx, &p, &len);
    }
    printf("p3: %p\n", p);

	/* We should end up not overrunning, nor have extra */
	if (!p || len) {
        printf("!p: %p || len: %lu failed\n", p, len);
		return tal_free(b);
    }

	tal_free(linear_tx);
	return b;
}

/* We do the same hex-reversing crud as txids. */
bool bitcoin_blkid_from_hex(const char *hexstr, size_t hexstr_len,
			    struct bitcoin_blkid *blockid)
{
	struct bitcoin_txid fake_txid;
	if (!bitcoin_txid_from_hex(hexstr, hexstr_len, &fake_txid))
		return false;
	blockid->shad = fake_txid.shad;
	return true;
}

bool bitcoin_blkid_to_hex(const struct bitcoin_blkid *blockid,
			  char *hexstr, size_t hexstr_len)
{
	struct bitcoin_txid fake_txid;
	fake_txid.shad = blockid->shad;
	return bitcoin_txid_to_hex(&fake_txid, hexstr, hexstr_len);
}

static char *fmt_bitcoin_blkid(const tal_t *ctx,
			       const struct bitcoin_blkid *blkid)
{
	char *hexstr = tal_arr(ctx, char, hex_str_size(sizeof(*blkid)));

	bitcoin_blkid_to_hex(blkid, hexstr, hex_str_size(sizeof(*blkid)));
	return hexstr;
}
REGISTER_TYPE_TO_STRING(bitcoin_blkid, fmt_bitcoin_blkid);

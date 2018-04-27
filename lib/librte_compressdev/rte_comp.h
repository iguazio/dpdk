/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2017-2018 Intel Corporation
 */

#ifndef _RTE_COMP_H_
#define _RTE_COMP_H_

/**
 * @file rte_comp.h
 *
 * RTE definitions for Data Compression Service
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <rte_mempool.h>
#include <rte_mbuf.h>

/** Status of comp operation */
enum rte_comp_op_status {
	RTE_COMP_OP_STATUS_SUCCESS = 0,
	/**< Operation completed successfully */
	RTE_COMP_OP_STATUS_NOT_PROCESSED,
	/**< Operation has not yet been processed by the device */
	RTE_COMP_OP_STATUS_INVALID_ARGS,
	/**< Operation failed due to invalid arguments in request */
	RTE_COMP_OP_STATUS_ERROR,
	/**< Error handling operation */
	RTE_COMP_OP_STATUS_INVALID_STATE,
	/**< Operation is invoked in invalid state */
	RTE_COMP_OP_STATUS_OUT_OF_SPACE_TERMINATED,
	/**< Output buffer ran out of space before operation completed.
	 * Error case. Application must resubmit all data with a larger
	 * output buffer.
	 */
	RTE_COMP_OP_STATUS_OUT_OF_SPACE_RECOVERABLE,
	/**< Output buffer ran out of space before operation completed, but this
	 * is not an error case. Output data up to op.produced can be used and
	 * next op in the stream should continue on from op.consumed+1.
	 */
};

/** Compression Algorithms */
enum rte_comp_algorithm {
	RTE_COMP_ALGO_UNSPECIFIED = 0,
	/** No Compression algorithm */
	RTE_COMP_ALGO_NULL,
	/**< No compression.
	 * Pass-through, data is copied unchanged from source buffer to
	 * destination buffer.
	 */
	RTE_COMP_ALGO_DEFLATE,
	/**< DEFLATE compression algorithm
	 * https://tools.ietf.org/html/rfc1951
	 */
	RTE_COMP_ALGO_LZS,
	/**< LZS compression algorithm
	 * https://tools.ietf.org/html/rfc2395
	 */
	RTE_COMP_ALGO_LIST_END
};

/**< Compression Level.
 * The number is interpreted by each PMD differently. However, lower numbers
 * give fastest compression, at the expense of compression ratio while
 * higher numbers may give better compression ratios but are likely slower.
 */
#define	RTE_COMP_LEVEL_PMD_DEFAULT	(-1)
/** Use PMD Default */
#define	RTE_COMP_LEVEL_NONE		(0)
/** Output uncompressed blocks if supported by the specified algorithm */
#define RTE_COMP_LEVEL_MIN		(1)
/** Use minimum compression level supported by the PMD */
#define RTE_COMP_LEVEL_MAX		(9)
/** Use maximum compression level supported by the PMD */

/** Compression checksum types */
enum rte_comp_checksum_type {
	RTE_COMP_CHECKSUM_NONE,
	/**< No checksum generated */
	RTE_COMP_CHECKSUM_CRC32,
	/**< Generates a CRC32 checksum, as used by gzip */
	RTE_COMP_CHECKSUM_ADLER32,
	/**< Generates an Adler-32 checksum, as used by zlib */
	RTE_COMP_CHECKSUM_CRC32_ADLER32,
	/**< Generates both Adler-32 and CRC32 checksums, concatenated.
	 * CRC32 is in the lower 32bits, Adler-32 in the upper 32 bits.
	 */
};


/** Compression Huffman Type - used by DEFLATE algorithm */
enum rte_comp_huffman {
	RTE_COMP_HUFFMAN_DEFAULT,
	/**< PMD may choose which Huffman codes to use */
	RTE_COMP_HUFFMAN_FIXED,
	/**< Use Fixed Huffman codes */
	RTE_COMP_HUFFMAN_DYNAMIC,
	/**< Use Dynamic Huffman codes */
};

/** Compression flush flags */
enum rte_comp_flush_flag {
	RTE_COMP_FLUSH_NONE,
	/**< Data is not flushed. Output may remain in the compressor and be
	 * processed during a following op. It may not be possible to decompress
	 * output until a later op with some other flush flag has been sent.
	 */
	RTE_COMP_FLUSH_SYNC,
	/**< All data should be flushed to output buffer. Output data can be
	 * decompressed. However state and history is not cleared, so future
	 * operations may use history from this operation.
	 */
	RTE_COMP_FLUSH_FULL,
	/**< All data should be flushed to output buffer. Output data can be
	 * decompressed. State and history data is cleared, so future
	 * ops will be independent of ops processed before this.
	 */
	RTE_COMP_FLUSH_FINAL
	/**< Same as RTE_COMP_FLUSH_FULL but if op.algo is RTE_COMP_ALGO_DEFLATE
	 * then bfinal bit is set in the last block.
	 */
};

/** Compression transform types */
enum rte_comp_xform_type {
	RTE_COMP_COMPRESS,
	/**< Compression service - compress */
	RTE_COMP_DECOMPRESS,
	/**< Compression service - decompress */
};

/** Compression operation type */
enum rte_comp_op_type {
	RTE_COMP_OP_STATELESS,
	/**< All data to be processed is submitted in the op, no state or
	 * history from previous ops is used and none will be stored for future
	 * ops. Flush flag must be set to either FLUSH_FULL or FLUSH_FINAL.
	 */
	RTE_COMP_OP_STATEFUL
	/**< There may be more data to be processed after this op, it's part of
	 * a stream of data. State and history from previous ops can be used
	 * and resulting state and history can be stored for future ops,
	 * depending on flush flag.
	 */
};


/** Parameters specific to the deflate algorithm */
struct rte_comp_deflate_params {
	enum rte_comp_huffman huffman;
	/**< Compression huffman encoding type */
};

/** Setup Data for compression */
struct rte_comp_compress_xform {
	enum rte_comp_algorithm algo;
	/**< Algorithm to use for compress operation */
	union {
		struct rte_comp_deflate_params deflate;
		/**< Parameters specific to the deflate algorithm */
	}; /**< Algorithm specific parameters */
	int level;
	/**< Compression level */
	uint8_t window_size;
	/**< Base two log value of sliding window to be used. If window size
	 * can't be supported by the PMD then it may fall back to a smaller
	 * size. This is likely to result in a worse compression ratio.
	 */
	enum rte_comp_checksum_type chksum;
	/**< Type of checksum to generate on the uncompressed data */
};

/**
 * Setup Data for decompression.
 */
struct rte_comp_decompress_xform {
	enum rte_comp_algorithm algo;
	/**< Algorithm to use for decompression */
	enum rte_comp_checksum_type chksum;
	/**< Type of checksum to generate on the decompressed data */
	uint8_t window_size;
	/**< Base two log value of sliding window which was used to generate
	 * compressed data. If window size can't be supported by the PMD then
	 * setup of stream or private_xform should fail.
	 */
};

/**
 * Compression transform structure.
 *
 * This is used to specify the compression transforms required.
 * Each transform structure can hold a single transform, the type field is
 * used to specify which transform is contained within the union.
 */
struct rte_comp_xform {
	enum rte_comp_xform_type type;
	/**< xform type */
	union {
		struct rte_comp_compress_xform compress;
		/**< xform for compress operation */
		struct rte_comp_decompress_xform decompress;
		/**< decompress xform */
	};
};

/**
 * Compression Operation.
 *
 * This structure contains data relating to performing a compression
 * operation on the referenced mbuf data buffers.
 *
 * Comp operations are enqueued and dequeued in comp PMDs using the
 * rte_compressdev_enqueue_burst() / rte_compressdev_dequeue_burst() APIs
 */
struct rte_comp_op {
	enum rte_comp_op_type op_type;
	union {
		void *private_xform;
		/**< Stateless private PMD data derived from an rte_comp_xform.
		 * A handle returned by rte_compressdev_private_xform_create()
		 * must be attached to operations of op_type RTE_COMP_STATELESS.
		 */
		void *stream;
		/**< Private PMD data derived initially from an rte_comp_xform,
		 * which holds state and history data and evolves as operations
		 * are processed. rte_compressdev_stream_create() must be called
		 * on a device for all STATEFUL data streams and the resulting
		 * stream attached to the one or more operations associated
		 * with the data stream.
		 * All operations in a stream must be sent to the same device.
		 */
	};

	struct rte_mempool *mempool;
	/**< Pool from which operation is allocated */
	rte_iova_t iova_addr;
	/**< IOVA address of this operation */
	struct rte_mbuf *m_src;
	/**< source mbuf
	 * The total size of the input buffer(s) can be retrieved using
	 * rte_pktmbuf_data_len(m_src)
	 */
	struct rte_mbuf *m_dst;
	/**< destination mbuf
	 * The total size of the output buffer(s) can be retrieved using
	 * rte_pktmbuf_data_len(m_dst)
	 */

	struct {
		uint32_t offset;
		/**< Starting point for compression or decompression,
		 * specified as number of bytes from start of packet in
		 * source buffer.
		 * Starting point for checksum generation in compress direction.
		 */
		uint32_t length;
		/**< The length, in bytes, of the data in source buffer
		 * to be compressed or decompressed.
		 * Also the length of the data over which the checksum
		 * should be generated in compress direction
		 */
	} src;
	struct {
		uint32_t offset;
		/**< Starting point for writing output data, specified as
		 * number of bytes from start of packet in dest
		 * buffer. Starting point for checksum generation in
		 * decompress direction.
		 */
	} dst;
	enum rte_comp_flush_flag flush_flag;
	/**< Defines flush characteristics for the output data.
	 * Only applicable in compress direction
	 */
	uint64_t input_chksum;
	/**< An input checksum can be provided to generate a
	 * cumulative checksum across sequential blocks in a STATELESS stream.
	 * Checksum type is as specified in xform chksum_type
	 */
	uint64_t output_chksum;
	/**< If a checksum is generated it will be written in here.
	 * Checksum type is as specified in xform chksum_type.
	 */
	uint32_t consumed;
	/**< The number of bytes from the source buffer
	 * which were compressed/decompressed.
	 */
	uint32_t produced;
	/**< The number of bytes written to the destination buffer
	 * which were compressed/decompressed.
	 */
	uint64_t debug_status;
	/**<
	 * Status of the operation is returned in the status param.
	 * This field allows the PMD to pass back extra
	 * pmd-specific debug information. Value is not defined on the API.
	 */
	uint8_t status;
	/**<
	 * Operation status - use values from enum rte_comp_status.
	 * This is reset to
	 * RTE_COMP_OP_STATUS_NOT_PROCESSED on allocation from mempool and
	 * will be set to RTE_COMP_OP_STATUS_SUCCESS after operation
	 * is successfully processed by a PMD
	 */
} __rte_cache_aligned;

#ifdef __cplusplus
}
#endif

#endif /* _RTE_COMP_H_ */

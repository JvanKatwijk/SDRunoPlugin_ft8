
#ifndef	__FT8_CONSTANTS__
#define	__FT8_CONSTANTS__


#define nrTONES 79
#define FRAMES_PER_TONE 5
#define nrBUFFERS       (3 * nrTONES * FRAMES_PER_TONE)
#define BINWIDTH (12000.0 / 3840)

// Define LDPC parameters
#define FTX_LDPC_N       (174) 
#define FT8_LDPC_BITS	(174) 
#define FTX_LDPC_M       (83)    ///< Number of LDPC checksum bits (FTX_LDPC_N - FTX_LDPC_K)
#define FT8_CHECKSUM_INDICES (83)    ///< Number of LDPC checks bits (FTX_LDPC_N - FTX_LDPC_K)
#define FTX_LDPC_K       (91) 
#define FTX_LDPC_M       (83)    ///< Number of LDPC checksum bits (FTX_LDPC_N - FTX_LDPC_K)
#define FTX_LDPC_N_BYTES ((FTX_LDPC_N + 7) / 8) ///< Number of whole bytes needed to store 174 bits (full message)
#define FTX_LDPC_K_BYTES ((FTX_LDPC_K + 7) / 8) ///< Number of whole bytes needed to store 91 bits (payload + CRC only)
#endif


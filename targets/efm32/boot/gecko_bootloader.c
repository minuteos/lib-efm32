/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/boot/gecko_bootloader.c
 *
 * compiles required gecko bootloader components
 */

#include <boot/gecko_bootloader.h>

#define HardFault_Handler   gbl_HardFault_Handler

#define __rom_end__ __data_load_end

__attribute__((used, section(".bootloader.table"))) const MainBootloaderTable_t mainStageTable;

__attribute__((used, section(".bootloader.props"))) const ApplicationProperties_t sl_app_properties;

#include <em_core.h>

#undef CORE_ENTER_CRITICAL
#undef CORE_EXIT_CRITICAL
#undef CORE_DECLARE_IRQ_STATE
#define CORE_ENTER_CRITICAL()
#define CORE_EXIT_CRITICAL()
#define CORE_DECLARE_IRQ_STATE

#include <core/btl_core.c>
#include <core/btl_main.c>

#include <sl_crypto/src/crypto_aes.c>
#include <sl_crypto/src/crypto_ecp.c>
#include <sl_crypto/src/crypto_sha.c>
#include <sl_crypto/src/crypto_management.c>
#include <sl_crypto/src/shax.c>

#include <sl_crypto/src/cryptoacc_aes.c>
#include <sl_crypto/src/cryptoacc_ecp.c>
#include <sl_crypto/src/cryptoacc_sha.c>

#if defined(SEMAILBOX_PRESENT) || defined(CRYPTOACC_PRESENT)

#include <sl_crypto/src/cryptoacc/src/ba414ep_config.c>
#include <sl_crypto/src/cryptoacc/src/cryptodma_internal.c>
#include <sl_crypto/src/cryptoacc/src/cryptolib_types.c>
#include <sl_crypto/src/cryptoacc/src/sx_aes.c>
#include <sl_crypto/src/cryptoacc/src/sx_blk_cipher.c>
#include <sl_crypto/src/cryptoacc/src/sx_ecc_curves.c>
#include <sl_crypto/src/cryptoacc/src/sx_ecdsa_alg.c>
#include <sl_crypto/src/cryptoacc/src/sx_hash.c>
#include <sl_crypto/src/cryptoacc/src/sx_memcpy.c>

// there is an incompatible declaration for these two functions in cryptoacc_management.c
#define sx_sm4_set_hw_config_for_key    _invalid_sx_sm4_set_hw_config_for_key
#define sx_aria_set_hw_config_for_key   _invalid_sx_aria_set_hw_config_for_key

#include <sl_crypto/src/cryptoacc_management.c>

#undef sx_sm4_set_hw_config_for_key
#undef sx_aria_set_hw_config_for_key

// but they still need to be weakly aliased to prevent link errors
__attribute__((weak, alias("__unsupported")))
uint32_t sx_sm4_set_hw_config_for_key(block_t *key, uint32_t *config);
__attribute__((weak, alias("__unsupported")))
uint32_t sx_aria_set_hw_config_for_key(block_t *key, uint32_t *config);

uint32_t __unsupported(block_t *key, uint32_t *config)
{
    return CRYPTOLIB_UNSUPPORTED_ERR;
}

#endif

#include <library/bignum.c>

#undef EC_BIGINT_COPY
#undef ECC_BIGINT_SIZE_IN_BYTES
#undef ECC_BIGINT_SIZE_IN_32BIT_WORDS

#include <plugin/parser/btl_image_parser.c>
#include <plugin/parser/ebl/btl_gbl_custom_tags.c>

#include <plugin/security/btl_crc32.c>
#include <plugin/security/btl_security_aes.c>
#include <plugin/security/btl_security_ecdsa.c>
#include <plugin/security/btl_security_sha256.c>
#include <plugin/security/btl_security_tokens.c>
#include <plugin/security/ecc/ecc.c>

#include <plugin/storage/btl_storage.c>

#if !GECKO_BOOTLOADER_STORAGE_EXTERNAL

#include <plugin/storage/internal_flash/btl_storage_internal_flash.c>

#endif

/*
 * Copyright (c) 2024 Rockchip Electronics Co. Ltd.
 * Written by Jon Lin <jon.lin@rock-chips.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of other contributors
 *	  may be used to endorse or promote products derived from this software
 *	  without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MTD_UTILS_ALL__
#define __MTD_UTILS_ALL__

#ifdef __cplusplus
extern "C" {
#endif

#define MTD_ABSENT              0
#define MTD_NORFLASH            3
#define MTD_NANDFLASH           4       /* SLC NAND */

/*
 * SPI Nor OTA APIs
 */
int mtd_debug_info(char *mtd_dev);
int mtd_debug_read(char *mtd_dev, size_t offset, size_t length, char *file);
int mtd_debug_write(char *mtd_dev, size_t offset, size_t length, char *file);
int mtd_debug_erase(char *mtd_dev, u_int32_t offset, u_int32_t length);

/*
 * SPI Nand OTA APIs
 */
int nandwrite(char *mtd_dev, char *file, int type);
int nanddump(char *mtd_dev, size_t len, char *file, int type);
int flash_erase(char *mtd_dev, size_t start, size_t length);

/*
 * Common APIs
 */
int flash_write_buf(char *mtd_dev, char *input_buf, size_t start, size_t len);
int flash_read_buf(char *mtd_dev, char *output_buf, size_t start, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __MTD_UTILS_ALL__ */
